#pragma once
#include "CudaUtils.h"
#include "SpectraCudaBackend.h"

namespace Spectra::Cuda::Bootstrap {
	class SPEC_CUDA_BK_RUNTIME_API CudaDriver final {
	public:
		static bool initCuda();
		static uint32_t getCudaDriverVersion();

		CudaDriver() = delete;
		~CudaDriver() = delete;

		CudaDriver(const CudaDriver&) = delete;
		CudaDriver& operator=(const CudaDriver&) = delete;

		CudaDriver(CudaDriver&&) noexcept = delete;
		CudaDriver& operator=(CudaDriver&&) noexcept = delete;
	};

	class SPEC_CUDA_BK_RUNTIME_API CudaDeviceManager final {
	public:
		static int getCudaDeviceCount();
		static Utils::DeviceHandle getCudaDevice(int v_Ordinal);
		static void getCudaDeviceName(char* p_Name, uint32_t v_Len,  Utils::DeviceHandle v_Handle);
		static void getCudaDeviceAttribute(int* p_Value, Utils::CudaDeviceAttribute v_Attr, Utils::DeviceHandle v_Handle);
		static size_t getCudaDeviceTotalMemory(Utils::DeviceHandle v_Handle);
		static Utils::DeviceUUID getCudaDeviceUUID(Utils::DeviceHandle v_Handle);
	};
}
