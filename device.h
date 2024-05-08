#ifndef device_H_
#define device_H_

#include "./library/webgpu.h"

WGPUDevice device_request(WGPUAdapter adapter);
WGPUShaderModule device_ShaderModule(WGPUDevice device, const char* path);
WGPUTexture device_Texture_load(
  WGPUDevice device,
  const char* const path,
  WGPUTextureView* view);
void device_inspect(WGPUDevice device);

#endif // device_H_
