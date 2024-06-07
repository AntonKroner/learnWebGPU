#include "adapter.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "webgpu.h"

typedef struct {
    WGPUAdapter adapter;
    bool requestEnded;
} UserData;

static void adapter_onRequest(
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
WGPUAdapter Application_adapter_request(
  WGPUInstance instance,
  const WGPURequestAdapterOptions* options) {
  UserData userData = { .adapter = 0, .requestEnded = false };
  wgpuInstanceRequestAdapter(instance, options, adapter_onRequest, (void*)&userData);
  assert(userData.requestEnded);
  return userData.adapter;
}
#define FEATURE_STRINGIFY(feature) \
  case feature:                    \
    return #feature
static const char* adapter_featureToString(WGPUFeatureName feature) {
  switch (feature) {
    FEATURE_STRINGIFY(WGPUFeatureName_Undefined);
    FEATURE_STRINGIFY(WGPUFeatureName_DepthClipControl);
    FEATURE_STRINGIFY(WGPUFeatureName_Depth32FloatStencil8);
    FEATURE_STRINGIFY(WGPUFeatureName_TimestampQuery);
    FEATURE_STRINGIFY(WGPUFeatureName_PipelineStatisticsQuery);
    FEATURE_STRINGIFY(WGPUFeatureName_TextureCompressionBC);
    FEATURE_STRINGIFY(WGPUFeatureName_TextureCompressionETC2);
    FEATURE_STRINGIFY(WGPUFeatureName_TextureCompressionASTC);
    FEATURE_STRINGIFY(WGPUFeatureName_IndirectFirstInstance);
    FEATURE_STRINGIFY(WGPUFeatureName_ShaderF16);
    FEATURE_STRINGIFY(WGPUFeatureName_RG11B10UfloatRenderable);
    FEATURE_STRINGIFY(WGPUFeatureName_BGRA8UnormStorage);
    FEATURE_STRINGIFY(WGPUFeatureName_Float32Filterable);
    FEATURE_STRINGIFY(WGPUFeatureName_DawnShaderFloat16);
    FEATURE_STRINGIFY(WGPUFeatureName_DawnInternalUsages);
    FEATURE_STRINGIFY(WGPUFeatureName_DawnMultiPlanarFormats);
    FEATURE_STRINGIFY(WGPUFeatureName_DawnNative);
    FEATURE_STRINGIFY(WGPUFeatureName_ChromiumExperimentalDp4a);
    FEATURE_STRINGIFY(WGPUFeatureName_TimestampQueryInsidePasses);
    FEATURE_STRINGIFY(WGPUFeatureName_ImplicitDeviceSynchronization);
    FEATURE_STRINGIFY(WGPUFeatureName_SurfaceCapabilities);
    FEATURE_STRINGIFY(WGPUFeatureName_TransientAttachments);
    FEATURE_STRINGIFY(WGPUFeatureName_MSAARenderToSingleSampled);
    FEATURE_STRINGIFY(WGPUFeatureName_Force32);
  }
}
void Application_adapter_inspect(WGPUAdapter adapter) {
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
