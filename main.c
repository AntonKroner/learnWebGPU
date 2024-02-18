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
int main(int argc, char* argv[static argc + 1]) {
  int result = EXIT_FAILURE;
  extern char* optarg;
  int index = 0;
  int option = 0;
  int flag = 0;
  const struct option options[] = {
    {"input", required_argument,     0, 'i'},
    { "flag",       no_argument, &flag,   1},
    {      0,                 0,     0,   0}
  };
  while (option != EOF) {
    option = getopt_long(argc, argv, "", options, &index);
    switch (option) {
      case 0:
        break;
      case '?':
        printf("Error case.");
        break;
      case 'i':
        printf("input: %s\n", optarg);
    }
  }
  WGPUInstanceDescriptor descriptor = { .nextInChain = 0 };
  WGPUInstance instance = 0;
  if (!glfwInit()) {
    perror("Could not initialize GLFW!");
  }
  else if (!(instance = wgpuCreateInstance(&descriptor))) {
    glfwTerminate();
    perror("Could not initialize WebGPU!");
  }
  else {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
    if (!window) {
      perror("Could not open window!");
    }
    else {
      WGPUSurface surface = glfwGetWGPUSurface(instance, window);
      WGPURequestAdapterOptions adapterOptions = { .nextInChain = 0,
                                                   .compatibleSurface = surface };
      WGPUAdapter adapter = adapter_request(instance, &adapterOptions);
      WGPUDeviceDescriptor deviceDescriptor = {
        .nextInChain = 0,
        .label = "Device 1",
        .requiredFeatureCount = 0,
        .requiredFeatures = 0,
        .requiredLimits = 0,
        .defaultQueue = {.nextInChain = 0, .label = "queueuue"}
      };
      WGPUDevice device = device_request(adapter, &deviceDescriptor);
      WGPUShaderModuleDescriptor shaderDesc = { .hintCount = 0,
                                                .hints = 0,
                                                .label = "triangle shader" };
      WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderDesc);

      WGPURenderPipelineDescriptor pipelineDescriptor = {
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
        .depthStencil = 0,
        .multisample.count = 1,
        .multisample.mask = ~0u,
        .multisample.alphaToCoverageEnabled = false,
      };
      WGPUFragmentState fragmentState = {.fragmentState.module = shaderModule;
      .fragmentState.entryPoint = "fs_main";
      .fragmentState.constantCount = 0;
      .fragmentState.constants = 0;
    };
    pipelineDescriptor.fragment = &fragmentState;
    WGPUBlendState blendState = {
      .color.srcFactor = WGPUBlendFactor_SrcAlpha,
      .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
      .color.operation = WGPUBlendOperation_Add,
      .alpha.srcFactor = WGPUBlendFactor_Zero,
      .alpha.dstFactor = WGPUBlendFactor_One,
      .alpha.operation = WGPUBlendOperation_Add,
    };
    WGPUColorTargetState colorTarget = {
      .format = WGPUTextureFormat_R8Unorm,
      .blend = &blendState,
      .writeMask = WGPUColorWriteMask_All,
    };
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    WGPURenderPipeline pipeline =
      wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      // Get the texture where to draw the next frame
      WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      // Getting the texture may fail, in particular if the window has been resized
      // and thus the target surface changed.
      if (!nextTexture) {
        perror("Cannot acquire next swap chain texture\n");
        break;
      }
      WGPUCommandEncoderDescriptor commandEncoderDesc = {};
      commandEncoderDesc.nextInChain = 0;
      commandEncoderDesc.label = "Command Encoder";
      WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);

      // Describe a render pass, which targets the texture view
      WGPURenderPassDescriptor renderPassDesc = {};

      WGPURenderPassColorAttachment renderPassColorAttachment = {};
      // The attachment is tighed to the view returned by the swap chain, so that
      // the render pass draws directly on screen.
      renderPassColorAttachment.view = nextTexture;
      // Not relevant here because we do not use multi-sampling
      renderPassColorAttachment.resolveTarget = 0;
      renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
      renderPassColorAttachment.storeOp = WGPUStoreOp_Store;

      renderPassColorAttachment.clearValue = (WGPUColor){ 0.9, 0.1, 0.2, 1.0 };
      renderPassDesc.colorAttachmentCount = 1;
      renderPassDesc.colorAttachments = &renderPassColorAttachment;

      // No depth buffer for now
      renderPassDesc.depthStencilAttachment = 0;

      // We do not use timers for now neither
      renderPassDesc.timestampWriteCount = 0;
      renderPassDesc.timestampWrites = 0;

      renderPassDesc.nextInChain = 0;

      // Create a render pass. We end it immediately because we use its built-in
      // mechanism for clearing the screen when it begins (see descriptor).
      WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
      wgpuRenderPassEncoderEnd(renderPass);
      wgpuRenderPassEncoderRelease(renderPass);

      wgpuTextureViewRelease(nextTexture);

      WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
      cmdBufferDescriptor.nextInChain = 0;
      cmdBufferDescriptor.label = "Command buffer";
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
      wgpuCommandEncoderRelease(encoder);
      wgpuQueueSubmit(queue, 1, &command);
      wgpuCommandBufferRelease(command);

      // We can tell the swap chain to present the next texture.
      wgpuSurfacePresent();
    }
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
    wgpuSurfaceRelease(surface);
    glfwDestroyWindow(window);
  }
  wgpuInstanceRelease(instance);
  glfwTerminate();
  result = EXIT_SUCCESS;
}
return result;
}
