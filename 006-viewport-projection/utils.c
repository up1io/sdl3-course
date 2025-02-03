#include "main.h"
#include <SDL3/SDL.h>

// Load a image from a specified path.
SDL_Surface *
APP_LoadImage(struct APP_Context *cxt, const char *image_filenamen, int desired_channels)
{
  char full_path[256];
  SDL_Surface *result;
  SDL_PixelFormat format;

  SDL_snprintf(full_path, sizeof(full_path), "%simages/%s", cxt->base_path, image_filenamen);

  SDL_Log("load bmp from paht: %s", full_path);

  result = SDL_LoadBMP(full_path);
  if(result == NULL)
  {
    SDL_Log("ERROR: Failed to load bmp: %s", SDL_GetError());
    return NULL;
  }

  if(desired_channels == 4)
  {
    format = SDL_PIXELFORMAT_ABGR8888;
  }
  else
  {
    SDL_assert(!"Unexpected desiredChannels");
    SDL_DestroySurface(result);
    SDL_Log("Unexpected desiredChannels");
    return NULL;
  }

  if(result->format != format)
  {
    SDL_Surface *next = SDL_ConvertSurface(result, format);
    SDL_DestroySurface(result);
    result = next;
  }

  SDL_Log("Image width: %i height: %i", result->w, result->w);
  return result;
}

// Load the compiled shader from a specified path.
SDL_GPUShader *
APP_LoadShader(
    struct APP_Context *cxt, const char *shader_filename, Uint32 sampler_count, Uint32 uniform_buffer_count,
    Uint32 storage_buffer_count, Uint32 storage_texture_count
)
{
  SDL_GPUShaderStage stage;
  if(SDL_strstr(shader_filename, ".vert"))
  {
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  }
  else if(SDL_strstr(shader_filename, ".frag"))
  {
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  }
  else
  {
    SDL_Log("Invalid shader stage");
    return NULL;
  }

  char full_path[256];
  SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(cxt->device);
  SDL_GPUShaderFormat format          = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;

  if(backend_formats & SDL_GPU_SHADERFORMAT_SPIRV)
  {
    SDL_Log("INFO: Use SPIRV shader format");
    SDL_snprintf(full_path, sizeof(full_path), "%sshaders/compiled/SPIRV/%s.spv", cxt->base_path, shader_filename);
    format     = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";
  }
  else if(backend_formats & SDL_GPU_SHADERFORMAT_MSL)
  {
    SDL_Log("INFO: Use MSL shader format");
    SDL_snprintf(full_path, sizeof(full_path), "%sshaders/compiled/MSL/%s.msl", cxt->base_path, shader_filename);
    format     = SDL_GPU_SHADERFORMAT_MSL;
    entrypoint = "main";
  }
  else if(backend_formats & SDL_GPU_SHADERFORMAT_DXIL)
  {
    SDL_Log("INFO: Use DXIL shader format");
    SDL_snprintf(full_path, sizeof(full_path), "%sshaders/compiled/DXIL/%s.dxil", cxt->base_path, shader_filename);
    format     = SDL_GPU_SHADERFORMAT_DXIL;
    entrypoint = "main";
  }
  else
  {
    SDL_Log("%s", "Unreconized backend shader format");
    return NULL;
  }

  size_t code_size;
  void *code = SDL_LoadFile(full_path, &code_size);
  if(code == NULL)
  {
    SDL_Log("Failed to load shader from disk: %s", full_path);
    return NULL;
  }

  SDL_GPUShaderCreateInfo shader_info = {
    .code                 = code,
    .code_size            = code_size,
    .entrypoint           = entrypoint,
    .format               = format,
    .stage                = stage,
    .num_samplers         = sampler_count,
    .num_uniform_buffers  = uniform_buffer_count,
    .num_storage_buffers  = storage_buffer_count,
    .num_storage_textures = storage_texture_count,
  };

  SDL_GPUShader *shader = SDL_CreateGPUShader(cxt->device, &shader_info);
  if(shader == NULL)
  {
    SDL_Log("Failed to create shader.");
    SDL_free(code);
    return NULL;
  }

  SDL_free(code);
  return shader;
}
