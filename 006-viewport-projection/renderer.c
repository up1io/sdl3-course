#include "renderer.h"
#include "app.h"
#include "math.h"
#include "utils.h"

int 
APP_InitRenderer(struct APP_Context *ctx) 
{
    SDL_GPUShader *vertex_shader = APP_LoadShader(ctx, "PositionColorTransform.vert", 0, 1, 0, 0);
    if (vertex_shader == NULL) 
    {
        SDL_Log("ERROR: Failed to create 'PositionColorTransform' vertext shader.");
        return -1;
    }

    SDL_GPUShader *frag_shader = APP_LoadShader(ctx, "default.frag", 0, 1, 0, 0);
    if (frag_shader == NULL) 
    {
        SDL_Log("ERROR: Failed to create fragment shader.");
        return -1;
    }

    ctx->pipeline = APP_CreateGraphicsPipeline(ctx, vertex_shader, frag_shader);
    if (ctx->pipeline == NULL) 
    {
        SDL_Log("ERROR: Failed to create gpu graphics pipeline. %s", SDL_GetError());
        return -1;
    }

    SDL_ReleaseGPUShader(ctx->device, vertex_shader);
    SDL_ReleaseGPUShader(ctx->device, frag_shader);

    ctx->scene_vertex_buffer = APP_CreateVertexBuffer(ctx);
    ctx->scene_index_buffer = APP_CreateIndexBuffer(ctx);

    APP_CreateAndSubmitCube(ctx);

    ctx->time = 0;

    return 0;
}

void 
APP_CreateAndSubmitCube(struct APP_Context *ctx) 
{
    SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (sizeof(struct APP_PositionColorVertex) * 24) + (sizeof(Uint16) * 36)
    };

    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
            ctx->device, 
            &transfer_buffer_create_info
            );

    struct APP_PositionColorVertex *transfer_data = SDL_MapGPUTransferBuffer(
            ctx->device, 
            transfer_buffer, 
            false
            );

    transfer_data[0] = (struct APP_PositionColorVertex){-10, -10, -10, 255, 0, 0, 255};
    transfer_data[1] = (struct APP_PositionColorVertex){10, -10, -10, 255, 0, 0, 255};
    transfer_data[2] = (struct APP_PositionColorVertex){10, 10, -10, 255, 0, 0, 255};
    transfer_data[3] = (struct APP_PositionColorVertex){-10, 10, -10, 255, 0, 0, 255};

    transfer_data[4] = (struct APP_PositionColorVertex){-10, -10, 10, 255, 0, 0, 255};
    transfer_data[5] = (struct APP_PositionColorVertex){10, -10, 10, 255, 0, 0, 255};
    transfer_data[6] = (struct APP_PositionColorVertex){10, 10, 10, 255, 0, 0, 255};
    transfer_data[7] = (struct APP_PositionColorVertex){-10, 10, 10, 255, 0, 0, 255};

    transfer_data[8] = (struct APP_PositionColorVertex){-10, -10, -10, 255, 0, 0, 255};
    transfer_data[9] = (struct APP_PositionColorVertex){-10, 10, -10, 255, 0, 0, 255};
    transfer_data[10] = (struct APP_PositionColorVertex){-10, 10, 10, 255, 0, 0, 255};
    transfer_data[11] = (struct APP_PositionColorVertex){-10, -10, 10, 255, 0, 0, 255};

    transfer_data[12] = (struct APP_PositionColorVertex){10, -10, -10, 0, 255, 0, 255};
    transfer_data[13] = (struct APP_PositionColorVertex){10, 10, -10, 0, 255, 0, 255};
    transfer_data[14] = (struct APP_PositionColorVertex){10, 10, 10, 0, 255, 0, 255};
    transfer_data[15] = (struct APP_PositionColorVertex){10, -10, 10, 0, 255, 0, 255};

    transfer_data[16] = (struct APP_PositionColorVertex){-10, -10, -10, 255, 0, 0, 255};
    transfer_data[17] = (struct APP_PositionColorVertex){-10, -10, 10, 255, 0, 0, 255};
    transfer_data[18] = (struct APP_PositionColorVertex){10, -10, 10, 255, 0, 0, 255};
    transfer_data[19] = (struct APP_PositionColorVertex){10, -10, -10, 255, 0, 0, 255};

    transfer_data[20] = (struct APP_PositionColorVertex){-10, 10, -10, 255, 0, 0, 255};
    transfer_data[21] = (struct APP_PositionColorVertex){-10, 10, 10, 255, 0, 0, 255};
    transfer_data[22] = (struct APP_PositionColorVertex){10, 10, 10, 255, 0, 0, 255};
    transfer_data[23] = (struct APP_PositionColorVertex){10, 10, -10, 255, 0, 0, 255};

    Uint16 *index_data = (Uint16 *)&transfer_data[24];
    Uint16 indices[] = {
        0,  1,  2,  0,  2,  3,  
        4,  5,  6,  4,  6,  7, 
        8,  9,  10, 8,  10, 11, 
        12, 13, 14, 12, 14, 15, 
        16, 17, 18, 16, 18, 19, 
        20, 21, 22, 20, 22, 23
    };

    SDL_memcpy(index_data, indices, sizeof(indices));

    SDL_UnmapGPUTransferBuffer(ctx->device, transfer_buffer);
    SDL_GPUCommandBuffer *upload_cmd_buffer = SDL_AcquireGPUCommandBuffer(ctx->device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buffer);

    SDL_UploadToGPUBuffer(
            copy_pass,
            &(SDL_GPUTransferBufferLocation){
                .transfer_buffer = transfer_buffer,
                .offset = 0
            },
            &(SDL_GPUBufferRegion){
                .buffer = ctx->scene_vertex_buffer,
                .offset = 0,
                .size = sizeof(struct APP_PositionColorVertex) * 24
            },
            false
    );

    SDL_UploadToGPUBuffer(
            copy_pass,
            &(SDL_GPUTransferBufferLocation){
                .transfer_buffer = transfer_buffer,
                .offset = sizeof(struct APP_PositionColorVertex) * 24
                },
            &(SDL_GPUBufferRegion){
                .buffer = ctx->scene_index_buffer,
                .offset = 0,
                .size = sizeof(Uint16) * 36
            },
            false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd_buffer);
    SDL_ReleaseGPUTransferBuffer(ctx->device, transfer_buffer);
}

