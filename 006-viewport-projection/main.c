#define SDL_MAIN_USE_CALLBACKS 1

#include "app.h"
#include "renderer.h"
#include <SDL3/SDL_main.h>
#include <stdlib.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

SDL_AppResult 
SDL_AppInit(void **appstate, int argc, char **argv) 
{
    if (!SDL_Init(SDL_INIT_VIDEO)) 
    {
        SDL_Log("ERROR: Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    struct APP_Context *ctx = malloc(sizeof(struct APP_Context));

    ctx->base_path = SDL_GetBasePath();
    ctx->device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV
            | SDL_GPU_SHADERFORMAT_MSL
            | SDL_GPU_SHADERFORMAT_DXIL, 
            false, 
            NULL
    );

    if (ctx->device == NULL) 
    {
        SDL_Log("ERROR: Failed to create device. %s", SDL_GetError());
        free(ctx);
        return SDL_APP_FAILURE;
    }

    ctx->window = SDL_CreateWindow("Viewport", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (ctx->window == NULL) 
    {
        SDL_Log("ERROR: Failed to create window. %s", SDL_GetError());
        free(ctx);
        return SDL_APP_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(ctx->device, ctx->window)) 
    {
        SDL_Log("ERROR: Failed to claim window. %s", SDL_GetError());
        free(ctx);
        return SDL_APP_FAILURE;
    }

    int result = APP_InitRenderer(ctx);
    if (result == -1)
    { 
        SDL_Log("ERROR: Failed to init renderer.");
        free(ctx);
        return SDL_APP_FAILURE;
    }

    *appstate = ctx;

    return SDL_APP_CONTINUE;
}

SDL_AppResult 
SDL_AppEvent(void *appstate, SDL_Event *event) 
{
    if(event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if(event->type == SDL_EVENT_KEY_DOWN)
    {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult 
SDL_AppIterate(void *appstate) 
{
    struct APP_Context *ctx = appstate;

    ctx->time += 0.1f; 

    APP_Draw(appstate);
    return SDL_APP_CONTINUE;
}

void 
SDL_AppQuit(void *appstate, SDL_AppResult result) 
{
    struct APP_Context *ctx = appstate;

    SDL_ReleaseGPUGraphicsPipeline(ctx->device, ctx->pipeline);

    SDL_ReleaseGPUBuffer(ctx->device, ctx->scene_vertex_buffer);
    SDL_ReleaseGPUBuffer(ctx->device, ctx->scene_index_buffer);

    SDL_ReleaseWindowFromGPUDevice(ctx->device, ctx->window);
    SDL_DestroyWindow(ctx->window);
    SDL_DestroyGPUDevice(ctx->device);

    free(ctx);
}
