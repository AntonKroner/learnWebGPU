#ifndef device_H_
#define device_H_

#include "library/webgpu.h"

WGPUDevice device_request(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor);
WGPUShaderModule device_ShaderModule(WGPUDevice device, const char* path);
void device_inspect(WGPUDevice device);

#endif // device_H_
