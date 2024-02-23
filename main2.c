#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <tgmath.h>
#include "library/glfw/include/GLFW/glfw3.h"
#include "library/webgpu.h"
#include "library/glfw3webgpu/glfw3webgpu.h"
#include "adapter.h"
#include "device.h"

void onQueueWorkDone(WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
  printf("Queued work finished with status: %u \n", status);
}
int main(int, char**) {
  WGPUInstanceDescriptor instanceDesc = {};
  instanceDesc.nextInChain = 0;
  WGPUInstance instance = wgpuCreateInstance(&instanceDesc);
  if (!instance) {
    printf("Could not initialize WebGPU!\n");
    return 1;
  }

  if (!glfwInit()) {
    printf("Could not initialize GLFW!\n");
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
  if (!window) {
    printf("Could not open window!\n");
    return 1;
  }

  printf("Requesting adapter...\n");
  WGPUSurface surface = glfwGetWGPUSurface(instance, window);
  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = 0;
  adapterOpts.compatibleSurface = surface;
  WGPUAdapter adapter = adapter_request(instance, &adapterOpts);

  printf("Requesting device...\n");
  WGPUDeviceDescriptor deviceDesc;
  deviceDesc.nextInChain = 0;
  deviceDesc.label = "My Device";
  deviceDesc.requiredFeaturesCount = 0;
  deviceDesc.requiredLimits = 0;
  deviceDesc.defaultQueue.label = "The default queue";
  WGPUDevice device = device_request(adapter, &deviceDesc);
  printf("Got device: ");

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  printf("Creating swapchain...\n");

  WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;
  WGPUSwapChainDescriptor swapChainDesc = {};
  swapChainDesc.nextInChain = 0;
  swapChainDesc.width = 640;
  swapChainDesc.height = 480;
  swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
  swapChainDesc.format = swapChainFormat;
  swapChainDesc.presentMode = WGPUPresentMode_Fifo;
  WGPUSwapChain swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);
  printf("Swapchain: \n");

  printf("Creating shader module...\n");
  const char* shaderSource =
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

  WGPUShaderModuleDescriptor shaderDesc = {};
  shaderDesc.nextInChain = 0;

  // Use the extension mechanism to load a WGSL shader source code
  WGPUShaderModuleWGSLDescriptor shaderCodeDesc = {};
  // Set the chained struct's header
  shaderCodeDesc.chain.next = 0;
  shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  // Connect the chain
  shaderDesc.nextInChain = &shaderCodeDesc.chain;

  // Setup the actual payload of the shader code descriptor
  shaderCodeDesc.code = shaderSource;

  WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderDesc);

  printf("Creating render pipeline...\n");
  WGPURenderPipelineDescriptor pipelineDesc = {};
  pipelineDesc.nextInChain = 0;

  // Vertex fetch
  // (We don't use any input buffer so far)
  pipelineDesc.vertex.bufferCount = 0;
  pipelineDesc.vertex.buffers = 0;

  // Vertex shader
  pipelineDesc.vertex.module = shaderModule;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = 0;

  // Primitive assembly and rasterization
  // Each sequence of 3 vertices is considered as a triangle
  pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
  // We'll see later how to specify the order in which vertices should be
  // connected. When not specified, vertices are considered sequentially.
  pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  // The face orientation is defined by assuming that when looking
  // from the front of the face, its corner vertices are enumerated
  // in the counter-clockwise (CCW) order.
  pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
  // But the face orientation does not matter much because we do not
  // cull (i.e. "hide") the faces pointing away from us (which is often
  // used for optimization).
  pipelineDesc.primitive.cullMode = WGPUCullMode_None;

  // Fragment shader
  WGPUFragmentState fragmentState = {};
  fragmentState.nextInChain = 0;
  pipelineDesc.fragment = &fragmentState;
  fragmentState.module = shaderModule;
  fragmentState.entryPoint = "fs_main";
  fragmentState.constantCount = 0;
  fragmentState.constants = 0;

  // Configure blend state
  WGPUBlendState blendState;
  // Usual alpha blending for the color:
  blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.color.operation = WGPUBlendOperation_Add;
  // We leave the target alpha untouched:
  blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
  blendState.alpha.dstFactor = WGPUBlendFactor_One;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget = {};
  colorTarget.nextInChain = 0;
  colorTarget.format = swapChainFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask =
    WGPUColorWriteMask_All; // We could write to only some of the color channels.

  // We have only one target because our render pass has only one output color
  // attachment.
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;

  // Depth and stencil tests are not used here
  pipelineDesc.depthStencil = 0;

  // Multi-sampling
  // Samples per pixel
  pipelineDesc.multisample.count = 1;
  // Default value for the mask, meaning "all bits on"
  pipelineDesc.multisample.mask = ~0u;
  // Default value as well (irrelevant for count = 1 anyways)
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  // Pipeline layout
  pipelineDesc.layout = 0;

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
    if (!nextTexture) {
      printf("Cannot acquire next swap chain texture\n");
      return 1;
    }

    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    commandEncoderDesc.nextInChain = 0;
    commandEncoderDesc.label = "Command Encoder";
    WGPUCommandEncoder encoder =
      wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);

    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = 0;

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

    // In its overall outline, drawing a triangle is as simple as this:
    // Select which render pipeline to use
    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
    // Draw 1 instance of a 3-vertices shape
    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

    wgpuRenderPassEncoderEnd(renderPass);

    wgpuTextureViewRelease(nextTexture);

    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    cmdBufferDesc.nextInChain = 0;
    cmdBufferDesc.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    wgpuQueueSubmit(queue, 1, &command);

    wgpuSwapChainPresent(swapChain);
  }

  wgpuSwapChainRelease(swapChain);
  wgpuDeviceRelease(device);
  wgpuAdapterRelease(adapter);
  wgpuInstanceRelease(instance);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
