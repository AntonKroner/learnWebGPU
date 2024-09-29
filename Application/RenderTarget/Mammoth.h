#ifndef Mammoth_H_
#define Mammoth_H_

#include <stdlib.h>
#include "webgpu.h"
#include "linear/algebra.h"
#include "./RenderTarget.h"

#define EXTEND(B, T) \
  struct {           \
      struct {       \
          B super;   \
      };             \
      struct T;      \
  }

typedef EXTEND(RenderTarget, { int placeholder; }) Mammoth;

Mammoth* Mammoth_Create(
  Mammoth* result,
  WGPUDevice device,
  WGPUQueue queue,
  WGPUTextureFormat depthFormat,
  WGPUBuffer lightningBuffer,
  size_t lightningBufferSize,
  WGPUBuffer uniformBuffer,
  size_t uniformBufferSize,
  Vector3f offset) {
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
      offset,
      RESOURCE_DIR "/lightning/specularity.wgsl",
      RESOURCE_DIR "/meshes/mammoth.obj",
      RESOURCE_DIR "/fourareen/fourareen2K_albedo.jpg");
  }
  return result;
}

#endif // Mammoth_H_
