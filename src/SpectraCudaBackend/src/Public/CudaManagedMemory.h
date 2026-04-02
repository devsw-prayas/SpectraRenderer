#pragma once
#include "CudaStream.h"
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "SpecCudaDiagnostics.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Memory {
	using namespace Utils;
	class ManagedMemory final {
	public:
		static GpuAddress allocManaged(size_t v_Bytes, uint32_t v_Flags);
		static void adviseMemory(
			const GpuAddress& ro_Addr, size_t v_Coubt,
			MemoryAdvise v_Advise, const DeviceHandle& ro_Handle);
		static void prefetchAsync(
			const GpuAddress& ro_Addr, size_t v_Coubt,
			const DeviceHandle& ro_Handle, const Streams::GpuStream& ro_Stream);
	};
}
