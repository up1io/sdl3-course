#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

typedef struct
{
  const char *base_path;

  SDL_Window *window;
  SDL_GPUDevice *device;

  SDL_GPUGraphicsPipeline *pipeline;
  SDL_GPUSampler *sampler;
  SDL_GPUBuffer *index_buffer;
  SDL_GPUBuffer *vertex_buffer;
  SDL_GPUTexture *texture;
} Context;

typedef struct
{
  float x, y, z;
  float u, v;
} PositionTextureVertex;

// ====================
// Rendering
// ====================

SDL_Surface *
load_image (Context *context, const char *image_filenamen,
            int desired_channels)
{
  char full_path[256];
  SDL_Surface *result;
  SDL_PixelFormat format;

  SDL_snprintf (full_path, sizeof (full_path), "%simages/%s",
                context->base_path, image_filenamen);

  SDL_Log ("load bmp from paht: %s", full_path);

  result = SDL_LoadBMP (full_path);
  if (result == NULL)
    {
      SDL_Log ("ERROR: Failed to load bmp: %s", SDL_GetError ());
      return NULL;
    }

  if (desired_channels == 4)
    {
      format = SDL_PIXELFORMAT_ABGR8888;
    }
  else
    {
      SDL_assert (!"Unexpected desiredChannels");
      SDL_DestroySurface (result);
      SDL_Log ("Unexpected desiredChannels");
      return NULL;
    }

  if (result->format != format)
    {
      SDL_Surface *next = SDL_ConvertSurface (result, format);
      SDL_DestroySurface (result);
      result = next;
    }

  SDL_Log ("Image width: %i height: %i", result->w, result->w);
  return result;
}

SDL_GPUShader *
load_shader (Context *context, const char *shader_filename,
             Uint32 sampler_count, Uint32 uniform_buffer_count,
             Uint32 storage_buffer_count, Uint32 storage_texture_count)
{
  SDL_GPUShaderStage stage;
  if (SDL_strstr (shader_filename, ".vert"))
    {
      stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
  else if (SDL_strstr (shader_filename, ".frag"))
    {
      stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
  else
    {
      SDL_Log ("Invalid shader stage");
      return NULL;
    }

  char full_path[256];
  SDL_GPUShaderFormat backend_formats
      = SDL_GetGPUShaderFormats (context->device);
  SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;

  if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
      SDL_Log ("INFO: Use SPIRV shader format");
      SDL_snprintf (full_path, sizeof (full_path),
                    "%sshaders/compiled/SPIRV/%s.spv", context->base_path,
                    shader_filename);
      format = SDL_GPU_SHADERFORMAT_SPIRV;
      entrypoint = "main";
    }
  else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL)
    {
      SDL_Log ("INFO: Use MSL shader format");
      SDL_snprintf (full_path, sizeof (full_path),
                    "%sshaders/compiled/MSL/%s.msl", context->base_path,
                    shader_filename);
      format = SDL_GPU_SHADERFORMAT_MSL;
      entrypoint = "main";
    }
  else if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL)
    {
      SDL_Log ("INFO: Use DXIL shader format");
      SDL_snprintf (full_path, sizeof (full_path),
                    "%sshaders/compiled/DXIL/%s.dxil", context->base_path,
                    shader_filename);
      format = SDL_GPU_SHADERFORMAT_DXIL;
      entrypoint = "main";
    }
  else
    {
      SDL_Log ("%s", "Unreconized backend shader format");
      return NULL;
    }

  size_t code_size;
  void *code = SDL_LoadFile (full_path, &code_size);
  if (code == NULL)
    {
      SDL_Log ("Failed to load shader from disk: %s", full_path);
      return NULL;
    }

  SDL_GPUShaderCreateInfo shader_info = {
    .code = code,
    .code_size = code_size,
    .entrypoint = entrypoint,
    .format = format,
    .stage = stage,
    .num_samplers = sampler_count,
    .num_uniform_buffers = uniform_buffer_count,
    .num_storage_buffers = storage_buffer_count,
    .num_storage_textures = storage_texture_count,
  };

  SDL_GPUShader *shader = SDL_CreateGPUShader (context->device, &shader_info);
  if (shader == NULL)
    {
      SDL_Log ("Failed to create shader.");
      SDL_free (code);
      return NULL;
    }

  SDL_free (code);
  return shader;
}

