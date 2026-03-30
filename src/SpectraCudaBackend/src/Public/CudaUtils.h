#pragma once

#include "SpectraCudaBackend.h"
#include "SpecCudaDiagnostics.h"

namespace Spectra::Cuda::Utils {
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) DeviceHandle final {
		int m_HandleValue;

		DeviceHandle() = default;
		~DeviceHandle() = default;

		DeviceHandle(const DeviceHandle&) = default;
		DeviceHandle& operator=(const DeviceHandle&) = default;

		DeviceHandle(DeviceHandle&&) noexcept = default;
		DeviceHandle& operator=(DeviceHandle&&) noexcept = default;
	};

	enum class SPEC_CUDA_BK_RUNTIME_API CudaDeviceAttribute : uint8_t {
		COMPUTE_CAPABILITY_MAJOR,               // Major SM version (architecture generation)
		COMPUTE_CAPABILITY_MINOR,               // Minor SM version (architecture revision)

		MAX_THREADS_PER_BLOCK,                  // Maximum number of threads in a single block
		MAX_GRID_DIM_X,                         // Maximum grid dimension in X direction
		MAX_GRID_DIM_Y,                         // Maximum grid dimension in Y direction
		MAX_GRID_DIM_Z,                         // Maximum grid dimension in Z direction

		MAX_SHARED_MEMORY_PER_BLOCK,            // Maximum shared memory available per block (bytes)

		WARP_SIZE,                              // Number of threads per warp (typically 32)

		MEMORY_CLOCK_RATE,                      // Memory clock frequency (kHz)
		GLOBAL_MEMORY_BUS_WIDTH,                // Width of memory bus (bits)

		L2_CACHE_SIZE,                          // Size of L2 cache (bytes)

		UNIFIED_ADDRESSING,                     // Whether unified virtual addressing is supported (0/1)
		CONCURRENT_KERNELS,                     // Whether multiple kernels can execute concurrently (0/1)

		CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM,// Whether host pointers can be directly used after registration (0/1)

		GPU_DIRECT_RDMA_SUPPORTED,              // Support for GPUDirect RDMA (0/1)
		VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED     // Support for CUDA virtual memory APIs (0/1)
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) DeviceUUID final {
		uint64_t m_Lo;
		uint64_t m_Hi;

		DeviceUUID() = default;
		~DeviceUUID() = default;

		DeviceUUID(const DeviceUUID&) = default;
		DeviceUUID& operator=(const DeviceUUID&) = default;

		DeviceUUID(DeviceUUID&&) noexcept = default;
		DeviceUUID& operator=(DeviceUUID&&) noexcept = default;
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(DeviceUUID) == 16, "Inavlid UUID Struct layout size, must be 16");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<DeviceUUID>, "Invalid UUID struct layout, must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<DeviceUUID>, "Invalid UUID struct members, must be trivial");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<DeviceUUID>, "Invalid UUID struct members, must be trivial");

	using CtxPtr = void*;

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) CudaContext final {
		CtxPtr m_Handle;

		CudaContext() = default;
		~CudaContext() = default;

		CudaContext(const CudaContext&) = default;
		CudaContext& operator=(const CudaContext&) = default;

		CudaContext(CudaContext&&) noexcept = default;
		CudaContext& operator=(CudaContext&&) noexcept = default;
	};

	enum class SPEC_CUDA_BK_RUNTIME_API	ContextSchedulingFlags final : uint8_t {
		SCHEDULE_AUTO,
		SCHEDULE_SPIN,
		SCHEDULE_YIELD,
		SCHEDULE_BLOCKING_SYNC
	};

	enum class SPEC_CUDA_BK_RUNTIME_API ContextCreationFlags final : uint8_t {
		NONE,
		MAP_HOST,
		LMEM_RESIZE_TO_MAX
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) PinnedAddress final {
		void* m_GpuAddr = nullptr;

		explicit PinnedAddress(void* addr) : m_GpuAddr(addr) {}
		PinnedAddress() = default;
		~PinnedAddress() = default;

		PinnedAddress(const PinnedAddress&) = default;
		PinnedAddress& operator=(const PinnedAddress&) = default;

		PinnedAddress(PinnedAddress&&) noexcept = default;
		PinnedAddress& operator=(PinnedAddress&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_GpuAddr != nullptr;
		}
	};

	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<PinnedAddress>, "GpuAddress must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<PinnedAddress>, "GpuAddress must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<PinnedAddress>, "GpuAddress must be trivially move assignable");
	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(PinnedAddress) == 8, "Invalid GpuAddress size, must be 64bit");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) GpuMemory final {
		size_t m_TotalMemory = 0;
		size_t m_AvailableMemory = 0;

		GpuMemory() = default;
		~GpuMemory() = default;

		GpuMemory(const GpuMemory&) = default;
		GpuMemory& operator=(const GpuMemory&) = default;

		GpuMemory(GpuMemory&&) noexcept = default;
		GpuMemory& operator=(GpuMemory&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuAddress final {
		uint64_t m_GpuAddr = 0;

		explicit GpuAddress(uint64_t addr) : m_GpuAddr(addr) {}
		GpuAddress() = default;
		~GpuAddress() = default;

		GpuAddress(const GpuAddress&) = default;
		GpuAddress& operator=(const GpuAddress&) = default;

		GpuAddress(GpuAddress&&) noexcept = default;
		GpuAddress& operator=(GpuAddress&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_GpuAddr != 0;
		}
	};

	enum class SPEC_CUDA_BK_RUNTIME_API HostAllocFlags final : uint8_t {
		ALLOC_PORTABLE,
		ALLOC_DEVICE_MAP,
		ALLOC_WRITE_COMBINED
	};

	enum class SPEC_CUDA_BK_RUNTIME_API HostRegisterFlags final : uint8_t {
		REG_PORTABLE,
		REG_DEVICE_MAP,
		REG_IO_MEMORY,
		REG_READ_ONLY
	};

	class CudaHelpers final {
	public:
		static uint32_t computeAllocFlag(std::initializer_list<HostAllocFlags> flags);
		static uint32_t computeRegFlag(std::initializer_list<HostRegisterFlags> flags);
	};
}