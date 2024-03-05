#include "../basic3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../../library/glfw/include/GLFW/glfw3.h"
#include "../../library/webgpu.h"
#include "../../library/glfw3webgpu/glfw3webgpu.h"
#include "../../adapter.h"
#include "../../device.h"
typedef float Point[2];
static Point points[] = {
  {  0.5,   0.0},
  {  1.0, 0.866},
  {  0.0, 0.866},
  { 0.75, 0.433},
  { 1.25, 0.433},
  {  1.0, 0.866},
  {  1.0,   0.0},
  { 1.25, 0.433},
  { 0.75, 0.433},
  { 1.25, 0.433},
  {1.375,  0.65},
  {1.125,  0.65},
  {1.125,  0.65},
  {1.375,  0.65},
  { 1.25, 0.866}
};
typedef float Color[3];
static Color colors[] = {
  {0.0, 0.353, 0.612},
  {0.0, 0.353, 0.612},
  {0.0, 0.353, 0.612},
  {0.0,   0.4,   0.7},
  {0.0,   0.4,   0.7},
  {0.0,   0.4,   0.7},
  {0.0, 0.463,   0.8},
  {0.0, 0.463,   0.8},
  {0.0, 0.463,   0.8},
  {0.0, 0.525,  0.91},
  {0.0, 0.525,  0.91},
  {0.0, 0.525,  0.91},
  {0.0, 0.576,   1.0},
  {0.0, 0.576,   1.0},
  {0.0, 0.576,   1.0}
};
static uint16_t indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

typedef struct {
    float color[4];
    float time;
		float _pad[3]
} Uniforms;

