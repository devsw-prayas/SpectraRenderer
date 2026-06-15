#pragma once
#include "CudaUtils.h"
#include "SpectraCudaBackend.h"

namespace Spectra::Cuda::Context {
	class SPEC_CUDA_BK_RUNTIME_API ContextManager final {
	public:
		static Utils::CudaContext createCudaContext(
			Utils::DeviceHandle v_Handle,
			Utils::ContextSchedulingFlags v_SchedFlag,
			Utils::ContextCreationFlags v_CreatFlag = Utils::ContextCreationFlags::NONE,
			const Utils::CtxCreateParams* p_Params = nullptr);
		static void setCurrentCudaContext(const Utils::CudaContext& ro_Context);
		static Utils::CudaContext getCurrentCudaContext();
		static void pushCudaContext(const Utils::CudaContext& ro_Context);
		static Utils::CudaContext popCudaContext();
		static void cudaContextSynchronize();
		static void destroyCudaContext(Utils::CudaContext& ro_Context);
	};
}
