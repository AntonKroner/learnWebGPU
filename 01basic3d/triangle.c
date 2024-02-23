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
    result = true;
  }
  else {
    WGPUSurface surface = glfwGetWGPUSurface(instance, window);
    WGPURequestAdapterOptions adapterOptions = { .nextInChain = 0,
                                                 .compatibleSurface = surface };
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
    WGPURenderPipelineDescriptor pipelineDesc = {
      .nextInChain = 0,
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
    };
    WGPUFragmentState fragmentState = {};
    fragmentState.nextInChain = 0;
    pipelineDesc.fragment = &fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = 0;
    WGPUBlendState blendState;
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;
    WGPUColorTargetState colorTarget = {};
    colorTarget.nextInChain = 0;
    colorTarget.format = swapChainDescriptor.format;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.depthStencil = 0;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = 0;
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (!nextTexture) {
        perror("Cannot acquire next swap chain texture\n");
        break;
      }
      WGPUCommandEncoderDescriptor commandEncoderDesc = {};
      commandEncoderDesc.nextInChain = 0;
      commandEncoderDesc.label = "Command Encoder";
      WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
      WGPURenderPassDescriptor renderPassDesc = {
        .nextInChain = 0,
      };
      WGPURenderPassColorAttachment renderPassColorAttachment = {};
      renderPassColorAttachment.view = nextTexture;
      renderPassColorAttachment.resolveTarget = 0;
      renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
      renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
      renderPassColorAttachment.clearValue = (WGPUColor){ 0.9, 0.1, 0.2, 1.0 };
      renderPassDesc.colorAttachmentCount = 1;
      renderPassDesc.colorAttachments = &renderPassColorAttachment;
      renderPassDesc.depthStencilAttachment = 0;
      renderPassDesc.timestampWriteCount = 0;
      renderPassDesc.timestampWrites = 0;
      WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
      wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
      wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
      wgpuRenderPassEncoderEnd(renderPass);
      wgpuTextureViewRelease(nextTexture);
      WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
      cmdBufferDescriptor.nextInChain = 0;
      cmdBufferDescriptor.label = "Command buffer";
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
    result = EXIT_SUCCESS;
  }
  return result;
}
