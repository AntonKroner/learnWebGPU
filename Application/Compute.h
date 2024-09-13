#ifndef Compute_H_
#define Compute_H_

#include "webgpu.h"
#include "./BindGroupLayoutEntry.h"
#include "./device.h"

typedef struct {
    WGPUComputePipeline pipeline;
    WGPUBindGroup bindGroup;
    struct {
        WGPUBuffer input;
        WGPUBuffer output;
    } buffer;
} Application_Compute;

static WGPUBindGroupLayout bindGroupLayout_attach(WGPUDevice device);
static WGPUBindGroup bindGroup_attach(WGPUDevice device, WGPUBindGroupLayout layout);
static WGPUComputePipeline computePipeline_attach(WGPUDevice device);
static void Compute_buffers_attach(Application_Compute compute[static 1]);

Application_Compute Application_Compute_create(WGPUDevice device) {
  const WGPUBindGroupLayout layout = bindGroupLayout_attach(device);
  Application_Compute result = {
    .bindGroup = bindGroup_attach(device, layout),
    .pipeline = computePipeline_attach(device),
  };
  Compute_buffers_attach(&result);
  return result;
}
void Application_Compute_destroy(Application_Compute compute) {
  wgpuComputePipelineRelease(compute.pipeline);
  wgpuBindGroupRelease(compute.bindGroup);
  wgpuBufferDestroy(compute.buffer.input);
  wgpuBufferRelease(compute.buffer.input);
  wgpuBufferDestroy(compute.buffer.output);
  wgpuBufferRelease(compute.buffer.output);
}
void Application_Compute_compute(
  Application_Compute compute,
  WGPUDevice device,
  WGPUQueue queue) {
  // Initialize a command encoder
  WGPUCommandEncoderDescriptor commandEncoderDesc = {
    .nextInChain = 0,
    .label = "Command Encoder",
  };
  WGPUCommandEncoder encoder =
    wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);

  // Create and use compute pass here!
  WGPUComputePassDescriptor descriptor = {
    .nextInChain = 0,
    .label = "compute pass",
    .timestampWrites = 0,
  };
  WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &descriptor);

  wgpuComputePassEncoderSetPipeline(pass, compute.pipeline);
  wgpuComputePassEncoderSetBindGroup(pass, 0, 0, 0, 0);
  wgpuComputePassEncoderDispatchWorkgroups(pass, 1, 1, 1);

  wgpuComputePassEncoderEnd(pass);
  WGPUCommandBufferDescriptor cmdBufferDescriptor = {
    .nextInChain = 0,
    .label = "command buffer",
  };
  // Encode and submit the GPU commands
  WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

  wgpuQueueSubmit(queue, 1, &commands);
  wgpuCommandBufferRelease(commands);
  wgpuCommandEncoderRelease(encoder);
  wgpuComputePassEncoderRelease(pass);
  // maybe?  wgpuDeviceTick(device);
}
static WGPUBindGroupLayout bindGroupLayout_attach(WGPUDevice device) {
  const size_t entryCount = 2;
  WGPUBindGroupLayoutEntry bindingLayouts[/*entryCount*/] = {
    Application_BindGroupLayoutEntry_make(),
    Application_BindGroupLayoutEntry_make(),
  };
  // input buffer
  bindingLayouts[0].binding = 0;
  bindingLayouts[0].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
  bindingLayouts[0].visibility = WGPUShaderStage_Compute;
  // output buffer
  bindingLayouts[1].binding = 1;
  bindingLayouts[1].buffer.type = WGPUBufferBindingType_Storage;
  bindingLayouts[1].visibility = WGPUShaderStage_Compute;
  WGPUBindGroupLayoutDescriptor descriptor = {
    .nextInChain = 0,
    .label = "compute bind group layout",
    .entryCount = entryCount,
    .entries = bindingLayouts,
  };
  WGPUBindGroupLayout result = wgpuDeviceCreateBindGroupLayout(device, &descriptor);
  return result;
}
static WGPUBindGroup bindGroup_attach(WGPUDevice device, WGPUBindGroupLayout layout) {
  const size_t entryCount = 2;
  WGPUBindGroupEntry bindings[/*entryCount*/] = {
    {
     .nextInChain = 0,
     .binding = 0,
     .buffer = 0, // input buffer
 .offset = 0,
     .size = 0 // input buffer size,
 },
    {
     .nextInChain = 0,
     .binding = 0,
     .buffer = 0, // output buffer
 .offset = 0,
     .size = 0 // output buffer size,
    },
  };
  WGPUBindGroupDescriptor descriptor = {
    .nextInChain = 0,
    .layout = layout,
    .entryCount = entryCount,
    .entries = bindings,
  };
  WGPUBindGroup result = wgpuDeviceCreateBindGroup(device, &descriptor);
  return result;
}
static WGPUComputePipeline computePipeline_attach(WGPUDevice device) {
  WGPUShaderModule shader =
    Application_device_ShaderModule(device, RESOURCE_DIR "/compute/pipeline.wgsl");
  WGPUBindGroupLayout bindGroupLayout = bindGroupLayout_attach(device);
  WGPUPipelineLayoutDescriptor layoutDescriptor = {
    .nextInChain = 0,
    .label = "compute pipeline layout",
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &bindGroupLayout,
  };
  WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layoutDescriptor);
  WGPUComputePipelineDescriptor descriptor = {
    .nextInChain = 0,
    .label = "pipeline descriptor",
    .layout = layout,
    .compute.entryPoint = "main",
    .compute.module = shader,
    .compute.constantCount = 0,
    .compute.constants = 0,
  };
  WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(device, &descriptor);
  return pipeline;
}
static void Compute_buffers_attach(Application_Compute compute[static 1]) {
}

#endif // Compute_H_
