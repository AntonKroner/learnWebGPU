#ifndef Application_Lightning_H_
#define Application_Lightning_H_

#include <stdio.h>
#include <stdbool.h>
#include "webgpu.h"

typedef struct {
    float directions[2][4];
    float colors[2][4];
} Application_Lighting_Uniforms;

typedef struct {
    bool update;
    WGPUBuffer buffer;
    Application_Lighting_Uniforms uniforms;
} Application_Lighting;

Application_Lighting Application_Lightning_create(WGPUDevice device) {
  WGPUBufferDescriptor bufferDescriptor = {
    .size = sizeof(Application_Lighting_Uniforms),
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
    .mappedAtCreation = false
  };
  Application_Lighting result = {
    .update = true,
    .buffer = wgpuDeviceCreateBuffer(device, &bufferDescriptor),
    .uniforms = {.colors = { { 0.5f, -0.9f, 0.1f, 1.0f }, { 1.0f, 0.4f, 0.3f, 1.0f } },
                 .directions = { { 1.0f, 0.9f, 0.6f, 1.0f }, { 0.6f, 0.9f, 1.0f, 1.0f } }}
  };
  // maybe check if the created buffer is null?
  return result;
}
void Application_Lightning_destroy(Application_Lighting lightning) {
  wgpuBufferDestroy(lightning.buffer);
  wgpuBufferRelease(lightning.buffer);
}
void Application_Lightning_update(
  Application_Lighting lightning[static 1],
  WGPUQueue queue) {
  if (lightning->update) {
    wgpuQueueWriteBuffer(
      queue,
      lightning->buffer,
      0,
      &lightning->uniforms,
      sizeof(Application_Lighting_Uniforms));
    // printf("update lightning\n");
    lightning->update = false;
  }
}

#endif // Application_Lightning_H_
