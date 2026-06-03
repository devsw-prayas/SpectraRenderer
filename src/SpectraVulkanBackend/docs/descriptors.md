# Descriptors & Binding

**Header:** `Public/VulkanDescriptors.h`  
**Namespace:** `Spectra::Vulkan`

## Descriptor set layout

```cpp
#include "VulkanDescriptors.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Binding 0 = uniform buffer (vertex+fragment), binding 1 = combined image sampler (fragment)
DescriptorSetLayoutBinding bindings[2];
bindings[0] = { 0, DescriptorType::UNIFORM_BUFFER,         1, ShaderStage::VERTEX | ShaderStage::FRAGMENT };
bindings[1] = { 1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, ShaderStage::FRAGMENT };

DescriptorSetLayoutHandle layout = VulkanDescriptors::createDescriptorSetLayout(2, bindings, {});

// Destroy
VulkanDescriptors::destroyDescriptorSetLayout(layout, {});
```

## Descriptor pool

```cpp
DescriptorPoolSize sizes[2];
sizes[0] = { DescriptorType::UNIFORM_BUFFER,         64 };
sizes[1] = { DescriptorType::COMBINED_IMAGE_SAMPLER, 64 };

// MAX_SETS = 32, allocate from it repeatedly
DescriptorPoolHandle pool = VulkanDescriptors::createDescriptorPool(32, 2, sizes, {});

// Reset frees all sets at once (no per-set free needed, fast path for per-frame pools)
VulkanDescriptors::resetDescriptorPool(pool);

VulkanDescriptors::destroyDescriptorPool(pool, {});
```

## Allocating descriptor sets

```cpp
// Allocate 2 sets from the same layout (e.g. one per frame-in-flight)
DescriptorSetLayoutHandle layouts[2] = { layout, layout };
DescriptorSetHandle       sets   [2];
VulkanDescriptors::allocateDescriptorSets(pool, 2, layouts, sets);
// sets[0] and sets[1] are ready to write to
```

Up to `VulkanDescriptors::MAX_SETS_PER_ALLOC` (16) sets per call.

## Writing descriptor sets

```cpp
// Write a uniform buffer to binding 0
DescriptorBufferInfo bufInfo{};
bufInfo.m_Buffer = uniformBuf;
bufInfo.m_Offset = 0;
bufInfo.m_Range  = sizeof(MyUBO);   // or ~0ULL for whole buffer

WriteDescriptorSet bufWrite{};
bufWrite.m_DstSet       = sets[0];
bufWrite.m_Binding      = 0;
bufWrite.m_ArrayElement = 0;
bufWrite.m_Count        = 1;
bufWrite.m_Type         = DescriptorType::UNIFORM_BUFFER;
bufWrite.m_pBufferInfo  = &bufInfo;

// Write a texture + sampler to binding 1
DescriptorImageInfo imgInfo{};
imgInfo.m_Sampler   = sampler;
imgInfo.m_ImageView = texView;
imgInfo.m_Layout    = ImageLayout::SHADER_READ_ONLY;

WriteDescriptorSet imgWrite{};
imgWrite.m_DstSet      = sets[0];
imgWrite.m_Binding     = 1;
imgWrite.m_Count       = 1;
imgWrite.m_Type        = DescriptorType::COMBINED_IMAGE_SAMPLER;
imgWrite.m_pImageInfo  = &imgInfo;

WriteDescriptorSet writes[2] = { bufWrite, imgWrite };
VulkanDescriptors::updateDescriptorSets(2, writes);
```

For each `WriteDescriptorSet` set exactly one of `m_pBufferInfo`, `m_pImageInfo`, or `m_pAccelerationStructures`. Up to `MAX_WRITES` (64) writes and `MAX_INFOS_PER_CALL` (256) total descriptors per call.

## Acceleration structure descriptor (ray tracing)

```cpp
WriteDescriptorSet asWrite{};
asWrite.m_DstSet                 = sets[0];
asWrite.m_Binding                = 2;
asWrite.m_Count                  = 1;
asWrite.m_Type                   = DescriptorType::ACCELERATION_STRUCTURE;
asWrite.m_pAccelerationStructures = &tlasHandle;

VulkanDescriptors::updateDescriptorSets(1, &asWrite);
```
