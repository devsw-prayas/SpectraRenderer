#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "SpecCudaDiagnostics.h"
#include "CudaUtils.h"
#include "CudaStream.h"

namespace Spectra::Cuda::Utility {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API Utility final {
	public:
		static void memcpy3DAsync(const MemCpy3DDesc& ro_MemcpyDesc, const Streams::GpuStream& ro_Stream);
	};
}
