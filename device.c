#ifndef device_H_
#define device_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "library/webgpu.h"

typedef struct {
    WGPUDevice device;
    bool requestEnded;
} DeviceData;

static void device_onRequest(
  WGPURequestDeviceStatus status,
  WGPUDevice device,
  const char* message,
  void* pUserData) {
  DeviceData* userData = (DeviceData*)pUserData;
  if (status == WGPURequestDeviceStatus_Success) {
    userData->device = device;
  }
  else {
    printf("Could not get WebGPU device: %s\n", message);
  }
  userData->requestEnded = true;
}
static void onDeviceError(WGPUErrorType type, const char* message, void* /* pUserData */) {
  printf("Uncaptured device error: type %u", type);
  if (message) {
    printf("%s ", message);
  }
  printf("\n");
}
WGPUDevice device_request(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor) {
  DeviceData userData = { .device = 0, .requestEnded = 0 };
  wgpuAdapterRequestDevice(adapter, descriptor, device_onRequest, (void*)&userData);
  assert(userData.requestEnded);
  wgpuDeviceSetUncapturedErrorCallback(userData.device, onDeviceError, 0);
  return userData.device;
}

void device_inspect(WGPUDevice device) {
  size_t featureCount = wgpuDeviceEnumerateFeatures(device, 0);
  WGPUFeatureName* features = calloc(featureCount, sizeof(*features));
  wgpuDeviceEnumerateFeatures(device, features);
  printf("Device Features: \n");
  for (size_t i = 0; featureCount > i; i++) {
    printf("- %i \n", features[i]);
  }
  free(features);
  WGPUSupportedLimits limits = { .nextInChain = 0 };
  if (wgpuDeviceGetLimits(device, &limits)) {
    printf("Device limits: \n");
    printf("- maxTextureDimension1D: %u \n", limits.limits.maxTextureDimension1D);
    printf("- maxTextureDimension2D: %u \n", limits.limits.maxTextureDimension2D);
    printf("- maxTextureDimension3D: %u \n", limits.limits.maxTextureDimension3D);
    printf("- maxTextureArrayLayers: %u \n", limits.limits.maxTextureArrayLayers);
    printf("- maxBindGroups: %u \n", limits.limits.maxBindGroups);
    printf(
      "- maxDynamicUniformBuffersPerPipelineLayout: %u \n",
      limits.limits.maxDynamicUniformBuffersPerPipelineLayout);
    printf(
      "- maxDynamicStorageBuffersPerPipelineLayout: %u \n",
      limits.limits.maxDynamicStorageBuffersPerPipelineLayout);
    printf(
      "- maxSampledTexturesPerShaderStage: %u \n",
      limits.limits.maxSampledTexturesPerShaderStage);
    printf("- maxSamplersPerShaderStage: %u \n", limits.limits.maxSamplersPerShaderStage);
    printf(
      "- maxStorageBuffersPerShaderStage: %u \n",
      limits.limits.maxStorageBuffersPerShaderStage);
    printf(
      "- maxStorageTexturesPerShaderStage: %u \n",
      limits.limits.maxStorageTexturesPerShaderStage);
    printf(
      "- maxUniformBuffersPerShaderStage: %u \n",
      limits.limits.maxUniformBuffersPerShaderStage);
    printf(
      "- maxUniformBufferBindingSize: %zu \n",
      limits.limits.maxUniformBufferBindingSize);
    printf(
      "- maxStorageBufferBindingSize: %zu \n",
      limits.limits.maxStorageBufferBindingSize);
    printf(
      "- minUniformBufferOffsetAlignment: %u \n",
      limits.limits.minUniformBufferOffsetAlignment);
    printf(
      "- minStorageBufferOffsetAlignment: %u \n",
      limits.limits.minStorageBufferOffsetAlignment);
    printf("- maxVertexBuffers: %u \n", limits.limits.maxVertexBuffers);
    printf("- maxVertexAttributes: %u \n", limits.limits.maxVertexAttributes);
    printf("- maxVertexBufferArrayStride: %u \n", limits.limits.maxVertexBufferArrayStride);
    printf(
      "- maxInterStageShaderComponents: %u \n",
      limits.limits.maxInterStageShaderComponents);
    printf(
      "- maxComputeWorkgroupStorageSize: %u \n",
      limits.limits.maxComputeWorkgroupStorageSize);
    printf(
      "- maxComputeInvocationsPerWorkgroup: %u \n",
      limits.limits.maxComputeInvocationsPerWorkgroup);
    printf("- maxComputeWorkgroupSizeX: %u \n", limits.limits.maxComputeWorkgroupSizeX);
    printf("- maxComputeWorkgroupSizeY: %u \n", limits.limits.maxComputeWorkgroupSizeY);
    printf("- maxComputeWorkgroupSizeZ: %u \n", limits.limits.maxComputeWorkgroupSizeZ);
    printf(
      "- maxComputeWorkgroupsPerDimension: %u \n",
      limits.limits.maxComputeWorkgroupsPerDimension);
  }
}

#endif // device_H_
