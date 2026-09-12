#include "SpectraCudaBackend.h"
#include "CudaEvent.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Events {
	GpuEvent DeviceEvents::createEvent(uint32_t v_Flags) {
		CUevent hEvent;
		GpuEvent ro_Event{};

		Instrumentation::staticSwitch(cuEventCreate(&hEvent, static_cast<unsigned int>(v_Flags)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Event.m_EventHandle = hEvent; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Event;
	}

	void DeviceEvents::destroyEvent(GpuEvent& ro_Event) {
		if (!ro_Event.isValid()) return;

		Instrumentation::staticSwitch(cuEventDestroy(static_cast<CUevent>(ro_Event.m_EventHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Event.m_EventHandle = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceEvents::recordEvent(const GpuEvent& ro_Event, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Event.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		Instrumentation::staticSwitch(cuEventRecord(
				static_cast<CUevent>(ro_Event.m_EventHandle),
				static_cast<CUstream>(ro_Stream.m_StreamHandle)
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceEvents::syncEvent(const GpuEvent& ro_Event) {
		SPEC_CUDA_BK_ASSERT(ro_Event.isValid());

		Instrumentation::staticSwitch(cuEventSynchronize(static_cast<CUevent>(ro_Event.m_EventHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	bool DeviceEvents::queryEvent(const GpuEvent& ro_Event) {
		if (!ro_Event.isValid()) return true;

		bool ready = false;
		Instrumentation::staticSwitch(cuEventQuery(static_cast<CUevent>(ro_Event.m_EventHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ready = true; }),
			Instrumentation::caseOf<CUDA_ERROR_NOT_READY>([&]{ ready = false; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ready;
	}

	float DeviceEvents::elapsedTime(const GpuEvent& ro_Start, const GpuEvent& ro_End) {
		SPEC_CUDA_BK_ASSERT(ro_Start.isValid());
		SPEC_CUDA_BK_ASSERT(ro_End.isValid());

		float ms = 0.0f;
		Instrumentation::staticSwitch(cuEventElapsedTime(&ms,
				static_cast<CUevent>(ro_Start.m_EventHandle),
				static_cast<CUevent>(ro_End.m_EventHandle)
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ms;
	}

	GpuIpcEventHandle DeviceEvents::getIpcHandle(const GpuEvent& ro_Event) {
		SPEC_CUDA_BK_ASSERT(ro_Event.isValid());

		CUipcEventHandle handle;
		GpuIpcEventHandle ro_Handle{};
		Instrumentation::staticSwitch(cuIpcGetEventHandle(&handle, static_cast<CUevent>(ro_Event.m_EventHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ std::memcpy(ro_Handle.m_Reserved, &handle, sizeof(CUipcEventHandle)); }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Handle;
	}

	GpuEvent DeviceEvents::openIpcHandle(const GpuIpcEventHandle& ro_Handle) {
		CUipcEventHandle handle;
		std::memcpy(&handle, ro_Handle.m_Reserved, sizeof(CUipcEventHandle));

		CUevent hEvent;
		GpuEvent ro_Event{};
		Instrumentation::staticSwitch(cuIpcOpenEventHandle(&hEvent, handle),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Event.m_EventHandle = hEvent; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Event;
	}
}
