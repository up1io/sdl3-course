#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>

struct APP_PositionColorVertex {
  float x, y, z;
  Uint8 f, g, b, a;
};

struct scene {
  SDL_GPUGraphicsPipeline *pipeline;
  SDL_GPUBuffer *vertext_buffer;
  SDL_GPUBuffer *index_buffer;
  SDL_GPUTexture *color_texture;
  SDL_GPUTexture *depth_texture;
};

// This is a comment
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
