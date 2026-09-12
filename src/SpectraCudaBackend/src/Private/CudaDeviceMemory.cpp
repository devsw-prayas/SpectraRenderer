#include "SpectraCudaBackend.h"
#include "CudaDeviceMemory.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define  ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Memory {
	GpuAddress DeviceMemory::deviceAlloc(size_t v_Bytes) {
		CUdeviceptr ptr{};
		GpuAddress addr{};
		Instrumentation::staticSwitch(cuMemAlloc(&ptr, v_Bytes),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ addr.m_GpuAddr = ptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return addr;
	}

	PitchedAllocation DeviceMemory::deviceAllocPitch(size_t v_WidthInBytes, size_t v_Height, uint32_t v_ElemsInBytes) {
		PitchedAllocation alloc{};
		CUdeviceptr ptr{};
		size_t pitch = 0;
		Instrumentation::staticSwitch(cuMemAllocPitch(&ptr, &pitch, v_WidthInBytes, v_Height, v_ElemsInBytes),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{
				alloc.m_Address.m_GpuAddr = ptr;
				alloc.m_Pitch = pitch;
			}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return alloc;
	}

	void DeviceMemory::deviceFree(GpuAddress& ro_Address) {
		if (!ro_Address.isValid()) return;
		Instrumentation::staticSwitch(cuMemFree(static_cast<CUdeviceptr>(ro_Address.m_GpuAddr)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Address.m_GpuAddr = 0; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	GpuMemory DeviceMemory::query() {
		GpuMemory mem{};
		Instrumentation::staticSwitch(cuMemGetInfo(&mem.m_AvailableMemory, &mem.m_TotalMemory),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return mem;
	}

	void DeviceMemory::memsetD8(const GpuAddress& ro_Address, uint8_t v_Val, size_t v_Count) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		Instrumentation::staticSwitch(cuMemsetD8(ro_Address.m_GpuAddr, v_Val, v_Count),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::memsetD16(const GpuAddress& ro_Address, uint16_t v_Val, size_t v_Count) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		Instrumentation::staticSwitch(cuMemsetD16(ro_Address.m_GpuAddr, v_Val, v_Count),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::memsetD32(const GpuAddress& ro_Address, uint32_t v_Val, size_t v_Count) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		Instrumentation::staticSwitch(cuMemsetD32(ro_Address.m_GpuAddr, v_Val, v_Count),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::memsetD8Async(const GpuAddress& ro_Address, uint8_t v_Val, size_t v_Count, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		Instrumentation::staticSwitch(cuMemsetD8Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::memsetD16Async(const GpuAddress& ro_Address, uint16_t v_Val, size_t v_Count, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		Instrumentation::staticSwitch(cuMemsetD16Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::memsetD32Async(const GpuAddress& ro_Address, uint32_t v_Val, size_t v_Count, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		SPEC_CUDA_BK_ASSERT(ro_Address.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		Instrumentation::staticSwitch(cuMemsetD32Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}


	void DeviceMemory::copyDeviceToDevice(const GpuAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes) {
		SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
		SPEC_CUDA_BK_ASSERT(ro_Src.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Dst.isValid());
		Instrumentation::staticSwitch(cuMemcpyDtoD(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceMemory::copyDeviceToDeviceAsync(const GpuAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
		SPEC_CUDA_BK_ASSERT(ro_Src.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Dst.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		Instrumentation::staticSwitch(cuMemcpyDtoDAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}
}