int
init_renderer (Context *context)
{
  SDL_GPUShader *vertex_shader
      = load_shader (context, "default.vert", 0, 0, 0, 0);
  if (vertex_shader == NULL)
    {
      SDL_Log ("ERROR: Failed to create vertex shader.");
      return -1;
    }

  SDL_GPUShader *fragment_shader
      = load_shader (context, "default.frag", 1, 0, 0, 0);
  if (fragment_shader == NULL)
    {
      SDL_Log ("ERROR: Failed to create fragment shader.");
      return -1;
    }

  SDL_Surface *image_data = load_image (context, "default.bmp", 4);
  if (image_data == NULL)
    {
      SDL_Log ("ERROR: Could not load image data.");
      return -1;
    }

  SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
      .target_info =
          {
              .num_color_targets = 1,
              .color_target_descriptions =
                  (SDL_GPUColorTargetDescription[]){
                      {.format = SDL_GetGPUSwapchainTextureFormat(
                           context->device, context->window)}},
          },
      .vertex_input_state =
          (SDL_GPUVertexInputState){
              .num_vertex_buffers = 1,
              .vertex_buffer_descriptions =
                  (SDL_GPUVertexBufferDescription[]){
                      {.slot = 0,
                       .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                       .instance_step_rate = 0,
                       .pitch = sizeof(PositionTextureVertex)}},
              .num_vertex_attributes = 2,
              .vertex_attributes =
                  (SDL_GPUVertexAttribute[]){
                      {.buffer_slot = 0,
                       .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                       .location = 0,
                       .offset = 0},
                      {.buffer_slot = 0,
                       .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                       .location = 1,
                       .offset = sizeof(float) * 3}}},
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader};

  context->pipeline
      = SDL_CreateGPUGraphicsPipeline (context->device, &pipeline_create_info);
  if (context->pipeline == NULL)
    {
      SDL_Log ("ERROR: Failed to create pipeline.");
      return -1;
    }

  SDL_ReleaseGPUShader (context->device, vertex_shader);
  SDL_ReleaseGPUShader (context->device, fragment_shader);

  context->sampler = SDL_CreateGPUSampler (
      context->device, &(SDL_GPUSamplerCreateInfo){
                           .min_filter = SDL_GPU_FILTER_LINEAR,
                           .mag_filter = SDL_GPU_FILTER_LINEAR,
                           .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                           .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                           .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                           .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                       });

  context->vertex_buffer = SDL_CreateGPUBuffer (
      context->device, &(SDL_GPUBufferCreateInfo){
                           .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                           .size = sizeof (PositionTextureVertex) * 4,
                       });
  SDL_SetGPUBufferName (context->device, context->vertex_buffer,
                        "Vertex Buffer");

  context->index_buffer = SDL_CreateGPUBuffer (
      context->device,
      &(SDL_GPUBufferCreateInfo){ .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                                  .size = sizeof (Uint16) * 6 });

  context->texture = SDL_CreateGPUTexture (
      context->device, &(SDL_GPUTextureCreateInfo){
                           .type = SDL_GPU_TEXTURETYPE_2D,
                           .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                           .width = image_data->w,
                           .height = image_data->h,
                           .layer_count_or_depth = 1,
                           .num_levels = 1,
                           .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER });

  SDL_SetGPUTextureName (context->device, context->texture, "Texture");

  SDL_GPUTransferBuffer *buffer_transfer_buffer = SDL_CreateGPUTransferBuffer (
      context->device, &(SDL_GPUTransferBufferCreateInfo){
                           .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                           .size = (sizeof (PositionTextureVertex) * 4)
                                   + (sizeof (Uint16) * 6) });

  PositionTextureVertex *transfer_data = SDL_MapGPUTransferBuffer (
      context->device, buffer_transfer_buffer, false);

  transfer_data[0] = (PositionTextureVertex){ -1, 1, 0, 0, 0 };
  transfer_data[1] = (PositionTextureVertex){ 1, 1, 0, 4, 0 };
  transfer_data[2] = (PositionTextureVertex){ 1, -1, 0, 4, 4 };
  transfer_data[3] = (PositionTextureVertex){ -1, -1, 0, 0, 4 };

  Uint16 *index_data = (Uint16 *)&transfer_data[4];

  index_data[0] = 0;
  index_data[1] = 1;
  index_data[2] = 2;
  index_data[3] = 0;
  index_data[4] = 2;
  index_data[5] = 3;

  SDL_UnmapGPUTransferBuffer (context->device, buffer_transfer_buffer);

  SDL_GPUTransferBuffer *texture_transfer_buffer
      = SDL_CreateGPUTransferBuffer (
          context->device, &(SDL_GPUTransferBufferCreateInfo){
                               .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                               .size = image_data->w * image_data->h * 4 });

  Uint8 *texture_transfer_ptr = SDL_MapGPUTransferBuffer (
      context->device, texture_transfer_buffer, false);

  SDL_memcpy (texture_transfer_ptr, image_data->pixels,
              image_data->w * image_data->h * 4);
  SDL_UnmapGPUTransferBuffer (context->device, texture_transfer_buffer);

  SDL_GPUCommandBuffer *upload_cmd_buffer
      = SDL_AcquireGPUCommandBuffer (context->device);
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass (upload_cmd_buffer);

  SDL_UploadToGPUBuffer (
      copy_pass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = buffer_transfer_buffer, .offset = 0 },
      &(SDL_GPUBufferRegion){ .buffer = context->vertex_buffer,
                              .offset = 0,
                              .size = sizeof (PositionTextureVertex) * 4 },
      false);

  SDL_UploadToGPUBuffer (
      copy_pass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = buffer_transfer_buffer,
          .offset = sizeof (PositionTextureVertex) * 4 },
      &(SDL_GPUBufferRegion){ .buffer = context->index_buffer,
                              .offset = 0,
                              .size = sizeof (Uint16) * 6 },
      false);

  SDL_UploadToGPUTexture (
      copy_pass,
      &(SDL_GPUTextureTransferInfo){
          .transfer_buffer = texture_transfer_buffer, .offset = 0 },
      &(SDL_GPUTextureRegion){ .texture = context->texture,
                               .w = image_data->w,
                               .h = image_data->h,
                               .d = 1 },
      false);

  SDL_EndGPUCopyPass (copy_pass);
  SDL_SubmitGPUCommandBuffer (upload_cmd_buffer);
  SDL_DestroySurface (image_data);
  SDL_ReleaseGPUTransferBuffer (context->device, buffer_transfer_buffer);
  SDL_ReleaseGPUTransferBuffer (context->device, texture_transfer_buffer);

  return 0;
}

