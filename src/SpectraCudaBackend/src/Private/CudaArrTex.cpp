#include "SpectraCudaBackend.h"
#include "CudaArrTex.h"
#include "CudaBootstrap.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Arrays {
	TexObject Textures::createTextureObject(const ResourceDesc& ro_ResDesc, const TextureDesc& ro_TexDesc, const ResourceViewDesc& ro_ResViewDesc) {
		SPEC_CUDA_BK_ASSERT(validateResourceDesc(ro_ResDesc));
		SPEC_CUDA_BK_ASSERT(validateTextureDesc(ro_TexDesc));
		SPEC_CUDA_BK_ASSERT(validateResourceViewDesc(ro_ResViewDesc));
		TexObject obj{};
		CUDA_RESOURCE_DESC descA = Internal::CUDA_PackingFunctions::packResourceDesc(ro_ResDesc);
		CUDA_TEXTURE_DESC descB = Internal::CUDA_PackingFunctions::packTextureDesc(ro_TexDesc);
		CUDA_RESOURCE_VIEW_DESC descC = Internal::CUDA_PackingFunctions::packResourceViewDesc(ro_ResViewDesc);
		Instrumentation::staticSwitch(cuTexObjectCreate(&obj.m_TextureObject, &descA, &descB, &descC),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return obj;
	}

	void Textures::destroyTextureObject(TexObject& ro_TextureObject) {
		Instrumentation::staticSwitch(cuTexObjectDestroy(ro_TextureObject.m_TextureObject),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_TextureObject.m_TextureObject = 0; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	CudaArray Arrays::createArray(const Array3dDesc& ro_Desc) {
		SPEC_CUDA_BK_ASSERT(validateArray3dDesc(ro_Desc));
		CudaArray arr{};
		CUDA_ARRAY3D_DESCRIPTOR dsec = Internal::CUDA_PackingFunctions::packArray3dDesc(ro_Desc);
		Instrumentation::staticSwitch(cuArray3DCreate_v2(reinterpret_cast<CUarray*>(&arr.m_Array), &dsec),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return arr;
	}

	void Arrays::destroyArray(CudaArray& ro_Array) {
		SPEC_CUDA_BK_ASSERT(ro_Array.isValid());
		Instrumentation::staticSwitch(cuArrayDestroy(static_cast<CUarray>(ro_Array.m_Array)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Array.m_Array = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	SurfObject Surfaces::createSurfaceObject(const ResourceDesc& ro_Desc) {
		SPEC_CUDA_BK_ASSERT(validateResourceDesc(ro_Desc));
		SurfObject obj{};
		CUDA_RESOURCE_DESC desc = Internal::CUDA_PackingFunctions::packResourceDesc(ro_Desc);
		Instrumentation::staticSwitch(cuSurfObjectCreate(&obj.m_SurfaceObject, &desc),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return obj;
	}

	void Surfaces::destroySurfaceObject(SurfObject& ro_SurfaceObject) {
		SPEC_CUDA_BK_ASSERT(ro_SurfaceObject.isValid());
		Instrumentation::staticSwitch(cuSurfObjectDestroy(ro_SurfaceObject.m_SurfaceObject),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_SurfaceObject.m_SurfaceObject = 0; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}
}
