#include "CudaLinker.h"
#include "CudaModule.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Modules {

	GpuLinkState DeviceLinker::createLinkState(const JitOptions& ro_Options) {
		Internal::CUDA_JitOptionPacker packer(ro_Options);

		CUlinkState linkState = nullptr;
		Instrumentation::staticSwitch(cuLinkCreate(packer.getCount(), packer.getOptions(), packer.getValues(), &linkState),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GpuLinkState state;
		state.m_LinkStateHandle = static_cast<void*>(linkState);
		return state;
	}

	void DeviceLinker::addData(const GpuLinkState& ro_State, JitInputType v_Type, void* p_Data, size_t v_Size, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_State.isValid() && "Invalid Link State Handle");
		SPEC_CUDA_BK_ASSERT(p_Data && v_Size > 0 && "Invalid Link Data");

		CUjitInputType type = Internal::CUDA_InternalHelpers::toCudaJitInputType(v_Type);
		Instrumentation::staticSwitch(cuLinkAddData(static_cast<CUlinkState>(ro_State.m_LinkStateHandle), type, p_Data, v_Size, p_Name, 0, nullptr, nullptr),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	GpuModule DeviceLinker::completeAndLoad(GpuLinkState& ro_State) {
		SPEC_CUDA_BK_ASSERT(ro_State.isValid() && "Invalid Link State Handle");

		void* cubinOut = nullptr;
		size_t sizeOut = 0;

		Instrumentation::staticSwitch(cuLinkComplete(static_cast<CUlinkState>(ro_State.m_LinkStateHandle), &cubinOut, &sizeOut),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		return DeviceModules::loadModuleData(cubinOut);
	}

	void DeviceLinker::destroyLinkState(GpuLinkState& ro_State) {
		if (ro_State.isValid()) {
			Instrumentation::staticSwitch(cuLinkDestroy(static_cast<CUlinkState>(ro_State.m_LinkStateHandle)),
				Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
				Instrumentation::otherwise(Internal::trapCudaError));
			ro_State.m_LinkStateHandle = nullptr;
		}
	}

}
