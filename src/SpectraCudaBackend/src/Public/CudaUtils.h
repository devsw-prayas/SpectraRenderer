#pragma once

#include "SpectraCudaBackend.h"
#include "SpecCudaDiagnostics.h"

namespace Spectra::Cuda::Utils {
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
		VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED,    // Support for CUDA virtual memory APIs (0/1)
		CONCURRENT_MANAGED_ACCESS               // GPU can access managed memory concurrently with CPU (0/1); false on Windows WDDM
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

	// Mapping for CUmem_advise
	enum class MemoryAdvise final : uint8_t {
		SET_READ_MOSTLY,
		UNSET_READ_MOSTLY,

		SET_PREFERRED_LOCATION,
		UNSET_PREFERRED_LOCATION,

		SET_ACCESSED_BY,
		UNSET_ACCESSED_BY
	};

	enum class DeviceLocation final : uint8_t {
		CPU, GPU
	};

	enum class SPEC_CUDA_BK_RUNTIME_API AllocationType final : uint8_t {
		INVALID,
		PINNED,
	};

	enum class SPEC_CUDA_BK_RUNTIME_API AllocationHandleType final : uint8_t {
		NONE,
		WIN32_HANDLE,
		FABRIC_HANDLE
	};

	enum class SPEC_CUDA_BK_RUNTIME_API AccessFlagBits : uint8_t {
		NONE = 0,
		READ = 1 << 0,
		READWRITE = 1 << 1
	};

	enum class SPEC_CUDA_BK_RUNTIME_API AllocationGranularityOption final : uint8_t {
		MINIMUM,
		RECOMMENDED
	};

	enum class SPEC_CUDA_BK_RUNTIME_API ArrayFormat final : uint8_t {
		FP32_ARRAY,
		FP16_ARRAY,
		UINT8_ARRAY,
		UINT16_ARRAY,
		UINT32_ARRAY
	};

	enum class SPEC_CUDA_BK_RUNTIME_API ArrayFlags final : uint8_t {
		TEXTURE_WRITE,
		SURFACE_WRITE
	};

