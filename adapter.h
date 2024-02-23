#ifndef adapter_H_
#define adapter_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "library/webgpu.h"

typedef struct {
    WGPUAdapter adapter;
    bool requestEnded;
} UserData;
void adapter_onRequest(
  WGPURequestAdapterStatus status,
  WGPUAdapter adapter,
  const char* message,
  void* pUserData) {
  UserData* userData = (UserData*)pUserData;
  if (status == WGPURequestAdapterStatus_Success) {
    userData->adapter = adapter;
  }
  else {
    printf("Could not get WebGPU adapter: %s\n", message);
  }
  userData->requestEnded = true;
}
WGPUAdapter adapter_request(
  WGPUInstance instance,
  const WGPURequestAdapterOptions* options) {
  UserData userData = { .adapter = 0, .requestEnded = false };
  wgpuInstanceRequestAdapter(instance, options, adapter_onRequest, (void*)&userData);
  assert(userData.requestEnded);
  return userData.adapter;
}
const char* const adapter_featureToString(WGPUFeatureName feature) {
  switch (feature) {
    case WGPUFeatureName_Undefined:
      return "WGPUFeatureName_Undefined";
    case WGPUFeatureName_DepthClipControl:
      return "WGPUFeatureName_DepthClipControl";
    case WGPUFeatureName_Depth32FloatStencil8:
      return "WGPUFeatureName_Depth32FloatStencil8";
    case WGPUFeatureName_TimestampQuery:
      return "WGPUFeatureName_TimestampQuery";
    case WGPUFeatureName_TextureCompressionBC:
      return "WGPUFeatureName_TextureCompressionBC";
    case WGPUFeatureName_TextureCompressionETC2:
      return "WGPUFeatureName_TextureCompressionETC2";
    case WGPUFeatureName_TextureCompressionASTC:
      return "WGPUFeatureName_TextureCompressionASTC";
    case WGPUFeatureName_IndirectFirstInstance:
      return "WGPUFeatureName_IndirectFirstInstance";
    case WGPUFeatureName_ShaderF16:
      return "WGPUFeatureName_ShaderF16";
    case WGPUFeatureName_RG11B10UfloatRenderable:
      return "WGPUFeatureName_RG11B10UfloatRenderable";
    case WGPUFeatureName_BGRA8UnormStorage:
      return "WGPUFeatureName_BGRA8UnormStorage";
    case WGPUFeatureName_Float32Filterable:
      return "WGPUFeatureName_Float32Filterable";
    case WGPUFeatureName_Force32:
      return "WGPUFeatureName_Force32";
  }
  return "Unknown feature";
}
void adapter_inspect(WGPUAdapter adapter) {
  size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, 0);
  WGPUFeatureName* features = calloc(featureCount, sizeof(*features));
  wgpuAdapterEnumerateFeatures(adapter, features);
  printf("Adapter Features:\n");
  for (size_t i = 0; featureCount > i; i++) {
    printf("- %i: %s\n", features[i], adapter_featureToString(features[i]));
  }
  free(features);
  WGPUSupportedLimits limits = { .nextInChain = 0 };
  if (wgpuAdapterGetLimits(adapter, &limits)) {
    printf("Adapter limits: \n");
    printf("- maxTextureDimension1D: %u\n", limits.limits.maxTextureDimension1D);
    printf("- maxTextureDimension2D: %u\n", limits.limits.maxTextureDimension2D);
    printf("- maxTextureDimension3D: %u\n", limits.limits.maxTextureDimension3D);
    printf("- maxTextureArrayLayers: %u\n", limits.limits.maxTextureArrayLayers);
    printf("- maxBindGroups: %u\n", limits.limits.maxBindGroups);
    printf(
      "- maxDynamicUniformBuffersPerPipelineLayout: %u\n",
      limits.limits.maxDynamicUniformBuffersPerPipelineLayout);
    printf(
      "- maxDynamicStorageBuffersPerPipelineLayout: %u\n",
      limits.limits.maxDynamicStorageBuffersPerPipelineLayout);
    printf(
      "- maxSampledTexturesPerShaderStage: %u\n",
      limits.limits.maxSampledTexturesPerShaderStage);
    printf("- maxSamplersPerShaderStage: %u\n", limits.limits.maxSamplersPerShaderStage);
    printf(
      "- maxStorageBuffersPerShaderStage: %u\n",
      limits.limits.maxStorageBuffersPerShaderStage);
    printf(
      "- maxStorageTexturesPerShaderStage: %u\n",
      limits.limits.maxStorageTexturesPerShaderStage);
    printf(
      "- maxUniformBuffersPerShaderStage: %u\n",
      limits.limits.maxUniformBuffersPerShaderStage);
    printf(
      "- maxUniformBufferBindingSize: %zu\n",
      limits.limits.maxUniformBufferBindingSize);
    printf(
      "- maxStorageBufferBindingSize: %zu\n",
      limits.limits.maxStorageBufferBindingSize);
    printf(
      "- minUniformBufferOffsetAlignment: %u\n",
      limits.limits.minUniformBufferOffsetAlignment);
    printf(
      "- minStorageBufferOffsetAlignment: %u\n",
      limits.limits.minStorageBufferOffsetAlignment);
    printf("- maxVertexBuffers: %u\n", limits.limits.maxVertexBuffers);
    printf("- maxVertexAttributes: %u\n", limits.limits.maxVertexAttributes);
    printf("- maxVertexBufferArrayStride: %u\n", limits.limits.maxVertexBufferArrayStride);
    printf(
      "- maxInterStageShaderComponents: %u\n",
      limits.limits.maxInterStageShaderComponents);
    printf(
      "- maxComputeWorkgroupStorageSize: %u\n",
      limits.limits.maxComputeWorkgroupStorageSize);
    printf(
      "- maxComputeInvocationsPerWorkgroup: %u\n",
      limits.limits.maxComputeInvocationsPerWorkgroup);
    printf("- maxComputeWorkgroupSizeX: %u\n", limits.limits.maxComputeWorkgroupSizeX);
    printf("- maxComputeWorkgroupSizeY: %u\n", limits.limits.maxComputeWorkgroupSizeY);
    printf("- maxComputeWorkgroupSizeZ: %u\n", limits.limits.maxComputeWorkgroupSizeZ);
    printf(
      "- maxComputeWorkgroupsPerDimension: %u\n",
      limits.limits.maxComputeWorkgroupsPerDimension);
  }
  else {
    printf("Failed to get adapter limits.\n");
  }
  WGPUAdapterProperties properties = { .nextInChain = 0 };
  wgpuAdapterGetProperties(adapter, &properties);
  printf("Adapter properties:\n");
  printf("- vendorID: %u\n", properties.vendorID);
  printf("- deviceID: %u\n", properties.deviceID);
  printf("- name: %s\n", properties.name);
  printf("- vendorName: %s\n", properties.vendorName);
  if (properties.driverDescription) {
    printf("- driverDescription: %s\n", properties.driverDescription);
  }
  if (properties.architecture) {
    printf("- architecture: %s\n", properties.architecture);
  }
  printf("- adapterType: %u\n", properties.adapterType);
  printf("- backendType: %u\n", properties.backendType);
}
#endif // adapter_H_
