#include <SDL3/SDL_init.h>
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_Window *window = NULL;

SDL_GPUGraphicsPipeline *fill_pipeline;
SDL_GPUDevice *device = NULL;

const char *base_path = NULL;

SDL_GPUShader*
load_shader(
        SDL_GPUDevice *device, 
        const char *shader_filename,
        Uint32 sampler_count, 
        Uint32 uniform_buffer_count,
        Uint32 storage_buffer_count,
        Uint32 storage_texture_count
);

SDL_AppResult
SDL_AppInit(void **appstate, int argc, char **argv)
{
    SDL_SetAppMetadata("Window", "1.0", "com.up.window");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("ERROR: Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    base_path = SDL_GetBasePath();

    // Note(john): The following steps are required
    // Load Shader (vertex, fragment)
    // Create Pipelines
    // Release Shaders

    device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV
            | SDL_GPU_SHADERFORMAT_MSL
            | SDL_GPU_SHADERFORMAT_DXIL,
            false, 
            NULL
    );
    if (device == NULL)
    {
        SDL_Log("ERROR: Failed to create device. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Example", 640, 480, 0);
    if (window == NULL)
    {
        SDL_Log("ERROR: Failed to create window. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window))
    {
        SDL_Log("ERROR: Failed to claim window. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUShader *vertex_shader = load_shader(device, "default.vert", 0, 0, 0, 0);
    if (vertex_shader == NULL)
    {
        SDL_Log("ERROR: Failed to create vertext shader. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUShader *fragment_shader = load_shader(device, "default.frag", 0, 0, 0, 0);
    if (fragment_shader == NULL)
    {
        SDL_Log("ERROR: Failed to create fragment shader. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                {
                    .format = SDL_GetGPUSwapchainTextureFormat(device, window)
                }
            },
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
    };

    pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    fill_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_create_info);
    if (fill_pipeline == NULL)
    {
        SDL_Log("ERROR: Failed to create fill pipeline. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, fragment_shader);

    return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent(void *appstate, SDL_Event *event)
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
SDL_AppIterate(void *appstate)
{
    SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(device);
    if (cmd_buf == NULL)
    {
        SDL_Log("ERROR: failed to acquire gpu cmd buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture *swapchain_tex;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, window, &swapchain_tex, NULL, NULL))
    {
        SDL_Log("ERROR: Wait and acquire gpu swapchain texture failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchain_tex != NULL)
    {
        SDL_GPUColorTargetInfo color_target_info = { 0 };
        color_target_info.texture = swapchain_tex;
        color_target_info.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
        color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target_info.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmd_buf, &color_target_info, 1, NULL);

        SDL_BindGPUGraphicsPipeline(render_pass, fill_pipeline);

        SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(render_pass);
    }
    else
    {
        SDL_Log("INFO: swapchain_tex null");
    }

    SDL_SubmitGPUCommandBuffer(cmd_buf);

    return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_ReleaseGPUGraphicsPipeline(device, fill_pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(device);
}

SDL_GPUShader*
load_shader(
        SDL_GPUDevice *device, 
        const char *shader_filename,
        Uint32 sampler_count, 
        Uint32 uniform_buffer_count,
        Uint32 storage_buffer_count, 
        Uint32 storage_texture_count
)
{
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shader_filename, ".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (SDL_strstr(shader_filename, ".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("ERROR: Invalid shader stage");
        return NULL;
    }

    char full_path[256];
    SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char *entrypoint;

    if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        SDL_Log("INFO: Use SPIRV shader format");

        SDL_snprintf(
                full_path, 
                sizeof (full_path),
                "%sshaders/compiled/SPIRV/%s.spv",
                base_path,
                shader_filename
        );

        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL)
    {
        SDL_Log("INFO: Use MSL shader format");
        SDL_snprintf(
              full_path, 
              sizeof (full_path),
              "%sshaders/compiled/MSL/%s.msl", 
              base_path,
              shader_filename
        );
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main";
    }
    else if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        SDL_Log("INFO: Use DXIL shader format");
        SDL_snprintf(
                full_path, 
                sizeof (full_path),
                "%sshaders/compiled/DXIL/%s.dxil", 
                base_path,
                shader_filename
        );
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else
    {
        SDL_Log("ERROR: Unreconized backend shader format");
        return NULL;
    }

    size_t code_size;
    void *code = SDL_LoadFile(full_path, &code_size);
    if (code == NULL)
    {
        SDL_Log("ERROR: Failed to load shader from disk: %s", full_path);
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

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
    if (shader == NULL)
    {
        SDL_Log("ERROR: Failed to create shader.");
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);

    return shader;
}
