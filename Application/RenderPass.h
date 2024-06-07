#ifndef RenderPass_H_
#define RenderPass_H_

#include "webgpu.h"

WGPURenderPassEncoder Application_RenderPassEncoder_make(
  WGPUCommandEncoder encoder,
  WGPUTextureView texture,
  WGPUTextureView depth) {
  WGPURenderPassColorAttachment renderPassColorAttachment = {
    .view = texture,
    .resolveTarget = 0,
    .loadOp = WGPULoadOp_Clear,
    .storeOp = WGPUStoreOp_Store,
    .clearValue = {.r = 0.05, .g = 0.05, .b = 0.05, .a = 1.0},
  };
  WGPURenderPassDepthStencilAttachment depthStencilAttachment = {
    .view = depth,
    .depthClearValue = 1.0f,
    .depthLoadOp = WGPULoadOp_Clear,
    .depthStoreOp = WGPUStoreOp_Store,
    .depthReadOnly = false,
    .stencilClearValue = 0,
    .stencilLoadOp = WGPULoadOp_Undefined,
    .stencilStoreOp = WGPUStoreOp_Undefined,
    .stencilReadOnly = true,
  };
  WGPURenderPassDescriptor renderPassDesc = {
    .nextInChain = 0,
    .colorAttachmentCount = 1,
    .colorAttachments = &renderPassColorAttachment,
    .depthStencilAttachment = &depthStencilAttachment,
    .timestampWriteCount = 0,
    .timestampWrites = 0,
  };
  WGPURenderPassEncoder renderPass =
    wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  return renderPass;
}

#endif // RenderPass_H_
