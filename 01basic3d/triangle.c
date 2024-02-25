#include "basic3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../library/glfw/include/GLFW/glfw3.h"
#include "../library/webgpu.h"
#include "../library/glfw3webgpu/glfw3webgpu.h"
#include "../adapter.h"
#include "../device.h"

static bool setWindowHints() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return true;
}
bool basic3d_triangle() {
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
    WGPUDeviceDescriptor deviceDescriptor = {
      .nextInChain = 0,
      .label = "Device 1",
      .requiredFeaturesCount = 0,
      .requiredLimits = 0,
      .defaultQueue = { .label = "default queueuue" }
    };
    WGPUDevice device = device_request(adapter, &deviceDescriptor);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
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
    const char* const shaderSource =
      "@vertex fn"
      "  vs_main(@builtin(vertex_index) in_vertex_index"
      "          : u32)"
      "    ->@builtin(position) vec4<f32> {"
      "  var p = vec2f(0.0, 0.0);"
      "  if (in_vertex_index == 0u) {"
      "    p = vec2f(-0.5, -0.5);"
      "  }"
      "  else if (in_vertex_index == 1u) {"
      "    p = vec2f(0.5, -0.5);"
      "  }"
      "  else {"
      "    p = vec2f(0.0, 0.5);"
      "  }"
      "  return vec4f(p, 0.0, 1.0);"
      "}"
      "@fragment fn fs_main()->@location(0) vec4f {"
      "  return vec4f(0.0, 0.4, 1.0, 1.0);"
      "}";
    WGPUShaderModuleWGSLDescriptor shaderCodeDescriptor = {
      .chain.next = 0,
      .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
      .code = shaderSource,
    };
    WGPUShaderModuleDescriptor shaderDescriptor = { .nextInChain =
                                                      &shaderCodeDescriptor.chain };
    WGPUShaderModule shaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDescriptor);
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
    WGPURenderPipelineDescriptor pipelineDesc = {
      .nextInChain = 0,
      .fragment = &fragmentState,
      .vertex.bufferCount = 0,
      .vertex.buffers = 0,
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
        .clearValue = (WGPUColor){0.9, 0.1, 0.2, 1.0},
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
      wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
      wgpuRenderPassEncoderEnd(renderPass);
      wgpuTextureViewRelease(nextTexture);
      WGPUCommandBufferDescriptor cmdBufferDescriptor = {
        .nextInChain = 0,
        .label = "Command buffer",
      };
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
      wgpuCommandEncoderRelease(encoder);
      wgpuQueueSubmit(queue, 1, &command);
      wgpuCommandBufferRelease(command);
      wgpuSwapChainPresent(swapChain);
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
