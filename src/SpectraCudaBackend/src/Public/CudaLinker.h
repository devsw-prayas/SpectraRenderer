#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Modules {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceLinker final {
	public:
		static GpuLinkState createLinkState(const JitOptions& ro_Options);
		static void addData(const GpuLinkState& ro_State, JitInputType v_Type, void* p_Data, size_t v_Size, const char* p_Name);
		static GpuModule completeAndLoad(GpuLinkState& ro_State);
		static void destroyLinkState(GpuLinkState& ro_State);
	};

}
