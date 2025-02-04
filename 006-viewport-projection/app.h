#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

struct APP_Context {
    const char *base_path;
    float time;

    SDL_Window *window;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline;

    SDL_GPUBuffer *scene_vertex_buffer;
    SDL_GPUBuffer *scene_index_buffer;
};

#endif
