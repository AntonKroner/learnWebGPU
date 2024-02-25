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
typedef struct {
    WGPUBuffer buffer;
    size_t length;
} BufferContext;
static void onBuffer2Mapped(WGPUBufferMapAsyncStatus status, void* data) {
  BufferContext* context = (BufferContext*)data;
  printf("Buffer 2 mapped with status %u \n", status);
  if (status == WGPUBufferMapAsyncStatus_Success) {
    uint8_t* bufferData =
      (uint8_t*)wgpuBufferGetConstMappedRange(context->buffer, 0, context->length);
    for (size_t i = 0; context->length > i; i++) {
      printf("%u\n", bufferData[i]);
    }
    wgpuBufferUnmap(context->buffer);
  }
  else {
    printf("failed buffer mapped\n");
  }
}
bool basic3d_geometry_buffers() {
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
    && !(window = glfwCreateWindow(640, 480, "Basic 3D: Buffer Playing", NULL, NULL))) {
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

    const size_t length = 16;
    uint8_t data[length];
    for (size_t i = 0; length > i; i++) {
      data[i] = i;
    }
    WGPUBufferDescriptor bufferDesc = {
      .nextInChain = 0,
      .label = "buffer 1",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc,
      .size = length,
      .mappedAtCreation = false,
    };
    WGPUBuffer buffer1 = wgpuDeviceCreateBuffer(device, &bufferDesc);
    WGPUBufferDescriptor bufferDesc2 = {
      .nextInChain = 0,
      .label = "buffer 2",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead,
      .size = length,
      .mappedAtCreation = false,
    };
    WGPUBuffer buffer2 = wgpuDeviceCreateBuffer(device, &bufferDesc2);
    wgpuQueueWriteBuffer(queue, buffer1, 0, data, length);
    WGPUCommandEncoderDescriptor commandEncoderDesc = {
      .nextInChain = 0,
      .label = "Command Encoder",
    };
    WGPUCommandEncoder encoder =
      wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
    wgpuCommandEncoderCopyBufferToBuffer(encoder, buffer1, 0, buffer2, 0, length);
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {
      .nextInChain = 0,
      .label = "Command buffer",
    };
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(command);
    BufferContext context = { .buffer = buffer2, .length = length };
    wgpuBufferMapAsync(
      buffer2,
      WGPUMapMode_Read,
      0,
      sizeof(BufferContext),
      onBuffer2Mapped,
      &context);

    while (!glfwWindowShouldClose(window)) {
      wgpuDeviceTick(device);
      glfwPollEvents();
    }

    wgpuBufferDestroy(buffer1);
    wgpuBufferDestroy(buffer2);
    wgpuBufferRelease(buffer1);
    wgpuBufferRelease(buffer2);
    wgpuQueueRelease(queue);
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