static bool setWindowHints() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return true;
}
bool basic3d_uniforms_more() {
  bool result = false;
  WGPUInstanceDescriptor descriptor = { .nextInChain = 0 };
  WGPUInstance instance = 0;
  GLFWwindow* window = 0;
  if (!glfwInit()) {
    perror("Could not initialize GLFW!");
  }
  else if (!(instance = wgpuCreateInstance(&descriptor))) {
    glfwTerminate();
    perror("Could not initialize WebGPU!");
  }
  else if (
    setWindowHints()
    && !(window = glfwCreateWindow(640, 480, "Basic 3D: First Uniform", NULL, NULL))) {
    wgpuInstanceRelease(instance);
    glfwTerminate();
    perror("Could not open window!");
    result = false;
  }
  else {
    WGPUSurface surface = glfwGetWGPUSurface(instance, window);
    WGPURequestAdapterOptions adapterOptions = {
      .nextInChain = 0,
      .compatibleSurface = surface,
    };
    WGPUAdapter adapter = adapter_request(instance, &adapterOptions);
    WGPUSupportedLimits supported = { .nextInChain = 0 };
    wgpuAdapterGetLimits(adapter, &supported);
    WGPURequiredLimits required = { .nextInChain = 0, .limits = supported.limits };
    required.limits.maxVertexAttributes = 2;
    required.limits.maxVertexBuffers = 2;
    required.limits.maxBufferSize = 15 * 5 * sizeof(float);
    required.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
    required.limits.minStorageBufferOffsetAlignment =
      supported.limits.minStorageBufferOffsetAlignment;
    required.limits.minUniformBufferOffsetAlignment =
      supported.limits.minUniformBufferOffsetAlignment;
    required.limits.maxInterStageShaderComponents = 5;
    required.limits.maxBindGroups = 1;
    required.limits.maxUniformBuffersPerShaderStage = 1;
    required.limits.maxUniformBufferBindingSize = 16 * 4;

    WGPUDeviceDescriptor deviceDescriptor = {
      .nextInChain = 0,
      .label = "Device 1",
      .requiredFeaturesCount = 0,
      .requiredLimits = &required,
      .defaultQueue = { .label = "default queueuue" }
    };
    WGPUDevice device = device_request(adapter, &deviceDescriptor);
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    WGPUBufferDescriptor uniformBufferDescriptor = {
      .nextInChain = 0,
      .label = "uniformBuffer",
      .size = sizeof(Uniforms),
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
      .mappedAtCreation = false
    };

    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(device, &uniformBufferDescriptor);
    Uniforms uniforms = {
      .time = 0.0f,
      .color = {0.0f, 1.0f, 0.4f, 1.0f},
    };

    WGPUBindGroupLayoutEntry bindingLayout = {
      .buffer.nextInChain = 0,
      .buffer.type = WGPUBufferBindingType_Uniform,
      .buffer.minBindingSize = sizeof(Uniforms),
      .buffer.hasDynamicOffset = false,
      .sampler.nextInChain = 0,
      .sampler.type = WGPUSamplerBindingType_Undefined,
      .storageTexture.nextInChain = 0,
      .storageTexture.access = WGPUStorageTextureAccess_Undefined,
      .storageTexture.format = WGPUTextureFormat_Undefined,
      .storageTexture.viewDimension = WGPUTextureViewDimension_Undefined,
      .texture.nextInChain = 0,
      .texture.multisampled = false,
      .texture.sampleType = WGPUTextureSampleType_Undefined,
      .texture.viewDimension = WGPUTextureViewDimension_Undefined,
      .binding = 0,
      .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
    };
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
      .nextInChain = 0,
      .entryCount = 1,
      .entries = &bindingLayout,
    };
    WGPUBindGroupLayout bindGroupLayout =
      wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescriptor);
    WGPUPipelineLayoutDescriptor layoutDescriptor = {
      .nextInChain = 0,
      .bindGroupLayoutCount = 1,
      .bindGroupLayouts = &bindGroupLayout,
    };
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layoutDescriptor);

    WGPUBindGroupEntry binding = {
      .nextInChain = 0,
      .binding = 0,
      .buffer = uniformBuffer,
      .offset = 0,
      .size = sizeof(Uniforms),
    };
    WGPUBindGroupDescriptor bindGroupDescriptor = {
      .nextInChain = 0,
      .layout = bindGroupLayout,
      .entryCount = bindGroupLayoutDescriptor.entryCount,
      .entries = &binding,
    };

    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);

    const size_t coordinatesLength = sizeof(points) / sizeof(typeof(*points));
    WGPUBufferDescriptor coordinateBufferDescriptor = {
      .nextInChain = 0,
      .label = "coordinateBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = coordinatesLength * sizeof(Point),
      .mappedAtCreation = false,
    };
    WGPUBuffer coordinateBuffer =
      wgpuDeviceCreateBuffer(device, &coordinateBufferDescriptor);
    wgpuQueueWriteBuffer(
      queue,
      coordinateBuffer,
      0,
      points,
      coordinateBufferDescriptor.size);
    const size_t colorsLength = sizeof(colors) / sizeof(typeof(*colors));
    WGPUBufferDescriptor colorBufferDescriptor = {
      .nextInChain = 0,
      .label = "colorsBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = colorsLength * sizeof(Color),
      .mappedAtCreation = false,
    };
    WGPUBuffer colorBuffer = wgpuDeviceCreateBuffer(device, &colorBufferDescriptor);
    wgpuQueueWriteBuffer(queue, colorBuffer, 0, colors, colorBufferDescriptor.size);
    const size_t indexLength = sizeof(indices) / sizeof(typeof(*indices));
    WGPUBufferDescriptor indexBufferDescriptor = {
      .nextInChain = 0,
      .label = "indexBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index,
      .size = (indexLength * sizeof(uint16_t) + 3) & ~3,
      .mappedAtCreation = false,
    };
    WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(device, &indexBufferDescriptor);
    wgpuQueueWriteBuffer(queue, indexBuffer, 0, indices, indexBufferDescriptor.size);
    WGPUSwapChainDescriptor swapChainDescriptor = {
      .nextInChain = 0,
      .width = 640,
      .height = 480,
      .usage = WGPUTextureUsage_RenderAttachment,
      .format = WGPUTextureFormat_BGRA8Unorm,
      .presentMode = WGPUPresentMode_Fifo,
    };
    WGPUSwapChain swapChain =
      wgpuDeviceCreateSwapChain(device, surface, &swapChainDescriptor);
    WGPUShaderModule shaderModule =
      device_ShaderModule(device, RESOURCE_DIR "/uniforms/more.wgsl");
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
      .format = swapChainDescriptor.format,
      .blend = &blendState,
      .writeMask = WGPUColorWriteMask_All,
    };
    WGPUFragmentState fragmentState = {
      .nextInChain = 0,
      .module = shaderModule,
      .entryPoint = "fs_main",
      .constantCount = 0,
      .constants = 0,
      .targetCount = 1,
      .targets = &colorTarget,
    };
    WGPUVertexAttribute coordinateAttribute = {
      .shaderLocation = 0,
      .format = WGPUVertexFormat_Float32x2,
      .offset = 0,
    };
    WGPUVertexAttribute colorAttribute = {
      .shaderLocation = 1,
      .format = WGPUVertexFormat_Float32x3,
      .offset = 0,
    };
    WGPUVertexBufferLayout bufferLayouts[2] = {
      {
       .attributeCount = 1,
       .attributes = &coordinateAttribute,
       .arrayStride = sizeof(Point),
       .stepMode = WGPUVertexStepMode_Vertex,
       },
      {
       .attributeCount = 1,
       .attributes = &colorAttribute,
       .arrayStride = sizeof(Color),
       .stepMode = WGPUVertexStepMode_Vertex,
       }
    };
    const size_t bufferLayoutLength = 2;

    WGPURenderPipelineDescriptor pipelineDesc = {
      .nextInChain = 0,
      .fragment = &fragmentState,
      .vertex.bufferCount = bufferLayoutLength,
      .vertex.buffers = bufferLayouts,
      .vertex.module = shaderModule,
      .vertex.entryPoint = "vs_main",
      .vertex.constantCount = 0,
      .vertex.constants = 0,
      .primitive.topology = WGPUPrimitiveTopology_TriangleList,
      .primitive.stripIndexFormat = WGPUIndexFormat_Undefined,
      .primitive.frontFace = WGPUFrontFace_CCW,
      .primitive.cullMode = WGPUCullMode_None,
      .depthStencil = 0,
      .multisample.count = 1,
      .multisample.mask = ~0u,
      .multisample.alphaToCoverageEnabled = false,
      .layout = layout,
    };
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (!nextTexture) {
        perror("Cannot acquire next swap chain texture\n");
        break;
      }

      uniforms.time = (float)glfwGetTime();
      wgpuQueueWriteBuffer(
        queue,
        uniformBuffer,
        sizeof(uniforms.color),
        &uniforms.time,
        sizeof(float));
      uniforms.color[0] = 1.0f * uniforms.time;
      uniforms.color[1] = 0.5f * uniforms.time;
      uniforms.color[2] = 0.0f * uniforms.time;
      wgpuQueueWriteBuffer(
        queue,
        uniformBuffer,
        0,
        &uniforms.color,
        sizeof(uniforms.color));

      WGPUCommandEncoderDescriptor commandEncoderDesc = {
        .nextInChain = 0,
        .label = "Command Encoder",
      };
      WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
      WGPURenderPassColorAttachment renderPassColorAttachment = {
        .view = nextTexture,
        .resolveTarget = 0,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = (WGPUColor){0.05, 0.05, 0.05, 1.0},
      };
      WGPURenderPassDescriptor renderPassDesc = {
        .nextInChain = 0,
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
        .depthStencilAttachment = 0,
        .timestampWriteCount = 0,
        .timestampWrites = 0,
      };
      WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
      wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
      wgpuRenderPassEncoderSetVertexBuffer(
        renderPass,
        0,
        coordinateBuffer,
        0,
        coordinatesLength * sizeof(Point));
      wgpuRenderPassEncoderSetVertexBuffer(
        renderPass,
        1,
        colorBuffer,
        0,
        colorsLength * sizeof(Color));
      wgpuRenderPassEncoderSetIndexBuffer(
        renderPass,
        indexBuffer,
        WGPUIndexFormat_Uint16,
        0,
        indexLength * sizeof(uint16_t));
      wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, 0);
      wgpuRenderPassEncoderDrawIndexed(renderPass, indexLength, 1, 0, 0, 0);
      wgpuRenderPassEncoderEnd(renderPass);
      wgpuTextureViewRelease(nextTexture);
      WGPUCommandBufferDescriptor cmdBufferDescriptor = {
        .nextInChain = 0,
        .label = "Command coordinateBuffer",
      };
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
      wgpuCommandEncoderRelease(encoder);
      wgpuQueueSubmit(queue, 1, &command);
      wgpuCommandBufferRelease(command);
      wgpuSwapChainPresent(swapChain);
      wgpuDeviceTick(device);
    }
    wgpuQueueRelease(queue);
    wgpuSwapChainRelease(swapChain);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
    glfwDestroyWindow(window);
    glfwTerminate();
    result = true;
  }
  return result;
}
