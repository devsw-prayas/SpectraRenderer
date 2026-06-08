# Arrays, Textures and Surfaces

**Header:** `Public/CudaArrTex.h`  
**Namespace:** `Spectra::Cuda::Arrays`

CUDA arrays are opaque, driver-managed 1D/2D/3D allocations optimized for texture hardware access. Texture objects (read) and surface objects (read/write) wrap them.

All descriptor helper free functions live in `Spectra::Cuda::Utils`.

## Arrays

```cpp
#include "CudaArrTex.h"
using namespace Spectra::Cuda::Arrays;
using namespace Spectra::Cuda::Utils;

// Describe and create a 2D FP32 array (single channel)
Array3dDesc desc{};
initArray3dDesc(desc);
setArrayDimensions(desc, /*width=*/1024, /*height=*/1024, /*depth=*/0); // depth=0 for 2D
setArrayChannels(desc, 1);
setArrayFormat(desc, ArrayFormat::FP32_ARRAY);
setArrayFlags(desc, ArrayFlags::TEXTURE_WRITE);

bool valid = validateArray3dDesc(desc);
CudaArray array = Arrays::createArray(desc);

// Upload data from host via 3D memcopy
MemCpy3DDesc cpy{};
initMemCpy3DDesc(cpy);
setMemCpy3DSrcHost(cpy, pinnedSrc);
setMemCpy3DDstArray(cpy, array);
setMemCpy3DDimensions(cpy, 1024 * sizeof(float), 1024, 1);
// ... submit copy via graph or stream memcopy node ...

Arrays::destroyArray(array);
```

`ArrayFormat` options: `FP32_ARRAY`, `FP16_ARRAY`, `UINT8_ARRAY`, `UINT16_ARRAY`, `UINT32_ARRAY`.

## Texture objects

```cpp
// ResourceDesc: what the texture reads from
ResourceDesc res{};
initResourceDesc(res);
setResourceArray(res, array);   // or setResourceLinear / setResourcePitch2D

// TextureDesc: sampling parameters
TextureDesc tex{};
initTextureDesc(tex);
setTexAddressMode(tex, TexAddressMode::CLAMP, TexAddressMode::CLAMP, TexAddressMode::CLAMP);
setTexFilterMode(tex, TexFilterMode::LINEAR_FILTER);
setTexFlags(tex, static_cast<uint32_t>(TexFlags::NORMALIZED_COORDS));

// ResourceViewDesc: typed view (optional, needed for mip arrays or typed reinterpretation)
ResourceViewDesc view{};
initResourceViewDesc(view);
setResourceViewFormat(view, ResourceViewFormat::FLOAT_1X32);
setResourceViewDimensions(view, 1024, 1024, 0);

bool resOk  = validateResourceDesc(res);
bool texOk  = validateTextureDesc(tex);
bool viewOk = validateResourceViewDesc(view);

TexObject texObj = Textures::createTextureObject(res, tex, view);

// texObj.m_TextureObject is a uint64_t you pass directly to a kernel:
//   __device__ float sample(cudaTextureObject_t tex, float u, float v) {
//       return tex2D<float>(tex, u, v);
//   }

Textures::destroyTextureObject(texObj);
```

`TexFlags` (combine with bitwise OR cast to uint32_t):

| Flag | Effect |
|------|--------|
| `NORMALIZED_COORDS` | UV in [0,1]; otherwise pixel coordinates |
| `READ_AS_INTEGER` | Return raw integer, no conversion |
| `SRGB` | Apply sRGB -> linear conversion on read |

### Linear and Pitch2D resources (no CUDA array)

For read-only access to a regular device buffer without allocating a CUDA array:

```cpp
// Linear (1D buffer)
ResourceDesc linear{};
initResourceDesc(linear);
setResourceLinear(linear, deviceBuf, ArrayFormat::FP32_ARRAY, /*channels=*/4, sizeBytes);

// Pitch2D (2D with hardware-aligned rows)
PitchedAllocation pitched = DeviceMemory::deviceAllocPitch(width * sizeof(float4), height, sizeof(float));
ResourceDesc pitch2d{};
initResourceDesc(pitch2d);
setResourcePitch2D(pitch2d, pitched.m_Address, ArrayFormat::FP32_ARRAY, 4, width, height, pitched.m_Pitch);
```

## Surface objects

Surface objects provide read/write access to a CUDA array from a kernel.

```cpp
// Array must be created with ArrayFlags::SURFACE_WRITE
Array3dDesc surfDesc{};
initArray3dDesc(surfDesc);
setArrayDimensions(surfDesc, 1024, 1024, 0);
setArrayChannels(surfDesc, 4);
setArrayFormat(surfDesc, ArrayFormat::FP32_ARRAY);
setArrayFlags(surfDesc, ArrayFlags::SURFACE_WRITE);
CudaArray surfArray = Arrays::createArray(surfDesc);

ResourceDesc surfRes{};
initResourceDesc(surfRes);
setResourceArray(surfRes, surfArray);

SurfObject surf = Surfaces::createSurfaceObject(surfRes);

// surf.m_SurfaceObject is a uint64_t passed to kernel:
//   __device__ void write(cudaSurfaceObject_t surf, int x, int y, float4 val) {
//       surf2Dwrite(val, surf, x * sizeof(float4), y);
//   }

Surfaces::destroySurfaceObject(surf);
Arrays::destroyArray(surfArray);
```