int
draw (Context *context)
{
  SDL_GPUCommandBuffer *cmd_buf
      = SDL_AcquireGPUCommandBuffer (context->device);
  if (cmd_buf == NULL)
    {
      SDL_Log ("ERROR: Failed to acquire gpu command buffer. %s",
               SDL_GetError ());
      return -1;
    }

  SDL_GPUTexture *swapchain_texture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture (cmd_buf, context->window,
                                              &swapchain_texture, NULL, NULL))
    {
      SDL_Log ("ERROR: Failed to acquire gpu swapchain texture. %s",
               SDL_GetError ());
      return -1;
    }

  if (swapchain_texture != NULL)
    {
      SDL_GPUColorTargetInfo color_target_info = { 0 };
      color_target_info.texture = swapchain_texture;
      color_target_info.clear_color = (SDL_FColor){ 0.5f, 0.5f, 0.5f, 1.0f };
      color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
      color_target_info.store_op = SDL_GPU_STOREOP_STORE;

      SDL_GPURenderPass *render_pass
          = SDL_BeginGPURenderPass (cmd_buf, &color_target_info, 1, NULL);

      SDL_BindGPUGraphicsPipeline (render_pass, context->pipeline);

      SDL_BindGPUVertexBuffers (
          render_pass, 0,
          &(SDL_GPUBufferBinding){ .buffer = context->vertex_buffer,
                                   .offset = 0 },
          1);

      SDL_BindGPUIndexBuffer (
          render_pass,
          &(SDL_GPUBufferBinding){ .buffer = context->index_buffer,
                                   .offset = 0 },
          SDL_GPU_INDEXELEMENTSIZE_16BIT);

      SDL_BindGPUFragmentSamplers (render_pass, 0,
                                   &(SDL_GPUTextureSamplerBinding){
                                       .texture = context->texture,
                                       .sampler = context->sampler,
                                   },
                                   1);

      SDL_DrawGPUIndexedPrimitives (render_pass, 6, 1, 0, 0, 0);

      SDL_EndGPURenderPass (render_pass);
    }

  SDL_SubmitGPUCommandBuffer (cmd_buf);

  return 0;
}

