#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Modules {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceModules final {
	public:
		static GpuModule loadModule(const char* p_Path);
		static GpuModule loadModuleData(const void* p_Image);
		static GpuModule loadModuleDataEx(const void* p_Image, const JitOptions& ro_Options);
		
		static GpuFunction getFunction(const GpuModule& ro_Module, const char* p_Name);
		static GlobalMemorySegment getGlobal(const GpuModule& ro_Module, const char* p_Name);
		
		static void unloadModule(GpuModule& ro_Module);
	};

}
