#ifndef adapter_H_
#define adapter_H_

#include "webgpu.h"

WGPUAdapter Application_adapter_request(
  WGPUInstance instance,
  const WGPURequestAdapterOptions* options);
void Application_adapter_inspect(WGPUAdapter adapter);

#endif // adapter_H_
