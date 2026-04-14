#include "CudaModule.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Modules {

	GpuModule DeviceModules::loadModule(const char* p_Path) {
		SPEC_CUDA_BK_ASSERT(p_Path && "Invalid Module Path");

		CUmodule moduleHandle = nullptr;
		CUresult res = cuModuleLoad(&moduleHandle, p_Path);
		CUDA_ERROR_TRAP(res);

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuModule DeviceModules::loadModuleData(const void* p_Image) {
		SPEC_CUDA_BK_ASSERT(p_Image && "Invalid Module Image Data");

		CUmodule moduleHandle = nullptr;
		CUresult res = cuModuleLoadData(&moduleHandle, p_Image);
		CUDA_ERROR_TRAP(res);

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuModule DeviceModules::loadModuleDataEx(const void* p_Image, const JitOptions& ro_Options) {
		SPEC_CUDA_BK_ASSERT(p_Image && "Invalid Module Image Data");

		Internal::CUDA_JitOptionPacker packer(ro_Options);

		CUmodule moduleHandle = nullptr;
		CUresult res = cuModuleLoadDataEx(&moduleHandle, p_Image, packer.getCount(), packer.getOptions(), packer.getValues());
		CUDA_ERROR_TRAP(res);

		GpuModule gpuModule;
		gpuModule.m_ModuleHandle = static_cast<void*>(moduleHandle);
		return gpuModule;
	}

	GpuFunction DeviceModules::getFunction(const GpuModule& ro_Module, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_Module.isValid() && "Invalid Module Handle");
		SPEC_CUDA_BK_ASSERT(p_Name && "Invalid Function Name");

		CUfunction functionHandle = nullptr;
		CUresult res = cuModuleGetFunction(&functionHandle, static_cast<CUmodule>(ro_Module.m_ModuleHandle), p_Name);
		CUDA_ERROR_TRAP(res);

		GpuFunction gpuFunction;
		gpuFunction.m_FunctionHandle = static_cast<void*>(functionHandle);
		return gpuFunction;
	}

	GlobalMemorySegment DeviceModules::getGlobal(const GpuModule& ro_Module, const char* p_Name) {
		SPEC_CUDA_BK_ASSERT(ro_Module.isValid() && "Invalid Module Handle");
		SPEC_CUDA_BK_ASSERT(p_Name && "Invalid Global Name");

		CUdeviceptr ptr = 0;
		size_t size = 0;

		CUresult res = cuModuleGetGlobal(&ptr, &size, static_cast<CUmodule>(ro_Module.m_ModuleHandle), p_Name);
		CUDA_ERROR_TRAP(res);

		GlobalMemorySegment segment;
		segment.m_Address   = GpuAddress(ptr);
		segment.m_SizeBytes = size;

		return segment;
	}

	void DeviceModules::unloadModule(GpuModule& ro_Module) {
		if (ro_Module.isValid()) {
			CUresult res = cuModuleUnload(static_cast<CUmodule>(ro_Module.m_ModuleHandle));
			CUDA_ERROR_TRAP(res);
			ro_Module.m_ModuleHandle = nullptr;
		}
	}

}
