#include "../basic3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../../library/glfw/include/GLFW/glfw3.h"
#include "../../library/webgpu.h"
#include "../../library/glfw3webgpu/glfw3webgpu.h"
#include "../../adapter.h"
#include "../../device.h"

static bool setWindowHints() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return true;
}
bool basic3d_geometry_index() {
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
    && !(window = glfwCreateWindow(640, 480, "Basic 3D: Hello Triangle", NULL, NULL))) {
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
    required.limits.maxVertexAttributes = 1;
    required.limits.maxVertexBuffers = 2;
    required.limits.maxBufferSize = 6 * 3 * sizeof(float);
    required.limits.maxVertexBufferArrayStride = 3 * sizeof(float);
    required.limits.maxInterStageShaderComponents = 3;

    WGPUDeviceDescriptor deviceDescriptor = {
      .nextInChain = 0,
      .label = "Device 1",
      .requiredFeaturesCount = 0,
      .requiredLimits = &required,
      .defaultQueue = { .label = "default queueuue" }
    };
    WGPUDevice device = device_request(adapter, &deviceDescriptor);
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    float coordinates[] = {
      -0.5, -0.5, // A
      +0.5, -0.5, +0.5, +0.5, // C
      -0.5, +0.5,
    };
    const size_t coordinatesLength = sizeof(coordinates) / sizeof(typeof(*coordinates));
    WGPUBufferDescriptor coordinateBufferDescriptor = {
      .nextInChain = 0,
      .label = "coordinateBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = coordinatesLength * sizeof(float),
      .mappedAtCreation = false,
    };
    WGPUBuffer coordinateBuffer =
      wgpuDeviceCreateBuffer(device, &coordinateBufferDescriptor);
    wgpuQueueWriteBuffer(
      queue,
      coordinateBuffer,
      0,
      coordinates,
      coordinateBufferDescriptor.size);
    uint16_t indexes[] = {
      0, 1, 2, // Triangle #0
      0, 2, 3 // Triangle #1
    };
    const size_t indexLength = sizeof(indexes) / sizeof(typeof(*indexes));
    WGPUBufferDescriptor indexBufferDescriptor = {
      .nextInChain = 0,
      .label = "indexBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index,
      .size = indexLength * sizeof(uint16_t),
      .mappedAtCreation = false,
    };
    WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(device, &indexBufferDescriptor);
    wgpuQueueWriteBuffer(queue, indexBuffer, 0, indexes, indexBufferDescriptor.size);
    float colors[] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
                       1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0 };
    const size_t colorsLength = sizeof(colors) / sizeof(typeof(*colors));
    WGPUBufferDescriptor colorBufferDescriptor = {
      .nextInChain = 0,
      .label = "colorsBuffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = colorsLength * sizeof(float),
      .mappedAtCreation = false,
    };
    WGPUBuffer colorBuffer = wgpuDeviceCreateBuffer(device, &colorBufferDescriptor);
    wgpuQueueWriteBuffer(queue, colorBuffer, 0, colors, colorBufferDescriptor.size);
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
      device_ShaderModule(device, RESOURCE_DIR "/geometry/index.wgsl");
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
       .arrayStride = 2 * sizeof(float),
       .stepMode = WGPUVertexStepMode_Vertex,
       },
      {
       .attributeCount = 1,
       .attributes = &colorAttribute,
       .arrayStride = 3 * sizeof(float),
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
      .layout = 0,
    };
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (!nextTexture) {
        perror("Cannot acquire next swap chain texture\n");
        break;
      }
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
        coordinatesLength * sizeof(float));
      wgpuRenderPassEncoderSetVertexBuffer(
        renderPass,
        1,
        colorBuffer,
        0,
        colorsLength * sizeof(float));
      wgpuRenderPassEncoderSetIndexBuffer(
        renderPass,
        indexBuffer,
        WGPUIndexFormat_Uint16,
        0,
        indexLength * sizeof(uint16_t));
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
