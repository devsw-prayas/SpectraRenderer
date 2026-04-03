#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "SpecCudaDiagnostics.h"
#include "CudaUtils.h"
#include "CudaStream.h"

namespace Spectra::Cuda::Arrays {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API Arrays final {
	public:
		static CudaArray createArray(const Array3dDesc& ro_Desc);
		static void destroyArray(CudaArray& ro_Array);
	};

	class SPEC_CUDA_BK_RUNTIME_API Textures final {
	public:
		static TexObject createTextureObject(
			const ResourceDesc& ro_ResDesc, const TextureDesc& ro_TexDesc, const ResourceViewDesc& ro_ResViewDesc);
		static void destroyTextureObject(TexObject& ro_TextureObject);
	};

	class SPEC_CUDA_BK_RUNTIME_API Surfaces final {
	public:
		static SurfObject createSurfaceObject(const ResourceDesc& ro_Desc);
		static void destroySurfaceObject(SurfObject& ro_SurfaceObject);
	};
}
