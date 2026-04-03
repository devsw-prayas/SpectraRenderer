#include "SpectraCudaBackend.h"
#include "CudaUtility.h"
#include "CudaBootstrap.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Utility {
	void Utility::memcpy3DAsync(const MemCpy3DDesc& ro_MemcpyDesc, const Streams::GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(validateMemCpy3DDesc(ro_MemcpyDesc));
		CUDA_MEMCPY3D desc = Internal::CUDA_PackingFunctions::pack3dMemcpyDesc(ro_MemcpyDesc);
		const CUresult result = cuMemcpy3DAsync(&desc, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result)
	}
}
