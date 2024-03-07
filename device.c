#ifndef device_H_
#define device_H_

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "library/webgpu.h"
#include <string.h>

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
static char* errorToString(WGPUErrorType error) {
  switch (error) {
    case WGPUErrorType_NoError:
      return "WGPUErrorType_NoError";
    case WGPUErrorType_Validation:
      return "WGPUErrorType_Validation";
    case WGPUErrorType_OutOfMemory:
      return "WGPUErrorType_OutOfMemory";
    case WGPUErrorType_Internal:
      return "WGPUErrorType_Internal";
    case WGPUErrorType_Unknown:
      return "WGPUErrorType_Unknown";
    case WGPUErrorType_DeviceLost:
      return "WGPUErrorType_DeviceLost";
    case WGPUErrorType_Force32:
      return "WGPUErrorType_Force32";
  }
}
static void onDeviceError(WGPUErrorType type, const char* message, void* /* pUserData */) {
  printf("Uncaptured device error: %s\n", errorToString(type));
  if (message) {
    printf("%s ", message);
  }
  printf("\n");
  abort();
}
WGPUDevice device_request(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor) {
  DeviceData userData = { .device = 0, .requestEnded = 0 };
  wgpuAdapterRequestDevice(adapter, descriptor, device_onRequest, (void*)&userData);
  assert(userData.requestEnded);
  wgpuDeviceSetUncapturedErrorCallback(userData.device, onDeviceError, 0);
  return userData.device;
}
static char* readFile(const char* filename) {
  char* result = 0;
  FILE* input = fopen(filename, "rb");
  if (!input) {
    result = 0;
  }
  else {
    const size_t maxSize = 10 * 1024 * 1024;
    size_t size = BUFSIZ;
    result = (char*)malloc(size * sizeof(*result));
    size_t total = 0;
    while (!feof(input) && !ferror(input) && (maxSize > size)) {
      if (total + BUFSIZ > size) {
        size = size * 2;
        result = (char*)realloc(result, size);
      }
      char* buffer = result + total;
      total += fread(buffer, 1, BUFSIZ, input);
    }
    fclose(input);
    result = (char*)realloc(result, total + 1);
    result[total] = '\0';
  }
  return result;
}
WGPUShaderModule device_ShaderModule(WGPUDevice device, const char* path) {
  char* shader = readFile(path);
  if (!shader) {
    fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
    return 0;
  }
  WGPUShaderModuleWGSLDescriptor shaderCodeDescriptor = {
    .chain.next = 0,
    .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
    .code = shader,
  };
  WGPUShaderModuleDescriptor shaderDescriptor = {
    .nextInChain = &shaderCodeDescriptor.chain,
    .label = path,
  };
  WGPUShaderModule result = wgpuDeviceCreateShaderModule(device, &shaderDescriptor);
  free(shader);
  return result;
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
