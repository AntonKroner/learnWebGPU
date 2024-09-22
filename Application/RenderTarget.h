#ifndef RenderTarget_H_
#define RenderTarget_H_

#include "webgpu.h"
#include "./device.h"
#include "./Model.h"
#include "./DepthStencilState.h"
#include "./BindGroupLayoutEntry.h"

typedef struct {
    WGPUShaderModule shader;
    struct {
        WGPUSampler sampler;
        WGPUTexture texture;
        WGPUTextureView view;
    } texture;
    struct {
        WGPUBuffer vertex;
        WGPUBuffer uniform;
    } buffers;
    size_t vertexCount;
    WGPURenderPipeline pipeline;
    WGPUBindGroup bindGroup;
} RenderTarget;

static void buffers_attach(
  RenderTarget target[static 1],
  WGPUDevice device,
  size_t vertexCount) {
  WGPUBufferDescriptor vertexBufferDescriptor = {
    .nextInChain = 0,
    .label = "vertex buffer",
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
    .mappedAtCreation = false,
    .size = vertexCount * sizeof(Model_Vertex),
  };
  target->buffers.vertex = wgpuDeviceCreateBuffer(device, &vertexBufferDescriptor);
  WGPUBufferDescriptor uniformBufferDescriptor = {
    .nextInChain = 0,
    .label = "uniform buffer",
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
    .mappedAtCreation = false,
    .size = 224, //sizeof(Uniforms),
  };
  target->buffers.uniform = wgpuDeviceCreateBuffer(device, &uniformBufferDescriptor);
}
static void buffers_detach(RenderTarget target[static 1]) {
  wgpuBufferDestroy(target->buffers.vertex);
  wgpuBufferRelease(target->buffers.vertex);
  wgpuBufferDestroy(target->buffers.uniform);
  wgpuBufferRelease(target->buffers.uniform);
}
static void texture_attach(
  RenderTarget target[static 1],
  WGPUDevice device,
  const char* path) {
  target->texture.texture =
    Application_device_Texture_load(device, path, &target->texture.view);
  WGPUSamplerDescriptor samplerDescriptor = {
    .addressModeU = WGPUAddressMode_ClampToEdge,
    .addressModeV = WGPUAddressMode_ClampToEdge,
    .addressModeW = WGPUAddressMode_ClampToEdge,
    .magFilter = WGPUFilterMode_Linear,
    .minFilter = WGPUFilterMode_Linear,
    .mipmapFilter = WGPUMipmapFilterMode_Linear,
    .lodMinClamp = 0.0f,
    .lodMaxClamp = 1.0f,
    .compare = WGPUCompareFunction_Undefined,
    .maxAnisotropy = 1,
  };
  target->texture.sampler = wgpuDeviceCreateSampler(device, &samplerDescriptor);
}
static void texture_detach(RenderTarget target[static 1]) {
  wgpuTextureDestroy(target->texture.texture);
  wgpuTextureRelease(target->texture.texture);
  wgpuTextureViewRelease(target->texture.view);
  wgpuSamplerRelease(target->texture.sampler);
}
RenderTarget RenderTarget_create(
  WGPUDevice device,
  WGPUQueue queue,
  WGPUTextureFormat depthFormat,
  WGPUBuffer lightningBuffer) {
  RenderTarget result = { 0 };
  result.shader =
    Application_device_ShaderModule(device, RESOURCE_DIR "/lightning/specularity.wgsl");
  texture_attach(&result, device, RESOURCE_DIR "/fourareen/fourareen2K_albedo.jpg");
  Model model = Model_load(RESOURCE_DIR "/fourareen/fourareen.obj");
  result.vertexCount = model.vertexCount;
  buffers_attach(&result, device, result.vertexCount);
  wgpuQueueWriteBuffer(
    queue,
    result.buffers.vertex,
    0,
    model.vertices,
    result.vertexCount * sizeof(Model_Vertex));
  Model_unload(&model);
  // pipeline
  WGPUBlendState blendState = {
    .color.srcFactor = WGPUBlendFactor_SrcAlpha,
    .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
    .color.operation = WGPUBlendOperation_Add,
    .alpha.srcFactor = WGPUBlendFactor_Zero,
    .alpha.dstFactor = WGPUBlendFactor_One,
    .alpha.operation = WGPUBlendOperation_Add
  };
  WGPUColorTargetState colorTarget = {
    .nextInChain = 0,
    .format = WGPUTextureFormat_BGRA8Unorm,
    .blend = &blendState,
    .writeMask = WGPUColorWriteMask_All,
  };
  WGPUFragmentState fragmentState = {
    .nextInChain = 0,
    .module = result.shader,
    .entryPoint = "fs_main",
    .constantCount = 0,
    .constants = 0,
    .targetCount = 1,
    .targets = &colorTarget,
  };
  WGPUDepthStencilState depthStencilState = Application_DepthStencilState_make();
  depthStencilState.depthCompare = WGPUCompareFunction_Less;
  depthStencilState.depthWriteEnabled = true;
  depthStencilState.format = depthFormat;
  depthStencilState.stencilReadMask = 0;
  depthStencilState.stencilWriteMask = 0;
  WGPUBindGroupLayoutEntry bindingLayouts[] = {
    Application_BindGroupLayoutEntry_make(),
    Application_BindGroupLayoutEntry_make(),
    Application_BindGroupLayoutEntry_make(),
    Application_BindGroupLayoutEntry_make(),
  };
  bindingLayouts[0].buffer.type = WGPUBufferBindingType_Uniform;
  bindingLayouts[0].buffer.minBindingSize = 224, //sizeof(Uniforms);
    bindingLayouts[1].binding = 1;
  bindingLayouts[1].visibility = WGPUShaderStage_Fragment;
  bindingLayouts[1].texture.sampleType = WGPUTextureSampleType_Float;
  bindingLayouts[1].texture.viewDimension = WGPUTextureViewDimension_2D;
  bindingLayouts[2].binding = 2;
  bindingLayouts[2].visibility = WGPUShaderStage_Fragment;
  bindingLayouts[2].sampler.type = WGPUSamplerBindingType_Filtering;
  bindingLayouts[3].binding = 3;
  bindingLayouts[3].visibility = WGPUShaderStage_Fragment;
  bindingLayouts[3].buffer.type = WGPUBufferBindingType_Uniform;
  bindingLayouts[3].buffer.minBindingSize = 80; //sizeof(Application_Lighting_Uniforms);
  WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
    .nextInChain = 0,
    .entryCount = 4,
    .entries = bindingLayouts,
  };
  WGPUBindGroupLayout bindGroupLayout =
    wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescriptor);
  WGPUPipelineLayoutDescriptor layoutDescriptor = {
    .nextInChain = 0,
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &bindGroupLayout,
  };
  WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layoutDescriptor);
  WGPUVertexAttribute vertexAttributes[] = {
    {
     // position
      .shaderLocation = 0,
     .format = WGPUVertexFormat_Float32x3,
     .offset = 0,
     },
    {
     // normal
      .shaderLocation = 1,
     .format = WGPUVertexFormat_Float32x3,
     .offset = offsetof(Model_Vertex, normal),
     },
    {
     // color
      .shaderLocation = 2,
     .format = WGPUVertexFormat_Float32x3,
     .offset = offsetof(Model_Vertex, color),
     },
    {
     // uv coordinates
      .shaderLocation = 3,
     .format = WGPUVertexFormat_Float32x2,
     .offset = offsetof(Model_Vertex, uv),
     }
  };
  WGPUVertexBufferLayout bufferLayout = {
    .attributeCount = 4,
    .attributes = vertexAttributes,
    .arrayStride = sizeof(Model_Vertex),
    .stepMode = WGPUVertexStepMode_Vertex,
  };
  WGPURenderPipelineDescriptor pipelineDesc = {
    .nextInChain = 0,
    .fragment = &fragmentState,
    .vertex.bufferCount = 1,
    .vertex.buffers = &bufferLayout,
    .vertex.module = result.shader,
    .vertex.entryPoint = "vs_main",
    .vertex.constantCount = 0,
    .vertex.constants = 0,
    .primitive.topology = WGPUPrimitiveTopology_TriangleList,
    .primitive.stripIndexFormat = WGPUIndexFormat_Undefined,
    .primitive.frontFace = WGPUFrontFace_CCW,
    .primitive.cullMode = WGPUCullMode_None,
    .depthStencil = &depthStencilState,
    .multisample.count = 1,
    .multisample.mask = ~0u,
    .multisample.alphaToCoverageEnabled = false,
    .layout = layout,
  };
  result.pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
  // bind group
  WGPUBindGroupEntry bindings[] = {
    {
     .nextInChain = 0,
     .binding = 0,
     .buffer = result.buffers.uniform,
     .offset = 0,
     .size = 224, //sizeof(Uniforms),
    },
    {
     .nextInChain = 0,
     .binding = 1,
     .textureView = result.texture.view,
     },
    {
     .nextInChain = 0,
     .binding = 2,
     .sampler = result.texture.sampler,
     },
    {
     .nextInChain = 0,
     .binding = 3,
     .buffer = lightningBuffer,
     .offset = 0,
     .size = 80, //sizeof(Application_Lighting_Uniforms),
    }
  };
  WGPUBindGroupDescriptor bindGroupDescriptor = {
    .nextInChain = 0,
    .layout = bindGroupLayout,
    .entryCount = bindGroupLayoutDescriptor.entryCount,
    .entries = bindings,
  };
  result.bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);
  return result;
}
void RenderTarget_destroy(RenderTarget target[static 1]) {
  buffers_detach(target);
  texture_detach(target);
  wgpuBindGroupRelease(target->bindGroup);
  wgpuRenderPipelineRelease(target->pipeline);
  wgpuShaderModuleRelease(target->shader);
}
void RenderTarget_render(RenderTarget target, WGPURenderPassEncoder renderPass) {
  wgpuRenderPassEncoderSetPipeline(renderPass, target.pipeline);
  wgpuRenderPassEncoderSetVertexBuffer(
    renderPass,
    0,
    target.buffers.vertex,
    0,
    target.vertexCount * sizeof(Model_Vertex));
  wgpuRenderPassEncoderSetBindGroup(renderPass, 0, target.bindGroup, 0, 0);
  wgpuRenderPassEncoderDraw(renderPass, target.vertexCount, 1, 0, 0);
}

#endif // RenderTarget_H_
