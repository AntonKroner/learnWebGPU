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
    printf("WGPU instance: %zu \n", instance);
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
      WGPUQueue queue = wgpuDeviceGetQueue(device);
      wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, 0);
      WGPUCommandEncoderDescriptor encoderDesc = { .nextInChain = 0,
                                                   .label = "command encoder 1" };
      WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
      wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
      wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

      WGPUCommandBufferDescriptor cmdBufferDescriptor = { .nextInChain = 0,
                                                          .label = "command buffer 1" };
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
      wgpuCommandEncoderRelease(encoder);
      wgpuQueueSubmit(queue, 1, &command);
      wgpuCommandBufferRelease(command);

      WGPUSwapChainDescriptor swapChainDesc = {};
      swapChainDesc.nextInChain = nullptr;
      swapChainDesc.width = 640;
      swapChainDesc.height = 480;

      while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
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
