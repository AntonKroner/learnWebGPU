#ifndef Fourareen_H_
#define Fourareen_H_

#include "RenderTarget.h"
#include <stdlib.h>
#include "webgpu.h"

#define EXTEND(B, T) \
  struct {           \
      struct {       \
          B super;   \
      };             \
      struct T;      \
  }

typedef EXTEND(RenderTarget, { int placeholder; }) Fourareen;

Fourareen* Fourareen_Create(
  Fourareen* result,
  WGPUDevice device,
  WGPUQueue queue,
  WGPUTextureFormat depthFormat,
  WGPUBuffer lightningBuffer,
  size_t lightningBufferSize,
  WGPUBuffer uniformBuffer,
  size_t uniformBufferSize,
  float xOffset) {
  if (result || (result = calloc(1, sizeof(*result)))) {
    RenderTarget_create(
      &result->super,
      device,
      queue,
      depthFormat,
      lightningBuffer,
      lightningBufferSize,
      uniformBuffer,
      uniformBufferSize,
      xOffset,
      RESOURCE_DIR "/lightning/specularity.wgsl",
      RESOURCE_DIR "/fourareen/fourareen.obj",
      RESOURCE_DIR "/fourareen/fourareen2K_albedo.jpg");
  }
  return result;
}

#endif // Fourareen_H_
