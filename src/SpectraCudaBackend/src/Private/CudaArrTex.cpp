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
		TexObject obj;
		CUDA_RESOURCE_DESC descA = Internal::CUDA_PackingFunctions::packResourceDesc(ro_ResDesc);
		CUDA_TEXTURE_DESC descB = Internal::CUDA_PackingFunctions::packTextureDesc(ro_TexDesc);
		CUDA_RESOURCE_VIEW_DESC descC = Internal::CUDA_PackingFunctions::packResourceViewDesc(ro_ResViewDesc);
		CUresult result = cuTexObjectCreate(&obj.m_TextureObject, &descA, &descB, &descC);
		if (result == CUDA_SUCCESS) return obj;
		CUDA_ERROR_TRAP(result);
			SPEC_CUDA_BK_UNREACHABLE();
	}

	void Textures::destroyTextureObject(TexObject& ro_TextureObject) {
		CUresult result = cuTexObjectDestroy(ro_TextureObject.m_TextureObject);
		if (result == CUDA_SUCCESS) {
			ro_TextureObject.m_TextureObject = 0;
			return;
		}
		CUDA_ERROR_TRAP(result);
	}

	CudaArray Arrays::createArray(const Array3dDesc& ro_Desc) {
		SPEC_CUDA_BK_ASSERT(validateArray3dDesc(ro_Desc));
		CudaArray arr{};
		CUDA_ARRAY3D_DESCRIPTOR dsec = Internal::CUDA_PackingFunctions::packArray3dDesc(ro_Desc);
		CUresult result = cuArray3DCreate_v2(reinterpret_cast<CUarray*>(&arr.m_Array), &dsec);
		if (result == CUDA_SUCCESS) return arr;
		CUDA_ERROR_TRAP(result);
			SPEC_CUDA_BK_UNREACHABLE();
	}

	void Arrays::destroyArray(CudaArray& ro_Array) {
		SPEC_CUDA_BK_ASSERT(ro_Array.isValid());
		const CUresult result = cuArrayDestroy(static_cast<CUarray>(ro_Array.m_Array));
		if (result == CUDA_SUCCESS) {
			ro_Array.m_Array = nullptr;
			return;
		}
		CUDA_ERROR_TRAP(result);
	}

	SurfObject Surfaces::createSurfaceObject(const ResourceDesc& ro_Desc) {
		SPEC_CUDA_BK_ASSERT(validateResourceDesc(ro_Desc));
		SurfObject obj{};
		CUDA_RESOURCE_DESC desc = Internal::CUDA_PackingFunctions::packResourceDesc(ro_Desc);
		const CUresult result = cuSurfObjectCreate(&obj.m_SurfaceObject, &desc);
		if (result == CUDA_SUCCESS) return obj;
		CUDA_ERROR_TRAP(result);
		SPEC_CUDA_BK_UNREACHABLE();
	}

	void Surfaces::destroySurfaceObject(SurfObject& ro_SurfaceObject) {
		SPEC_CUDA_BK_ASSERT(ro_SurfaceObject.isValid());
		const CUresult result = cuSurfObjectDestroy(ro_SurfaceObject.m_SurfaceObject);
		if (result == CUDA_SUCCESS) {
			ro_SurfaceObject.m_SurfaceObject = 0;
			return;
		}
		CUDA_ERROR_TRAP(result);
	}
}