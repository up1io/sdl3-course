#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

typedef struct
{
  const char *base_path;

  SDL_Window *window;
  SDL_GPUDevice *device;
  SDL_GPUGraphicsPipeline *fill_pipeline;
  SDL_GPUBuffer *vertex_buffer;
} Context;

typedef struct
{
  float x, y, z;
  Uint8 r, g, b, a;
} PositionColorVertex;

SDL_GPUShader *load_shader (Context *context, const char *shader_filename,
                            Uint32 sampler_count, Uint32 uniform_buffer_count,
                            Uint32 storage_buffer_count,
                            Uint32 storage_texture_count);

int init_renderer (Context *context);

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
      = SDL_CreateWindow ("Vertext Bufffer", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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
  Context *context = appstate;

  SDL_GPUCommandBuffer *cmd_buf
      = SDL_AcquireGPUCommandBuffer (context->device);
  if (cmd_buf == NULL)
    {
      SDL_Log ("ERROR: failed to acquire gpu cmd buffer: %s", SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  SDL_GPUTexture *swapchain_tex;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture (cmd_buf, context->window,
                                              &swapchain_tex, NULL, NULL))
    {
      SDL_Log ("ERROR: Wait and acquire gpu swapchain texture failed: %s",
               SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  if (swapchain_tex != NULL)
    {
      SDL_GPUColorTargetInfo color_target_info = { 0 };
      color_target_info.texture = swapchain_tex;
      color_target_info.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
      color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
      color_target_info.store_op = SDL_GPU_STOREOP_STORE;

      SDL_GPURenderPass *render_pass
          = SDL_BeginGPURenderPass (cmd_buf, &color_target_info, 1, NULL);

      SDL_BindGPUGraphicsPipeline (render_pass, context->fill_pipeline);
      SDL_BindGPUVertexBuffers (render_pass, 0,
                                &(SDL_GPUBufferBinding){
                                    .buffer = context->vertex_buffer,
                                    .offset = 0,
                                },
                                1);

      SDL_DrawGPUPrimitives (render_pass, 3, 1, 0, 0);
      SDL_EndGPURenderPass (render_pass);
    }
  else
    {
      SDL_Log ("INFO: swapchain_tex null");
    }

  SDL_SubmitGPUCommandBuffer (cmd_buf);

  return SDL_APP_CONTINUE;
}

void
SDL_AppQuit (void *appstate, SDL_AppResult result)
{
  Context *context = appstate;
  SDL_ReleaseGPUGraphicsPipeline (context->device, context->fill_pipeline);
  SDL_ReleaseWindowFromGPUDevice (context->device, context->window);
  SDL_DestroyWindow (context->window);
  SDL_DestroyGPUDevice (context->device);
  free (context);
}

// ====================
// Rendering
// ====================

int
init_renderer (Context *context)
{
  SDL_GPUShader *vertex_shader
      = load_shader (context, "default.vert", 0, 0, 0, 0);
  if (vertex_shader == NULL)
    {
      SDL_Log ("ERROR: Failed to create vertext shader. %s", SDL_GetError ());
      return 1;
    }

  SDL_GPUShader *fragment_shader
      = load_shader (context, "default.frag", 0, 0, 0, 0);
  if (fragment_shader == NULL)
    {
      SDL_Log ("ERROR: Failed to create fragment shader. %s", SDL_GetError ());
      return 1;
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
                       .pitch = sizeof(PositionColorVertex)}},
              .num_vertex_attributes = 2,
              .vertex_attributes =
                  (SDL_GPUVertexAttribute[]){
                      {
                          .buffer_slot = 0,
                          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                          .location = 0,
                          .offset = 0,
                      },
                      {.buffer_slot = 0,
                       .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
                       .location = 1,
                       .offset = sizeof(float) * 3}},
          },
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
  };

  // pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  context->fill_pipeline
      = SDL_CreateGPUGraphicsPipeline (context->device, &pipeline_create_info);
  if (context->fill_pipeline == NULL)
    {
      SDL_Log ("ERROR: Failed to create fill pipeline. %s", SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  SDL_ReleaseGPUShader (context->device, vertex_shader);
  SDL_ReleaseGPUShader (context->device, fragment_shader);

  context->vertex_buffer = SDL_CreateGPUBuffer (
      context->device, &(SDL_GPUBufferCreateInfo){
                           .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                           .size = sizeof (PositionColorVertex) * 3,
                       });

  SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer (
      context->device, &(SDL_GPUTransferBufferCreateInfo){
                           .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                           .size = sizeof (PositionColorVertex) * 3 });

  PositionColorVertex *transfer_data
      = SDL_MapGPUTransferBuffer (context->device, transfer_buffer, false);

  transfer_data[0] = (PositionColorVertex){ -1, -1, 0, 255, 0, 0, 255 };
  transfer_data[1] = (PositionColorVertex){ 1, -1, 0, 255, 0, 0, 255 };
  transfer_data[2] = (PositionColorVertex){ 0, 1, 0, 255, 0, 0, 255 };

  SDL_UnmapGPUTransferBuffer (context->device, transfer_buffer);

  SDL_GPUCommandBuffer *upload_cmd_buf
      = SDL_AcquireGPUCommandBuffer (context->device);
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass (upload_cmd_buf);

  SDL_UploadToGPUBuffer (
      copy_pass,
      &(SDL_GPUTransferBufferLocation){ .transfer_buffer = transfer_buffer,
                                        .offset = 0 },
      &(SDL_GPUBufferRegion){ .buffer = context->vertex_buffer,
                              .offset = 0,
                              .size = sizeof (PositionColorVertex) * 3 },
      false);

  SDL_EndGPUCopyPass (copy_pass);
  SDL_SubmitGPUCommandBuffer (upload_cmd_buf);
  SDL_ReleaseGPUTransferBuffer (context->device, transfer_buffer);

  return 0;
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
