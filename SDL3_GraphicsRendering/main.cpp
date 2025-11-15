/* lines.c ... */

/*
 * This example creates an SDL window and renderer, and then draws some lines
 * to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>
#include <random>
#include <windows.h>
#include <chrono>
#include <iostream>


// 虚拟三维引擎
struct Vec3 {
    double x, y, z;
};

// 罗德里格公式旋转函数
Vec3 rotateRodriguesAroundPoint(const Vec3& point, const Vec3& axis, double theta, const Vec3& center) {
    // 平移点，使旋转中心在原点
    double px = point.x - center.x;
    double py = point.y - center.y;
    double pz = point.z - center.z;

    // 单位化旋转轴
    double len = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    double ux = axis.x / len;
    double uy = axis.y / len;
    double uz = axis.z / len;

    double cosT = std::cos(theta);
    double sinT = std::sin(theta);
    double dot = ux * px + uy * py + uz * pz;

    // 罗德里格公式
    double rx = px * cosT + (uy * pz - uz * py) * sinT + ux * dot * (1 - cosT);
    double ry = py * cosT + (uz * px - ux * pz) * sinT + uy * dot * (1 - cosT);
    double rz = pz * cosT + (ux * py - uy * px) * sinT + uz * dot * (1 - cosT);

    // 平移回旋转中心
    Vec3 result = { rx + center.x, ry + center.y, rz + center.z };
    return result;
}

// 点集
double points[][3]{
    {200, 200, 200},
    {800, 200, 200},
    {200, 800, 200},
    {800, 800, 200},
    {200, 200, 800},
    {800, 200, 800},
    {200, 800, 800},
    {800, 800, 800}
};
double center[3];

// 边集
double lines[][2][3]{
    {{200,200,200}, {800,200,200}},  // 底面边
    {{200,200,200}, {200,800,200}},
    {{800,200,200}, {800,800,200}},
    {{200,800,200}, {800,800,200}},

    {{200,200,800}, {800,200,800}},  // 顶面边
    {{200,200,800}, {200,800,800}},
    {{800,200,800}, {800,800,800}},
    {{200,800,800}, {800,800,800}},

    {{200,200,200}, {200,200,800}},  // 垂直边
    {{800,200,200}, {800,200,800}},
    {{200,800,200}, {200,800,800}},
    {{800,800,200}, {800,800,800}}
};

std::mt19937 gen{ std::random_device{}() };
std::uniform_real_distribution<> dis{ 0.0, 1.0 };

// 窗口
/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // 渲染初始化
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < sizeof(points) / sizeof(double) / 3; j++) {
            center[i] += points[j][i];
        }
        center[i] /= sizeof(points) / sizeof(double) / 3;
    }

    // 窗口
    SDL_SetAppMetadata("Example Renderer Lines", "1.0", "com.example.renderer-lines");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/lines", 1000, 1000, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, 1000, 1000, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
std::chrono::steady_clock::time_point timePoint;
SDL_AppResult SDL_AppIterate(void *appstate)
{
    // 清空画板（覆盖）
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // 线旋转逻辑
    static double theta = 3.14;
    static Vec3 axies;

    
    if (theta > 3.0) {
        theta = 0.0;
        axies = Vec3{ dis(gen), dis(gen), dis(gen) };
        timePoint = std::chrono::high_resolution_clock::now();
    }
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - timePoint;

    theta += elapsed.count() * 0.001;
    std::cout << theta << " : " << axies.x << " " << axies.y << " " << axies.z << std::endl;
    for (int i = 0; i < sizeof(lines) / sizeof(double) / 2 / 3; i++) {
        for (int j = 0; j < 2; j++) {
            auto vec = rotateRodriguesAroundPoint(
                Vec3{ lines[i][j][0], lines[i][j][1], lines[i][j][2] },
                axies,
                theta,
                Vec3{ center[0], center[1], center[2] }
            );
            lines[i][j][0] = vec.x;
            lines[i][j][1] = vec.y;
            lines[i][j][2] = vec.z;
        }
    }

    // 线渲染逻辑
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    for (auto line : lines) {
        SDL_RenderLine(renderer, line[0][0], line[0][1], line[1][0], line[1][1]);
    }

    // 完成渲染
    SDL_RenderPresent(renderer);  /* put it all on the screen! */
    timePoint = std::chrono::high_resolution_clock::now();

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