// ====================
// END Rendering
// ====================

// ====================
// SDL Callbacks
// ====================

SDL_AppResult
SDL_AppInit (void **appstate, int argc, char **argv)
{
  if (!SDL_Init (SDL_INIT_VIDEO))
    {
      SDL_Log ("ERROR: Couldn't initialize SDL: %s", SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  Context *context = (Context *)malloc (sizeof (Context));

  context->base_path = SDL_GetBasePath ();
  context->device = SDL_CreateGPUDevice (SDL_GPU_SHADERFORMAT_SPIRV
                                             | SDL_GPU_SHADERFORMAT_MSL
                                             | SDL_GPU_SHADERFORMAT_DXIL,
                                         false, NULL);

  if (context->device == NULL)
    {
      SDL_Log ("ERROR: Failed to create device. %s", SDL_GetError ());
      free (context);
      return SDL_APP_FAILURE;
    }

  context->window
      = SDL_CreateWindow ("UV Quad", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (context->window == NULL)
    {
      SDL_Log ("ERROR: Failed to create window. %s", SDL_GetError ());
      free (context);
      return SDL_APP_FAILURE;
    }

  if (!SDL_ClaimWindowForGPUDevice (context->device, context->window))
    {
      SDL_Log ("ERROR: Failed to claim window. %s", SDL_GetError ());
      free (context);
      return SDL_APP_FAILURE;
    }

  int result = init_renderer (context);
  if (result)
    {
      SDL_Log ("ERROR: Failed to init renderer.");
      free (context);
      return SDL_APP_FAILURE;
    }

  *appstate = context;

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent (void *appstate, SDL_Event *event)
{
  if (event->type == SDL_EVENT_QUIT)
    {
      return SDL_APP_SUCCESS;
    }

  if (event->type == SDL_EVENT_KEY_DOWN)
    {
      return SDL_APP_SUCCESS;
    }

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppIterate (void *appstate)
{
  draw (appstate);
  return SDL_APP_CONTINUE;
}

void
SDL_AppQuit (void *appstate, SDL_AppResult result)
{
  Context *context = appstate;

  SDL_ReleaseGPUGraphicsPipeline (context->device, context->pipeline);
  SDL_ReleaseGPUBuffer (context->device, context->vertex_buffer);
  SDL_ReleaseGPUBuffer (context->device, context->index_buffer);
  SDL_ReleaseGPUTexture (context->device, context->texture);
  SDL_ReleaseGPUSampler (context->device, context->sampler);

  SDL_ReleaseWindowFromGPUDevice (context->device, context->window);
  SDL_DestroyWindow (context->window);
  SDL_DestroyGPUDevice (context->device);

  free (context);
}

// ====================
// END SDL Callbacks
// ====================
