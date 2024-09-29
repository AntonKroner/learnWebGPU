#ifndef DepthStencilState_H_
#define DepthStencilState_H_

#include "webgpu.h"

WGPUDepthStencilState Application_DepthStencilState_make() {
  WGPUStencilFaceState face = {
    .compare = WGPUCompareFunction_Always,
    .failOp = WGPUStencilOperation_Keep,
    .depthFailOp = WGPUStencilOperation_Keep,
    .passOp = WGPUStencilOperation_Keep,
  };
  WGPUDepthStencilState result = {
    .nextInChain = 0,
    .format = WGPUTextureFormat_Undefined,
    .depthWriteEnabled = false,
    .depthCompare = WGPUCompareFunction_Always,
    .stencilFront = face,
    .stencilBack = face,
    .stencilReadMask = 0,
    .stencilWriteMask = 0,
    .depthBias = 0,
    .depthBiasSlopeScale = 0,
    .depthBiasClamp = 0,
  };
  return result;
}

#endif // DepthStencilState_H_
