#include "SpectraCudaBackend.h"
#include "CudaStream.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Streams {
	GpuStream DeviceStreams::createStream(StreamFlags v_Flags) {
		CUstream stream;
		const uint32_t flags = Internal::CUDA_InternalHelpers::toStreamFlags(v_Flags);
		GpuStream ro_Stream{};

		Instrumentation::staticSwitch(cuStreamCreate(&stream, flags),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Stream.m_StreamHandle = stream; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Stream;
	}

	GpuStream DeviceStreams::createStreamWithPriority(StreamFlags v_Flags, int v_Priority) {
		CUstream stream;
		const uint32_t flags = Internal::CUDA_InternalHelpers::toStreamFlags(v_Flags);
		GpuStream ro_Stream{};

		Instrumentation::staticSwitch(cuStreamCreateWithPriority(&stream, flags, v_Priority),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Stream.m_StreamHandle = stream; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Stream;
	}

	void DeviceStreams::destroyStream(GpuStream& ro_Stream) {
		if (!ro_Stream.isValid()) return;

		Instrumentation::staticSwitch(cuStreamDestroy(static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Stream.m_StreamHandle = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceStreams::syncStream(const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		Instrumentation::staticSwitch(cuStreamSynchronize(static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	bool DeviceStreams::queryStream(const GpuStream& ro_Stream) {
		if (!ro_Stream.isValid()) return true;

		bool ready = false;
		Instrumentation::staticSwitch(cuStreamQuery(static_cast<CUstream>(ro_Stream.m_StreamHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ready = true; }),
			Instrumentation::caseOf<CUDA_ERROR_NOT_READY>([&]{ ready = false; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ready;
	}

	void DeviceStreams::streamWaitEvent(GpuStream& ro_Stream, const GpuEvent& ro_Event) {
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Event.isValid());

		Instrumentation::staticSwitch(cuStreamWaitEvent(
				static_cast<CUstream>(ro_Stream.m_StreamHandle),
				static_cast<CUevent>(ro_Event.m_EventHandle),
				0
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceStreams::addStreamCallback(GpuStream& ro_Stream, void* p_Callback, void* p_UserData) {
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		SPEC_CUDA_BK_ASSERT(p_Callback != nullptr);

		Instrumentation::staticSwitch(cuStreamAddCallback(
				static_cast<CUstream>(ro_Stream.m_StreamHandle),
				reinterpret_cast<CUstreamCallback>(p_Callback),
				p_UserData,
				0
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	void DeviceStreams::beginStreamCapture(GpuStream& ro_Stream, StreamCaptureMode v_Mode) {
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		const CUstreamCaptureMode mode = Internal::CUDA_InternalHelpers::toCudaStreamCaptureMode(v_Mode);
		Instrumentation::staticSwitch(cuStreamBeginCapture(static_cast<CUstream>(ro_Stream.m_StreamHandle), mode),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	GpuGraph DeviceStreams::endStreamCapture(GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		CUgraph graph;
		GpuGraph ro_Graph{};
		Instrumentation::staticSwitch(cuStreamEndCapture(static_cast<CUstream>(ro_Stream.m_StreamHandle), &graph),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Graph.m_GraphHandle = graph; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Graph;
	}
}
