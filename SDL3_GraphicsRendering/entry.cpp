#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <chrono>
#include <random>
#include <cmath>

// 虚拟三维引擎
struct Vec3 {
    double x, y, z;
};
Vec3 tmpLines[12][2];
extern Vec3 lines[12][2];
extern Vec3 center;

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

void linesIterator() {
    using namespace std::chrono;
    static auto timePoint = high_resolution_clock::now();

    static std::mt19937 gen{ std::random_device{}() };
    static std::uniform_real_distribution<> dis{ 0.0, 1.0};

    static Vec3 axies{ dis(gen), dis(gen), dis(gen) };
    static double theta;

    theta = duration<double>{ high_resolution_clock::now() - timePoint }.count() * 0.1;

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 2; j++) {
            auto point = rotateRodriguesAroundPoint(
                lines[i][j],
                axies,
                theta,
                center
            );
            tmpLines[i][j] = point;
        }
    } 

    return;
}

// 图形渲染
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("test", "1.0", "test");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("test", 1000, 1000, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, 1000, 1000, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    // 清空画板（覆盖）
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // 线旋转逻辑
    linesIterator();

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    for (auto line : tmpLines) {
        SDL_RenderLine(renderer, line[0].x, line[0].y, line[1].x, line[1].y);
    }
    // 完成渲染
    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}