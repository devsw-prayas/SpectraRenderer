#include "SpectraCudaBackend.h"
#include "CudaBootstrap.h"
#include "SpecCudaCompiler.h"
#include "SpecCudaDiagnostics.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda {
	bool Bootstrap::CudaDriver::initCuda() {
		bool hasDevice = true;
		Instrumentation::staticSwitch(cuInit(0),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::caseOf<CUDA_ERROR_NO_DEVICE>([&]{ hasDevice = false; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		// ADD other handling behavior later TODO
		if (!hasDevice) return false;

		// Get all CUDA Devices
		int count = 0;
		Instrumentation::staticSwitch(cuDeviceGetCount(&count),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));

		Internal::CUDA_DeviceRegistry::s_DeviceCount = count > Internal::CUDA_DeviceRegistry::MAX_DEVICE_COUNT ? Internal::CUDA_DeviceRegistry::MAX_DEVICE_COUNT : count;
		for (int i = 0; i < Internal::CUDA_DeviceRegistry::s_DeviceCount; i++) {
			Instrumentation::staticSwitch(cuDeviceGet(&Internal::CUDA_DeviceRegistry::s_Devices[i], i),
				Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
				Instrumentation::otherwise(Internal::trapCudaError));
		}

		return true;
	}

	uint32_t Bootstrap::CudaDriver::getCudaDriverVersion() {
		int version = 0;
		uint32_t driverVersion = 0;
		Instrumentation::staticSwitch(cuDriverGetVersion(&version),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ driverVersion = static_cast<uint32_t>(version); }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return driverVersion;
	}

	int Bootstrap::CudaDeviceManager::getCudaDeviceCount() {
		return Internal::CUDA_DeviceRegistry::s_DeviceCount;
	}

	Utils::DeviceHandle Bootstrap::CudaDeviceManager::getCudaDevice(int v_Ordinal) {
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Ordinal));
		Utils::DeviceHandle handle{};
		handle.m_HandleValue = v_Ordinal;
		return handle;
	}

	void Bootstrap::CudaDeviceManager::getCudaDeviceName(char* p_Name, uint32_t v_Len, Utils::DeviceHandle v_Handle) {
		SPEC_CUDA_BK_ASSERT(p_Name != nullptr);
		SPEC_CUDA_BK_ASSERT(v_Len > 0 && v_Len <= INT_MAX);
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		Instrumentation::staticSwitch(cuDeviceGetName(p_Name, static_cast<int>(v_Len), Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(int* p_Value, Utils::CudaDeviceAttribute v_Attr, Utils::DeviceHandle v_Handle) {
		SPEC_CUDA_BK_ASSERT(p_Value != nullptr);
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		Instrumentation::staticSwitch(cuDeviceGetAttribute(p_Value, Internal::CUDA_InternalHelpers::toCudaAttr(v_Attr), Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	size_t Bootstrap::CudaDeviceManager::getCudaDeviceTotalMemory(Utils::DeviceHandle v_Handle) {
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		size_t mem = 0;
		Instrumentation::staticSwitch(cuDeviceTotalMem_v2(&mem, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return mem;
	}

	Utils::DeviceUUID Bootstrap::CudaDeviceManager::getCudaDeviceUUID(Utils::DeviceHandle v_Handle) {
		SPEC_CUDA_BK_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUuuid uuid{};
		Utils::DeviceUUID deviceUUID{};
		Instrumentation::staticSwitch(cuDeviceGetUuid_v2(&uuid, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{
				// Extremely dangerous, the struct layout must never be changed
				memcpy(&deviceUUID, uuid.bytes, 16);
			}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return deviceUUID;
	}
}
