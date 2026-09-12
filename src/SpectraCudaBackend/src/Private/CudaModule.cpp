#include "CudaModule.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Modules {

	GpuModule DeviceModules::loadModule(const char* p_Path) {
		SPEC_CUDA_BK_ASSERT(p_Path && "Invalid Module Path");

		CUmodule moduleHandle = nullptr;
		Instrumentation::staticSwitch(cuModuleLoad(&moduleHandle, p_Path),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuModule DeviceModules::loadModuleData(const void* p_Image) {
		SPEC_CUDA_BK_ASSERT(p_Image && "Invalid Module Image Data");

		CUmodule moduleHandle = nullptr;
		Instrumentation::staticSwitch(cuModuleLoadData(&moduleHandle, p_Image),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuModule DeviceModules::loadModuleDataEx(const void* p_Image, const JitOptions& ro_Options) {
		SPEC_CUDA_BK_ASSERT(p_Image && "Invalid Module Image Data");

		Internal::CUDA_JitOptionPacker packer(ro_Options);

		CUmodule moduleHandle = nullptr;
		Instrumentation::staticSwitch(cuModuleLoadDataEx(&moduleHandle, p_Image, packer.getCount(), packer.getOptions(), packer.getValues()),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuFunction DeviceModules::getFunction(const GpuModule& ro_Module, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_Module.isValid() && "Invalid Module Handle");
		SPEC_CUDA_BK_ASSERT(p_Name && "Invalid Function Name");

		CUfunction functionHandle = nullptr;
		Instrumentation::staticSwitch(cuModuleGetFunction(&functionHandle, static_cast<CUmodule>(ro_Module.m_ModuleHandle), p_Name),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GpuFunction gpuFunction;
		gpuFunction.m_FunctionHandle = static_cast<void*>(functionHandle);
		return gpuFunction;
	}

	GlobalMemorySegment DeviceModules::getGlobal(const GpuModule& ro_Module, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_Module.isValid() && "Invalid Module Handle");
		SPEC_CUDA_BK_ASSERT(p_Name && "Invalid Global Name");

		CUdeviceptr ptr = 0;
		size_t size = 0;

		Instrumentation::staticSwitch(cuModuleGetGlobal(&ptr, &size, static_cast<CUmodule>(ro_Module.m_ModuleHandle), p_Name),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		GlobalMemorySegment segment;
		segment.m_Address   = GpuAddress(ptr);
		segment.m_SizeBytes = size;

		return segment;
	}

	void DeviceModules::unloadModule(GpuModule& ro_Module) {
		if (ro_Module.isValid()) {
			Instrumentation::staticSwitch(cuModuleUnload(static_cast<CUmodule>(ro_Module.m_ModuleHandle)),
				Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
				Instrumentation::otherwise(Internal::trapCudaError));
			ro_Module.m_ModuleHandle = nullptr;
		}
	}

}