	class SPEC_CUDA_BK_RUNTIME_API CudaHelpers final {
	public:
		static uint32_t computeAllocFlag(std::initializer_list<HostAllocFlags> flags);
		static uint32_t computeRegFlag(std::initializer_list<HostRegisterFlags> flags);
		static uint64_t computeAccessFlags(std::initializer_list<AccessFlagBits> flags);
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) DeviceHandle final {
		int m_HandleValue;

		bool isValid() const {
			return m_HandleValue > -1;
		}
		DeviceHandle() = default;
		~DeviceHandle() = default;

		DeviceHandle(const DeviceHandle&) = default;
		DeviceHandle& operator=(const DeviceHandle&) = default;

		DeviceHandle(DeviceHandle&&) noexcept = default;
		DeviceHandle& operator=(DeviceHandle&&) noexcept = default;

		static DeviceHandle makeCpu();
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

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) AllocHandle final {
		uint64_t m_Handle;

		bool isValid() const {
			return m_Handle ? 1 : 0;
		}

		AllocHandle() = default;
		~AllocHandle() = default;

		AllocHandle(const AllocHandle&) = default;
		AllocHandle& operator=(const AllocHandle&) = default;

		AllocHandle(AllocHandle&&) noexcept = default;
		AllocHandle& operator=(AllocHandle&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) Location final {
		DeviceLocation m_Location;
		DeviceHandle m_Handle;

		Location() = default;
		~Location() = default;

		Location(const Location&) = default;
		Location& operator=(const Location&) = default;

		Location(Location&&) noexcept = default;
		Location& operator=(Location&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) AllocDesc final {
		void* m_win32meta;
		Location m_Loc;
		AllocationType m_Type;
		AllocationHandleType m_HandleType;

		AllocDesc() = default;
		~AllocDesc() = default;

		AllocDesc(const AllocDesc&) = default;
		AllocDesc& operator=(const AllocDesc&) = default;

		AllocDesc(AllocDesc&&) noexcept = default;
		AllocDesc& operator=(AllocDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initAllocDesc(AllocDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setAllocationType(AllocDesc& ro_Desc, AllocationType v_Type);
	SPEC_CUDA_BK_RUNTIME_API void setAllocationHandleType(AllocDesc& ro_Desc, AllocationHandleType v_Type);
	SPEC_CUDA_BK_RUNTIME_API void setLocation(AllocDesc& ro_Desc, DeviceHandle& ro_Handle);

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) AccessDesc final {
		Location m_Loc;
		uint64_t flags;

		AccessDesc() = default;
		~AccessDesc() = default;

		AccessDesc(const AccessDesc&) = default;
		AccessDesc& operator=(const AccessDesc&) = default;

		AccessDesc(AccessDesc&&) noexcept = default;
		AccessDesc& operator=(AccessDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initAccessDesc(AccessDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setAccessLocation(AccessDesc& ro_Desc, DeviceHandle& ro_Handle);
	SPEC_CUDA_BK_RUNTIME_API void setAccessFlags(AccessDesc& ro_Desc, uint64_t v_Flags);

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) CudaArray final {
		void* m_Array = nullptr;

		CudaArray() = default;
		~CudaArray() = default;

		CudaArray(const CudaArray&) = default;
		CudaArray& operator=(const CudaArray&) = default;

		CudaArray(CudaArray&&) noexcept = default;
		CudaArray& operator=(CudaArray&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_Array != nullptr;
		}
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(CudaArray) == 8, "Invalid CudaArray size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<CudaArray>, "CudaArray must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<CudaArray>, "CudaArray must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<CudaArray>, "CudaArray must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) TexObject final {
		uint64_t m_TextureObject = 0;

		TexObject() = default;
		~TexObject() = default;

		TexObject(const TexObject&) = default;
		TexObject& operator=(const TexObject&) = default;

		TexObject(TexObject&&) noexcept = default;
		TexObject& operator=(TexObject&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_TextureObject != 0;
		}
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(TexObject) == 8, "Invalid TexObject size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<TexObject>, "TexObject must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<TexObject>, "TexObject must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<TexObject>, "TexObject must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) SurfObject final {
		uint64_t m_SurfaceObject = 0;

		SurfObject() = default;
		~SurfObject() = default;

		SurfObject(const SurfObject&) = default;
		SurfObject& operator=(const SurfObject&) = default;

		SurfObject(SurfObject&&) noexcept = default;
		SurfObject& operator=(SurfObject&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_SurfaceObject != 0;
		}
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(SurfObject) == 8, "Invalid SurfObject size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<SurfObject>, "SurfObject must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<SurfObject>, "SurfObject must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<SurfObject>, "SurfObject must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) Array3dDesc final {
		uint64_t    m_Width = 0;
		uint64_t    m_Height = 0;
		uint64_t    m_Depth = 0;
		uint32_t    m_Channels = 0;
		uint32_t    m_Flags = 0;
		ArrayFormat m_Format = ArrayFormat::FP32_ARRAY;

		Array3dDesc() = default;
		~Array3dDesc() = default;

		Array3dDesc(const Array3dDesc&) = default;
		Array3dDesc& operator=(const Array3dDesc&) = default;

		Array3dDesc(Array3dDesc&&) noexcept = default;
		Array3dDesc& operator=(Array3dDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initArray3dDesc(Array3dDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setArrayDimensions(Array3dDesc& ro_Desc, uint64_t v_Width, uint64_t v_Height, uint64_t v_Depth);
	SPEC_CUDA_BK_RUNTIME_API void setArrayChannels(Array3dDesc& ro_Desc, uint32_t v_Channels);
	SPEC_CUDA_BK_RUNTIME_API void setArrayFormat(Array3dDesc& ro_Desc, ArrayFormat v_Format);
	SPEC_CUDA_BK_RUNTIME_API void setArrayFlags(Array3dDesc& ro_Desc, ArrayFlags v_Flags);

	enum class SPEC_CUDA_BK_RUNTIME_API CopyMemoryType final : uint8_t {
		HOST,
		DEVICE,
		ARRAY
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) MemCpy3DDesc final {
		// source
		CopyMemoryType  m_SrcType           = CopyMemoryType::HOST;
		PinnedAddress   m_SrcHost           = {};
		GpuAddress      m_SrcDevice         = {};
		CudaArray       m_SrcArray          = {};
		size_t          m_SrcPitch          = 0;
		size_t          m_SrcHeight         = 0;
		size_t          m_SrcXOffsetBytes   = 0;
		size_t          m_SrcYOffset        = 0;
		size_t          m_SrcZOffset        = 0;

		// destination
		CopyMemoryType  m_DstType           = CopyMemoryType::ARRAY;
		PinnedAddress   m_DstHost           = {};
		GpuAddress      m_DstDevice         = {};
		CudaArray       m_DstArray          = {};
		size_t          m_DstPitch          = 0;
		size_t          m_DstHeight         = 0;
		size_t          m_DstXOffsetBytes   = 0;
		size_t          m_DstYOffset        = 0;
		size_t          m_DstZOffset        = 0;

		// copy dimensions
		size_t          m_WidthInBytes      = 0;
		size_t          m_Height            = 0;
		size_t          m_Depth             = 0;

		MemCpy3DDesc() = default;
		~MemCpy3DDesc() = default;

		MemCpy3DDesc(const MemCpy3DDesc&) = default;
		MemCpy3DDesc& operator=(const MemCpy3DDesc&) = default;

		MemCpy3DDesc(MemCpy3DDesc&&) noexcept = default;
		MemCpy3DDesc& operator=(MemCpy3DDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initMemCpy3DDesc(MemCpy3DDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Src);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Src);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcArray(MemCpy3DDesc& ro_Desc, CudaArray v_Src);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Dst);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Dst);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstArray(MemCpy3DDesc& ro_Desc, CudaArray v_Dst);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDimensions(MemCpy3DDesc& ro_Desc, size_t v_WidthInBytes, size_t v_Height, size_t v_Depth);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset);
	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset);

	// -------------------------------------------------------------------------
	// Arrays & Textures
	// -------------------------------------------------------------------------

	enum class SPEC_CUDA_BK_RUNTIME_API ResourceType final : uint8_t {
		ARRAY,
		MIPMAPPED_ARRAY,
		LINEAR,
		PITCH_2D
	};

	enum class SPEC_CUDA_BK_RUNTIME_API TexAddressMode final : uint8_t {
		WRAP,
		CLAMP,
		MIRROR,
		BORDER
	};

	enum class SPEC_CUDA_BK_RUNTIME_API TexFilterMode final : uint8_t {
		POINT,
		LINEAR_FILTER
	};

	enum class SPEC_CUDA_BK_RUNTIME_API TexFlags : uint8_t {
		NORMALIZED_COORDS = 1 << 0,
		READ_AS_INTEGER   = 1 << 1,
		SRGB              = 1 << 2
	};

	enum class SPEC_CUDA_BK_RUNTIME_API ResourceViewFormat final : uint8_t {
		NONE,
		UINT_1X8,  UINT_2X8,  UINT_4X8,
		SINT_1X8,  SINT_2X8,  SINT_4X8,
		UINT_1X16, UINT_2X16, UINT_4X16,
		SINT_1X16, SINT_2X16, SINT_4X16,
		UINT_1X32, UINT_2X32, UINT_4X32,
		SINT_1X32, SINT_2X32, SINT_4X32,
		FLOAT_1X16, FLOAT_2X16, FLOAT_4X16,
		FLOAT_1X32, FLOAT_2X32, FLOAT_4X32
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) MipmappedArray final {
		void* m_Array = nullptr;

		MipmappedArray() = default;
		~MipmappedArray() = default;

		MipmappedArray(const MipmappedArray&) = default;
		MipmappedArray& operator=(const MipmappedArray&) = default;

		MipmappedArray(MipmappedArray&&) noexcept = default;
		MipmappedArray& operator=(MipmappedArray&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const {
			return m_Array != nullptr;
		}
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(MipmappedArray) == 8, "Invalid MipmappedArray size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<MipmappedArray>, "MipmappedArray must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<MipmappedArray>, "MipmappedArray must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<MipmappedArray>, "MipmappedArray must be trivially move assignable");

	// ResourceDesc — describes what a tex/surf object wraps.
	// Only fields relevant to m_ResType are used; others are ignored by the driver.
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) ResourceDesc final {
		ResourceType    m_ResType           = ResourceType::ARRAY;

		// ARRAY
		CudaArray       m_Array             = {};

		// MIPMAPPED_ARRAY
		MipmappedArray  m_MipArray          = {};

		// LINEAR
		GpuAddress      m_LinearDevice      = {};
		ArrayFormat     m_LinearFormat      = ArrayFormat::FP32_ARRAY;
		uint32_t        m_LinearChannels    = 0;
		size_t          m_LinearSizeBytes   = 0;

		// PITCH_2D
		GpuAddress      m_Pitch2DDevice     = {};
		ArrayFormat     m_Pitch2DFormat     = ArrayFormat::FP32_ARRAY;
		uint32_t        m_Pitch2DChannels   = 0;
		size_t          m_Pitch2DWidth      = 0;
		size_t          m_Pitch2DHeight     = 0;
		size_t          m_Pitch2DPitch      = 0;

		ResourceDesc() = default;
		~ResourceDesc() = default;

		ResourceDesc(const ResourceDesc&) = default;
		ResourceDesc& operator=(const ResourceDesc&) = default;

		ResourceDesc(ResourceDesc&&) noexcept = default;
		ResourceDesc& operator=(ResourceDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initResourceDesc(ResourceDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setResourceArray(ResourceDesc& ro_Desc, CudaArray v_Array);
	SPEC_CUDA_BK_RUNTIME_API void setResourceMipmappedArray(ResourceDesc& ro_Desc, MipmappedArray v_Array);
	SPEC_CUDA_BK_RUNTIME_API void setResourceLinear(ResourceDesc& ro_Desc, GpuAddress v_Device, ArrayFormat v_Format, uint32_t v_Channels, size_t v_SizeBytes);
	SPEC_CUDA_BK_RUNTIME_API void setResourcePitch2D(ResourceDesc& ro_Desc, GpuAddress v_Device, ArrayFormat v_Format, uint32_t v_Channels, size_t v_Width, size_t v_Height, size_t v_Pitch);

	// TextureDesc — sampling parameters for cuTexObjectCreate.
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) TextureDesc final {
		TexAddressMode  m_AddressModeU          = TexAddressMode::CLAMP;
		TexAddressMode  m_AddressModeV          = TexAddressMode::CLAMP;
		TexAddressMode  m_AddressModeW          = TexAddressMode::CLAMP;
		TexFilterMode   m_FilterMode            = TexFilterMode::LINEAR_FILTER;
		uint32_t        m_Flags                 = 0;
		uint32_t        m_MaxAnisotropy         = 1;
		TexFilterMode   m_MipmapFilterMode      = TexFilterMode::POINT;
		float           m_MipmapLevelBias       = 0.0f;
		float           m_MinMipmapLevelClamp   = 0.0f;
		float           m_MaxMipmapLevelClamp   = 0.0f;
		float           m_BorderColor[4]        = { 0.0f, 0.0f, 0.0f, 0.0f };

		TextureDesc() = default;
		~TextureDesc() = default;

		TextureDesc(const TextureDesc&) = default;
		TextureDesc& operator=(const TextureDesc&) = default;

		TextureDesc(TextureDesc&&) noexcept = default;
		TextureDesc& operator=(TextureDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initTextureDesc(TextureDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setTexAddressMode(TextureDesc& ro_Desc, TexAddressMode v_U, TexAddressMode v_V, TexAddressMode v_W);
	SPEC_CUDA_BK_RUNTIME_API void setTexFilterMode(TextureDesc& ro_Desc, TexFilterMode v_Mode);
	SPEC_CUDA_BK_RUNTIME_API void setTexFlags(TextureDesc& ro_Desc, uint32_t v_Flags);
	SPEC_CUDA_BK_RUNTIME_API void setTexMipmapParams(TextureDesc& ro_Desc, TexFilterMode v_FilterMode, float v_Bias, float v_MinClamp, float v_MaxClamp);
	SPEC_CUDA_BK_RUNTIME_API void setTexMaxAnisotropy(TextureDesc& ro_Desc, uint32_t v_MaxAnisotropy);
	SPEC_CUDA_BK_RUNTIME_API void setTexBorderColor(TextureDesc& ro_Desc, float v_R, float v_G, float v_B, float v_A);

	// ResourceViewDesc — typed view into a resource, used with cuTexObjectCreate.
	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(16) ResourceViewDesc final {
		ResourceViewFormat  m_Format            = ResourceViewFormat::NONE;
		size_t              m_Width             = 0;
		size_t              m_Height            = 0;
		size_t              m_Depth             = 0;
		uint32_t            m_FirstMipmapLevel  = 0;
		uint32_t            m_LastMipmapLevel   = 0;
		uint32_t            m_FirstLayer        = 0;
		uint32_t            m_LastLayer         = 0;

		ResourceViewDesc() = default;
		~ResourceViewDesc() = default;

		ResourceViewDesc(const ResourceViewDesc&) = default;
		ResourceViewDesc& operator=(const ResourceViewDesc&) = default;

		ResourceViewDesc(ResourceViewDesc&&) noexcept = default;
		ResourceViewDesc& operator=(ResourceViewDesc&&) noexcept = default;
	};

	SPEC_CUDA_BK_RUNTIME_API void initResourceViewDesc(ResourceViewDesc& ro_Desc);
	SPEC_CUDA_BK_RUNTIME_API void setResourceViewFormat(ResourceViewDesc& ro_Desc, ResourceViewFormat v_Format);
	SPEC_CUDA_BK_RUNTIME_API void setResourceViewDimensions(ResourceViewDesc& ro_Desc, size_t v_Width, size_t v_Height, size_t v_Depth);
	SPEC_CUDA_BK_RUNTIME_API void setResourceViewMipmapRange(ResourceViewDesc& ro_Desc, uint32_t v_First, uint32_t v_Last);
	SPEC_CUDA_BK_RUNTIME_API void setResourceViewLayerRange(ResourceViewDesc& ro_Desc, uint32_t v_First, uint32_t v_Last);

	SPEC_CUDA_BK_NODISCARD SPEC_CUDA_BK_RUNTIME_API bool validateArray3dDesc(const Array3dDesc& ro_Desc);
	SPEC_CUDA_BK_NODISCARD SPEC_CUDA_BK_RUNTIME_API bool validateMemCpy3DDesc(const MemCpy3DDesc& ro_Desc);
	SPEC_CUDA_BK_NODISCARD SPEC_CUDA_BK_RUNTIME_API bool validateResourceDesc(const ResourceDesc& ro_Desc);
	SPEC_CUDA_BK_NODISCARD SPEC_CUDA_BK_RUNTIME_API bool validateTextureDesc(const TextureDesc& ro_Desc);
	SPEC_CUDA_BK_NODISCARD SPEC_CUDA_BK_RUNTIME_API bool validateResourceViewDesc(const ResourceViewDesc& ro_Desc);
}