#include "SpectraCudaBackend.h"
#include "CudaContextManager.h"

#include "CudaBootstrap.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Context {
	Utils::CudaContext ContextManager::createCudaContext(
		Utils::DeviceHandle v_Handle,
		Utils::ContextSchedulingFlags v_SchedFlag,
		Utils::ContextCreationFlags v_CreateFlag,
		const Utils::CtxCreateParams* p_Params) {
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUcontext context{};
		uint32_t flags = 0;
		if (v_CreateFlag == Utils::ContextCreationFlags::NONE) {
			flags = Internal::CUDA_InternalHelpers::toCudaContextScheduleFlags(v_SchedFlag);
		} else {
			flags = Internal::CUDA_InternalHelpers::toCudaContextScheduleFlags(v_SchedFlag) |
				Internal::CUDA_InternalHelpers::toCudaContextCreationFlags(v_CreateFlag);
		}

		Utils::CudaContext ctx;
		ctx.m_Handle = nullptr;
		CUctxCreateParams params{};
		CUexecAffinityParam affinityBuf[8] = {};
		if (p_Params) {
			if (p_Params->m_ExecAffinityParams && p_Params->m_NumExecAffinityParams > 0) {
				const int count = p_Params->m_NumExecAffinityParams;
				for (int i = 0; i < count; ++i) {
					affinityBuf[i].type              = static_cast<CUexecAffinityType>(p_Params->m_ExecAffinityParams[i].m_Type);
					affinityBuf[i].param.smCount.val = p_Params->m_ExecAffinityParams[i].m_SmCount.m_Val;
				}
				params.execAffinityParams    = affinityBuf;
				params.numExecAffinityParams = count;
			}
			params.cigParams = static_cast<CUctxCigParam*>(p_Params->m_CigParams);
		}
		Instrumentation::staticSwitch(cuCtxCreate(&context, &params, flags, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ctx.m_Handle = context; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ctx;
	}

	void ContextManager::setCurrentCudaContext(const Utils::CudaContext& ro_Context) {
		SPEC_CUDA_BK_ASSERT(ro_Context.m_Handle);
		Instrumentation::staticSwitch(cuCtxSetCurrent(static_cast<CUcontext>(ro_Context.m_Handle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	Utils::CudaContext ContextManager::getCurrentCudaContext() {
		CUcontext context{};
		Utils::CudaContext ctx;
		ctx.m_Handle = nullptr;
		Instrumentation::staticSwitch(cuCtxGetCurrent(&context),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ctx.m_Handle = context; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ctx;
	}

	void ContextManager::pushCudaContext(const Utils::CudaContext& ro_Context) {
		SPEC_CUDA_BK_ASSERT(ro_Context.m_Handle);
		Instrumentation::staticSwitch(cuCtxPushCurrent_v2(static_cast<CUcontext>(ro_Context.m_Handle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	Utils::CudaContext ContextManager::popCudaContext() {
		CUcontext context{};
		Utils::CudaContext ctx;
		ctx.m_Handle = nullptr;
		Instrumentation::staticSwitch(cuCtxPopCurrent_v2(&context),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ctx.m_Handle = context; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ctx;
	}

	void ContextManager::cudaContextSynchronize() {
		Instrumentation::staticSwitch(cuCtxSynchronize(),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void ContextManager::destroyCudaContext(Utils::CudaContext& ro_Context) {
		SPEC_CUDA_BK_ASSERT(ro_Context.m_Handle);
		Instrumentation::staticSwitch(cuCtxDestroy_v2(static_cast<CUcontext>(ro_Context.m_Handle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Context.m_Handle = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}
}
