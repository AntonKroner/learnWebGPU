#ifndef DepthBuffer_H_
#define DepthBuffer_H_

#include "webgpu.h"

typedef struct {
    WGPUTextureFormat format;
    WGPUTexture texture;
    WGPUTextureView view;
} Application_Depth;

Application_Depth Application_Depth_attach(WGPUDevice device, int width, int height) {
  Application_Depth result = { .format = WGPUTextureFormat_Depth24Plus };
  WGPUTextureDescriptor depthTextureDescriptor = {
    .dimension = WGPUTextureDimension_2D,
    .format = result.format,
    .mipLevelCount = 1,
    .sampleCount = 1,
    .size = {width, height, 1},
    .usage = WGPUTextureUsage_RenderAttachment,
    .viewFormatCount = 1,
    .viewFormats = &result.format,
  };
  result.texture = wgpuDeviceCreateTexture(device, &depthTextureDescriptor);
  WGPUTextureViewDescriptor depthTextureViewDescriptor = {
    .aspect = WGPUTextureAspect_DepthOnly,
    .baseArrayLayer = 0,
    .arrayLayerCount = 1,
    .baseMipLevel = 0,
    .mipLevelCount = 1,
    .dimension = WGPUTextureViewDimension_2D,
    .format = result.format,
  };
  result.view = wgpuTextureCreateView(result.texture, &depthTextureViewDescriptor);
  return result;
}
void Application_Depth_detach(Application_Depth depth) {
  wgpuTextureDestroy(depth.texture);
  wgpuTextureRelease(depth.texture);
  wgpuTextureViewRelease(depth.view);
}

#endif // DepthBuffer_H_
