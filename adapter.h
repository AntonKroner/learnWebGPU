#ifndef adapter_H_
#define adapter_H_

#include "./library/webgpu.h"

WGPUAdapter adapter_request(
  WGPUInstance instance,
  const WGPURequestAdapterOptions* options);
void adapter_inspect(WGPUAdapter adapter);

#endif // adapter_H_
