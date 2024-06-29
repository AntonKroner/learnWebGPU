#include "adapter.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "webgpu.h"

typedef struct {
    WGPUAdapter adapter;
    bool done;
} Response;

static void adapter_onRequest(
  WGPURequestAdapterStatus status,
  WGPUAdapter adapter,
  const char* message,
  void* inputResponse) {
  Response* response = (Response*)inputResponse;
  if (status != WGPURequestAdapterStatus_Success) {
    printf("Could not get WebGPU adapter: %s\n", message);
  }
  else {
    response->adapter = adapter;
  }
  response->done = true;
}
WGPUAdapter Application_adapter_request(
  WGPUInstance instance,
  const WGPURequestAdapterOptions* options) {
  Response userData = { .adapter = 0, .done = false };
  wgpuInstanceRequestAdapter(instance, options, adapter_onRequest, (void*)&userData);
  assert(userData.done);
  return userData.adapter;
}
static const char* featureStringify(WGPUFeatureName feature);
static const char* typeStringify(WGPUAdapterType type);
static const char* backendStringify(WGPUBackendType type);
void Application_adapter_inspect(WGPUAdapter adapter) {
  size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, 0);
  WGPUFeatureName* features = calloc(featureCount, sizeof(*features));
  wgpuAdapterEnumerateFeatures(adapter, features);
  printf("Adapter Features:\n");
  for (size_t i = 0; featureCount > i; i++) {
    printf("- %i: %s\n", features[i], featureStringify(features[i]));
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
  printf("- adapterType: %s\n", typeStringify(properties.adapterType));
  printf("- backendType: %s\n", backendStringify(properties.backendType));
  printf("- compatibility mode: %u\n", properties.compatibilityMode);
}
#define STRINGIFY(value) \
  case value:            \
    return #value
static const char* featureStringify(WGPUFeatureName feature) {
  switch (feature) {
    STRINGIFY(WGPUFeatureName_Undefined);
    STRINGIFY(WGPUFeatureName_DepthClipControl);
    STRINGIFY(WGPUFeatureName_Depth32FloatStencil8);
    STRINGIFY(WGPUFeatureName_TimestampQuery);
    STRINGIFY(WGPUFeatureName_PipelineStatisticsQuery);
    STRINGIFY(WGPUFeatureName_TextureCompressionBC);
    STRINGIFY(WGPUFeatureName_TextureCompressionETC2);
    STRINGIFY(WGPUFeatureName_TextureCompressionASTC);
    STRINGIFY(WGPUFeatureName_IndirectFirstInstance);
    STRINGIFY(WGPUFeatureName_ShaderF16);
    STRINGIFY(WGPUFeatureName_RG11B10UfloatRenderable);
    STRINGIFY(WGPUFeatureName_BGRA8UnormStorage);
    STRINGIFY(WGPUFeatureName_Float32Filterable);
    STRINGIFY(WGPUFeatureName_DawnShaderFloat16);
    STRINGIFY(WGPUFeatureName_DawnInternalUsages);
    STRINGIFY(WGPUFeatureName_DawnMultiPlanarFormats);
    STRINGIFY(WGPUFeatureName_DawnNative);
    STRINGIFY(WGPUFeatureName_ChromiumExperimentalDp4a);
    STRINGIFY(WGPUFeatureName_TimestampQueryInsidePasses);
    STRINGIFY(WGPUFeatureName_ImplicitDeviceSynchronization);
    STRINGIFY(WGPUFeatureName_SurfaceCapabilities);
    STRINGIFY(WGPUFeatureName_TransientAttachments);
    STRINGIFY(WGPUFeatureName_MSAARenderToSingleSampled);
    STRINGIFY(WGPUFeatureName_Force32);
  }
}
static const char* typeStringify(WGPUAdapterType type) {
  switch (type) {
    STRINGIFY(WGPUAdapterType_DiscreteGPU);
    STRINGIFY(WGPUAdapterType_IntegratedGPU);
    STRINGIFY(WGPUAdapterType_CPU);
    STRINGIFY(WGPUAdapterType_Unknown);
    STRINGIFY(WGPUAdapterType_Force32);
  }
}
static const char* backendStringify(WGPUBackendType type) {
  switch (type) {
    STRINGIFY(WGPUBackendType_Undefined);
    STRINGIFY(WGPUBackendType_Null);
    STRINGIFY(WGPUBackendType_WebGPU);
    STRINGIFY(WGPUBackendType_D3D11);
    STRINGIFY(WGPUBackendType_D3D12);
    STRINGIFY(WGPUBackendType_Metal);
    STRINGIFY(WGPUBackendType_Vulkan);
    STRINGIFY(WGPUBackendType_OpenGL);
    STRINGIFY(WGPUBackendType_OpenGLES);
    STRINGIFY(WGPUBackendType_Force32);
  }
}
#undef STRINGIFY
