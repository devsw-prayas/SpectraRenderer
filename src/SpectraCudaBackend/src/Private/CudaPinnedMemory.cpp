#include "SpectraCudaBackend.h"
#include "CudaPinnedMemory.h"
#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Memory {

    PinnedAddress PinnedMemory::pinnedAlloc(size_t v_Bytes) {
        SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
        void* ptr = nullptr;
        PinnedAddress result{};
        Instrumentation::staticSwitch(cuMemAllocHost(&ptr, v_Bytes),
            Instrumentation::caseOf<CUDA_SUCCESS>([&]{ result = PinnedAddress(ptr); }),
            Instrumentation::otherwise(Internal::trapCudaError));
        return result;
    }

    PinnedAddress PinnedMemory::pinnedAlloc(size_t v_Bytes, uint32_t v_Flags) {
        SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
        void* ptr = nullptr;
        PinnedAddress result{};
        Instrumentation::staticSwitch(cuMemHostAlloc(&ptr, v_Bytes, v_Flags),
            Instrumentation::caseOf<CUDA_SUCCESS>([&]{ result = PinnedAddress(ptr); }),
            Instrumentation::otherwise(Internal::trapCudaError));
        return result;
    }

    void PinnedMemory::pinnedFree(PinnedAddress& ro_Addr) {
        if (!ro_Addr.isValid()) return;
        Instrumentation::staticSwitch(cuMemFreeHost(ro_Addr.m_GpuAddr),
            Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Addr.m_GpuAddr = nullptr; }),
            Instrumentation::otherwise(Internal::trapCudaError));
    }

    void PinnedMemory::mapToDevice(GpuAddress& ro_DevAddr, const PinnedAddress& ro_Addr, uint32_t v_Flags) {
        SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
        SPEC_CUDA_BK_ASSERT(v_Flags == 0); // CUDA spec: must be 0
        CUdeviceptr ptr{};
        Instrumentation::staticSwitch(cuMemHostGetDevicePointer(&ptr, ro_Addr.m_GpuAddr, v_Flags),
            Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_DevAddr.m_GpuAddr = ptr; }),
            Instrumentation::otherwise(Internal::trapCudaError));
    }

    void PinnedMemory::hostMemRegister(const PinnedAddress& ro_Addr, size_t v_Bytes, uint32_t v_Flags) {
        SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
        SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
        Instrumentation::staticSwitch(cuMemHostRegister(ro_Addr.m_GpuAddr, v_Bytes, v_Flags),
            Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
            Instrumentation::otherwise(Internal::trapCudaError));
    }

    void PinnedMemory::hostMemUnRegister(const PinnedAddress& ro_Addr) {
        SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
        Instrumentation::staticSwitch(cuMemHostUnregister(ro_Addr.m_GpuAddr),
            Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
            Instrumentation::otherwise(Internal::trapCudaError));
    }

    void PinnedMemory::copyHostToDevAsync(const GpuAddress& ro_Dst, const PinnedAddress& ro_Src, size_t v_Bytes, const GpuStream& ro_Stream) {
        SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
        SPEC_CUDA_BK_ASSERT(ro_Dst.isValid());
        SPEC_CUDA_BK_ASSERT(ro_Src.isValid());
        SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
        Instrumentation::staticSwitch(cuMemcpyHtoDAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
            Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
            Instrumentation::otherwise(Internal::trapCudaError));
    }

    void PinnedMemory::copyDevToHostAsync(const PinnedAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes, const GpuStream& ro_Stream) {
        SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
        SPEC_CUDA_BK_ASSERT(ro_Dst.isValid());
        SPEC_CUDA_BK_ASSERT(ro_Src.isValid());
        SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
        Instrumentation::staticSwitch(cuMemcpyDtoHAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle)),
            Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
            Instrumentation::otherwise(Internal::trapCudaError));
    }
}
