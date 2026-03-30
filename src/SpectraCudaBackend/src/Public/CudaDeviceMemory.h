#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "CudaStream.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Memory {
	using namespace Utils;
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) PitchedAllocation final {
		GpuAddress m_Address;
		size_t     m_Pitch;

		PitchedAllocation() = default;
		~PitchedAllocation() = default;

		PitchedAllocation(const PitchedAllocation&) = default;
		PitchedAllocation& operator=(const PitchedAllocation&) = default;

		PitchedAllocation(PitchedAllocation&&) noexcept = default;
		PitchedAllocation& operator=(PitchedAllocation&&) noexcept = default;
	};

	class SPEC_CUDA_BK_RUNTIME_API DeviceMemory final {
	public:
		static GpuAddress deviceAlloc(size_t v_Bytes);
		static PitchedAllocation deviceAllocPitch(size_t v_WidthInBytes, size_t v_Height, size_t v_ElemsInBytes);
		static void deviceFree(GpuAddress& ro_Address);
		static GpuMemory query();

		static void memsetD8(const GpuAddress& ro_Address, uint8_t v_Val, size_t v_Count);
		static void memsetD16(const GpuAddress& ro_Address, uint16_t v_Val, size_t v_Count);
		static void memsetD32(const GpuAddress& ro_Address, uint32_t v_Val, size_t v_Count);

		static void memsetD8Async(
			const GpuAddress& ro_Address, uint8_t v_Val,
			size_t v_Count, const Streams::GpuStream& ro_Stream);

		static void memsetD16Async(
			const GpuAddress& ro_Address, uint16_t v_Val,
			size_t v_Count, const Streams::GpuStream& ro_Stream);

		static void memsetD32Async(
			const GpuAddress& ro_Address, uint32_t v_Val,
			size_t v_Count, const Streams::GpuStream& ro_Stream);

		static void copyDeviceToDevice(const GpuAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes);
		static void copyDeviceToDeviceAsync(
			const GpuAddress& ro_Dst, const GpuAddress& ro_Src,
			size_t v_Bytes, const Streams::GpuStream& ro_Stream);
	};
}
