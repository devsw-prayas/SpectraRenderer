#include "SpectraCudaBackend.h"
#include "CudaUtils.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Utils {
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
}
