#ifndef UTILS_H
#define UTILS_H

#include "app.h"

SDL_Surface *APP_LoadImage(
        struct APP_Context *cxt, 
        const char *image_filenamen, 
        int desired_channels
);

SDL_GPUShader *APP_LoadShader(
        struct APP_Context *context, 
        const char *shader_filename,
        Uint32 sampler_count, 
        Uint32 uniform_buffer_count,
        Uint32 storage_buffer_count, 
        Uint32 storage_texture_count
);

#endif
