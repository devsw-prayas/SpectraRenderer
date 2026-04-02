#include "SpectraCudaBackend.h"
#include "CudaUtils.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Utils {
	DeviceHandle DeviceHandle::makeCpu() {
		DeviceHandle handle;
		handle.m_HandleValue = CU_DEVICE_CPU;
		return handle;
	}

	uint32_t CudaHelpers::computeAllocFlag(std::initializer_list<HostAllocFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostAllocationFlags(each);
		return mask;
	}

	uint32_t CudaHelpers::computeRegFlag(std::initializer_list<HostRegisterFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostRegisterFlags(each);
		return mask;
	}

	SPEC_CUDA_BK_RUNTIME_API void initAllocDesc(AllocDesc& ro_Desc) {
		// Zero + explicit defaults (no memset, keep semantic clarity)
		ro_Desc.m_Type = AllocationType::INVALID;
		ro_Desc.m_HandleType = AllocationHandleType::NONE;

		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = DeviceHandle{}; // default constructed

		ro_Desc.m_win32meta = nullptr;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAllocationType(AllocDesc& ro_Desc, AllocationType v_Type) {
		ro_Desc.m_Type = v_Type;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAllocationHandleType(AllocDesc& ro_Desc, AllocationHandleType v_Type) {
		ro_Desc.m_HandleType = v_Type;
	}

	SPEC_CUDA_BK_RUNTIME_API void setLocation(AllocDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = ro_Handle;
	}

	SPEC_CUDA_BK_RUNTIME_API void initAccessDesc(AccessDesc& ro_Desc) {
		// Default: no access, GPU location
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = DeviceHandle{};
		ro_Desc.flags = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAccessLocation(AccessDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = ro_Handle;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAccessFlags(AccessDesc& ro_Desc, uint64_t v_Flags) {
		ro_Desc.flags = v_Flags;
	}

	uint64_t CudaHelpers::computeAccessFlags(std::initializer_list<AccessFlagBits> flags) {
		uint64_t result = 0;

		for (const auto& f : flags) {
			result |= static_cast<uint64_t>(f);
		}

		return result;
	}

}
