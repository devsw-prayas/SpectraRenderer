#include "CudaLinker.h"
#include "CudaModule.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Modules {

	GpuLinkState DeviceLinker::createLinkState(const JitOptions& ro_Options) {
		Internal::CUDA_JitOptionPacker packer(ro_Options);

		CUlinkState linkState = nullptr;
		CUresult res = cuLinkCreate(packer.getCount(), packer.getOptions(), packer.getValues(), &linkState);
		CUDA_ERROR_TRAP(res)

		GpuLinkState state;
		state.m_LinkStateHandle = static_cast<void*>(linkState);
		return state;
	}

	void DeviceLinker::addData(const GpuLinkState& ro_State, JitInputType v_Type, void* p_Data, size_t v_Size, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_State.isValid() && "Invalid Link State Handle");
		SPEC_CUDA_BK_ASSERT(p_Data && v_Size > 0 && "Invalid Link Data");

		CUjitInputType type = Internal::CUDA_InternalHelpers::toCudaJitInputType(v_Type);
		CUresult res = cuLinkAddData(static_cast<CUlinkState>(ro_State.m_LinkStateHandle), type, p_Data, v_Size, p_Name, 0, nullptr, nullptr);
		CUDA_ERROR_TRAP(res)
	}

	GpuModule DeviceLinker::completeAndLoad(GpuLinkState& ro_State) {
		SPEC_CUDA_BK_ASSERT(ro_State.isValid() && "Invalid Link State Handle");

		void* cubinOut = nullptr;
		size_t sizeOut = 0;

		CUresult res = cuLinkComplete(static_cast<CUlinkState>(ro_State.m_LinkStateHandle), &cubinOut, &sizeOut);
		CUDA_ERROR_TRAP(res)

		return DeviceModules::loadModuleData(cubinOut);
	}

	void DeviceLinker::destroyLinkState(GpuLinkState& ro_State) {
		if (ro_State.isValid()) {
			CUresult res = cuLinkDestroy(static_cast<CUlinkState>(ro_State.m_LinkStateHandle));
			CUDA_ERROR_TRAP(res)
			ro_State.m_LinkStateHandle = nullptr;
		}
	}

}
