#ifndef RENDERER_H
#define RENDERER_H

#include "main.h"
#include "math.h"

SDL_GPUBuffer* APP_CreateVertexBuffer(struct APP_Context *ctx);
SDL_GPUBuffer* APP_CreateIndexBuffer(struct APP_Context *ctx);
void APP_CreateAndSubmitCube(struct APP_Context *ctx);

SDL_GPUGraphicsPipeline* APP_CreateGraphicsPipeline(
    struct APP_Context *ctx,
    SDL_GPUShader *vertex_shader,
    SDL_GPUShader *fragment_shader
);

int APP_InitRenderer(struct APP_Context *ctx);
int APP_Draw(struct APP_Context *ctx);

#endif
