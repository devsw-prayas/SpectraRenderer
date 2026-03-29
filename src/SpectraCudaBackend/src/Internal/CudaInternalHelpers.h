#pragma once
#include "SpectraCudaBackend.h"
#include "CudaUtils.h"

#ifdef ALLOW_HELPERS
#include <cuda.h>

namespace Spectra::Cuda::Internal {
	class CUDA_DeviceRegistry {
	public:
		static constexpr int MAX_DEVICE_COUNT = 32;
		static inline CUdevice s_Devices[MAX_DEVICE_COUNT] = {};
		static inline int s_DeviceCount = 0;
		static constexpr size_t CUDA_UUID_LENGTH = 16;

		static bool validateDevice(int v_Ordinal) {
			if (v_Ordinal >= s_DeviceCount || v_Ordinal < 0) return false;
			return true;
		}
	};

	class CUDA_InternalHelpers final {
	public:
		static CUdevice_attribute toCudaAttr(Utils::CudaDeviceAttribute attr);
		static CUctx_flags_enum toCudaContextScheduleFlags(Utils::ContextSchedulingFlags flag);
		static CUctx_flags_enum toCudaContextCreationFlags(Utils::ContextCreationFlags flag);

	};
}

#else
#error "This is an internal backend header. To use, define ALLOW_SYSCALL before inclusion"
#endif
