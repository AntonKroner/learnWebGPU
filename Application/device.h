#ifndef device_H_
#define device_H_

#include "webgpu.h"

WGPUDevice Application_device_request(WGPUAdapter adapter);
WGPUShaderModule Application_device_ShaderModule(WGPUDevice device, const char* path);
WGPUTexture Application_device_Texture_load(
  WGPUDevice device,
  const char* const path,
  WGPUTextureView* view);
void Application_device_inspect(WGPUDevice device);

#endif // device_H_
