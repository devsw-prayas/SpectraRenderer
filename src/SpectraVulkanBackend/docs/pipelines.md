# Pipelines & Shaders

**Headers:** `Public/VulkanPipeline.h`  
**Namespace:** `Spectra::Vulkan`

## VulkanShader: SPIR-V module

```cpp
#include "VulkanPipeline.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Load SPIR-V from memory (e.g. embedded at compile time)
extern const uint32_t g_VertSpirv[];
extern const size_t   g_VertSpirvSize;   // byte count

ShaderModuleHandle vertMod = VulkanShader::createShaderModule(g_VertSpirv, g_VertSpirvSize, {});
ShaderModuleHandle fragMod = VulkanShader::createShaderModule(g_FragSpirv, g_FragSpirvSize, {});

// Destroy after pipeline creation — modules are not needed at draw time
VulkanShader::destroyShaderModule(vertMod, {});
VulkanShader::destroyShaderModule(fragMod, {});
```

## VulkanPipeline: layout and cache

```cpp
// Layout: 1 set, 1 push constant range (MVP matrix)
PushConstantRange pcRange{};
pcRange.m_StageFlags = ShaderStage::VERTEX;
pcRange.m_Offset     = 0;
pcRange.m_Size       = 64;   // sizeof(glm::mat4)

PipelineLayoutHandle layout = VulkanPipeline::createLayout(1, &descSetLayout, 1, &pcRange, {});

// Empty cache (new session) — or load saved binary from disk
PipelineCacheHandle cache = VulkanPipeline::createCache({});
// PipelineCacheHandle cache = VulkanPipeline::createCacheFromData(savedData, savedSize, {});
```

## Graphics pipeline

All graphics pipelines use dynamic viewport + scissor and dynamic rendering (no render pass).

```cpp
ShaderStageDesc stages[2];
stages[0] = { ShaderStage::VERTEX,   vertMod, "main" };
stages[1] = { ShaderStage::FRAGMENT, fragMod, "main" };

VertexInputBinding binding{ 0, sizeof(Vertex), VertexInputRate::VERTEX };

VertexInputAttribute attribs[2];
attribs[0] = { 0, 0, Format::R32G32B32_SFLOAT, 0 };              // position
attribs[1] = { 1, 0, Format::R32G32_SFLOAT,    sizeof(float)*3 }; // uv

ColorBlendAttachment blend{};   // default: no blending, write RGBA

Format colorFmt = Format::B8G8R8A8_SRGB;

GraphicsPipelineDesc desc{};
desc.m_Stages               = stages;
desc.m_StageCount           = 2;
desc.m_VertexBindings       = &binding;
desc.m_VertexBindingCount   = 1;
desc.m_VertexAttributes     = attribs;
desc.m_VertexAttributeCount = 2;
desc.m_Topology             = PrimitiveTopology::TRIANGLE_LIST;
desc.m_BlendAttachments     = &blend;
desc.m_BlendAttachmentCount = 1;
desc.m_ColorFormats         = &colorFmt;
desc.m_ColorFormatCount     = 1;
desc.m_DepthFormat          = Format::D32_SFLOAT;
desc.m_Layout               = layout;
desc.m_Cache                = cache;
// desc.m_Rasterization, desc.m_DepthStencil — zero-init defaults are sane

PipelineHandle gfxPipeline = VulkanPipeline::createGraphics(desc, {});
```

## Compute pipeline

```cpp
ShaderStageDesc computeStage{ ShaderStage::COMPUTE, computeMod, "main" };

ComputePipelineDesc desc{};
desc.m_Stage  = computeStage;
desc.m_Layout = layout;
desc.m_Cache  = cache;

PipelineHandle computePipeline = VulkanPipeline::createCompute(desc, {});
```

## Ray tracing pipeline

```cpp
ShaderStageDesc rtStages[3];
rtStages[0] = { ShaderStage::RAYGEN,      raygenMod, "__raygen__main" };
rtStages[1] = { ShaderStage::MISS,        missMod,   "__miss__radiance" };
rtStages[2] = { ShaderStage::CLOSEST_HIT, chitMod,   "__closesthit__radiance" };

RayTracingShaderGroup groups[3];
groups[0] = { ShaderGroupType::GENERAL,       0, ~0u, ~0u, ~0u };   // raygen
groups[1] = { ShaderGroupType::GENERAL,       1, ~0u, ~0u, ~0u };   // miss
groups[2] = { ShaderGroupType::TRIANGLES_HIT, ~0u, 2, ~0u, ~0u };   // hit

RayTracingPipelineDesc desc{};
desc.m_Stages            = rtStages;
desc.m_StageCount        = 3;
desc.m_Groups            = groups;
desc.m_GroupCount        = 3;
desc.m_MaxRecursionDepth = 2;
desc.m_Layout            = layout;
desc.m_Cache             = cache;

PipelineHandle rtPipeline = VulkanPipeline::createRayTracing(desc, {});

// Get shader handles for SBT construction (handleSize from physical device RT props)
uint32_t handleSize    = g_GlobalInstance.m_DeviceProps.m_RTProperties.shaderGroupHandleSize;
uint32_t totalSize     = 3 * handleSize;
uint8_t  handles[3 * 64];  // 64 bytes is the max handle size
VulkanPipeline::getRayTracingShaderGroupHandles(rtPipeline, 0, 3, totalSize, handles);
// Pack into SBT buffers — align each entry to shaderGroupBaseAlignment
```

## Teardown

```cpp
VulkanPipeline::destroyPipeline(gfxPipeline, {});
VulkanPipeline::destroyPipeline(computePipeline, {});
VulkanPipeline::destroyPipeline(rtPipeline, {});
VulkanPipeline::destroyLayout(layout, {});
VulkanPipeline::destroyCache(cache, {});
```