SDL_GPUBuffer*
APP_CreateVertexBuffer(struct APP_Context *ctx) 
{
    SDL_GPUBufferCreateInfo buffer_create_info = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(struct APP_PositionColorVertex) * 24
    };

    return SDL_CreateGPUBuffer(ctx->device, &buffer_create_info);
}

SDL_GPUBuffer*
APP_CreateIndexBuffer(struct APP_Context *ctx) 
{
    SDL_GPUBufferCreateInfo buffer_create_info = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX, 
        .size = sizeof(Uint16) * 36
    };

    return SDL_CreateGPUBuffer(ctx->device, &buffer_create_info);
}

SDL_GPUGraphicsPipeline*
APP_CreateGraphicsPipeline(
        struct APP_Context *ctx,
        SDL_GPUShader *vertex_shader,
        SDL_GPUShader *fragment_shader
) 
{
    SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions =
                (SDL_GPUColorTargetDescription[]){
                    {
                        .format = SDL_GetGPUSwapchainTextureFormat(ctx->device, ctx->window)
                    }
                },
        },
        .rasterizer_state = (SDL_GPURasterizerState) {
            .cull_mode = SDL_GPU_CULLMODE_NONE,
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .vertex_input_state = (SDL_GPUVertexInputState){
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){
                {
                    .slot = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .instance_step_rate = 0,
                    .pitch = sizeof(struct APP_PositionColorVertex),
                }
            },
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                    .location = 0,
                    .offset = 0
                },
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 1,
                    .offset = sizeof(float) * 3
                }
            }
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader
    };

    return SDL_CreateGPUGraphicsPipeline(ctx->device, &pipeline_create_info);
}

int 
APP_Draw(struct APP_Context *ctx) 
{
    SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(ctx->device);
    if (cmd_buffer == NULL) 
    {
      SDL_Log("ERROR: Failed to acquire gpu cmd buffer. %s", SDL_GetError());
      return -1;
    }

    SDL_GPUTexture *swapchain_texture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buffer, ctx->window, &swapchain_texture, NULL, NULL)) 
    {
        SDL_Log("ERROR: Failed to acquire swapchain texture. %s", SDL_GetError());
        return -1;
    }

    if (swapchain_texture != NULL) {
        float near_plane = 20.0f;
        float far_plane = 60.0f;

        struct APP_Matrix4x4 proj = APP_Matrix4x4_CreatePerspectiveFieldOfView(
                75.0f * SDL_PI_F / 180.0f, 600 / (float)400, 
                near_plane, 
                far_plane
        );

        struct APP_Matrix4x4 view = APP_Matrix4x4_CreateLookAt(
                (struct APP_Vector3) { SDL_cosf(ctx->time) * 30, 30, SDL_sinf(ctx->time) * 30 },
                (struct APP_Vector3) { 0, 0, 0 },
                (struct APP_Vector3) { 0, 2, 0 }
        );

        struct APP_Matrix4x4 view_proj = APP_Matrix4x4_Mutliply(view, proj);

        SDL_GPUColorTargetInfo color_target_info = { 0 };
        color_target_info.texture = swapchain_texture;
        color_target_info.clear_color = (SDL_FColor) { 0.0f, 0.0f, 0.0f, 0.0f };
        color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target_info.store_op = SDL_GPU_STOREOP_STORE;

        SDL_PushGPUVertexUniformData(cmd_buffer, 0, &view_proj, sizeof(view_proj));
        SDL_PushGPUFragmentUniformData(cmd_buffer, 0, (float[]) { near_plane, far_plane}, 8);

        SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmd_buffer, &color_target_info, 1, NULL);
        SDL_BindGPUGraphicsPipeline(render_pass, ctx->pipeline);

        SDL_BindGPUVertexBuffers(
                render_pass, 
                0, 
                &(SDL_GPUBufferBinding){
                    .buffer = ctx->scene_vertex_buffer, 
                    .offset = 0 
                }, 
                1
        );

        SDL_BindGPUIndexBuffer(
                render_pass, 
                &(SDL_GPUBufferBinding){ 
                    .buffer = ctx->scene_index_buffer, 
                    .offset = 0 
                }, 
                SDL_GPU_INDEXELEMENTSIZE_16BIT
        );


        SDL_DrawGPUIndexedPrimitives(render_pass, 36, 1, 0, 0, 0);
        SDL_EndGPURenderPass(render_pass);
    }

    SDL_SubmitGPUCommandBuffer(cmd_buffer);

    return 0;
}
