#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"

namespace Spectra::Cuda::Streams {
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuStream final {
		void* m_StreamHandle = nullptr;

		GpuStream() = default;
		~GpuStream() = default;

		GpuStream(const GpuStream&) = default;
		GpuStream& operator=(const GpuStream&) = default;

		GpuStream(GpuStream&&) noexcept = default;
		GpuStream& operator=(GpuStream&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_StreamHandle != nullptr;
		}
	};
}
