#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <atomic>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class TestFixture64 final
	: public Hades::Runtime::IFixture<TestFixture64, Hades::Runtime::NullDeviceAdapter> {
private:
	static inline std::atomic<int> s_Cpp{ -1 };
	static inline std::atomic<int> s_Seh{ -1 };
	static inline std::atomic<int> s_Depth{ -1 };
	static inline std::atomic<int> s_BoundsOk{ -1 };

	CORIUM_NOINLINE static int sehCatchesAccessViolation() {
		__try {
			*static_cast<volatile int*>(nullptr) = 1;
		} __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			return 1;
		}
		return 0;
	}

	CORIUM_NOINLINE static int cppCatchesThrow(int v_Value) {
		try {
			if (v_Value != 0) throw v_Value;
		} catch (int caught) {
			return caught == v_Value ? 1 : 0;
		}
		return 0;
	}

public:
	explicit TestFixture64(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		s_Cpp.store(-1);
		s_Seh.store(-1);
		s_Depth.store(-1);
		s_BoundsOk.store(-1);

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		CORIUM_ASSERT(Frame::validate(desc));

		auto created = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([]() {
			NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
			volatile int local = 0;
			const auto here = reinterpret_cast<uintptr_t>(&local);
			const bool inBounds = here < reinterpret_cast<uintptr_t>(tib->StackBase) && here >= reinterpret_cast<uintptr_t>(tib->StackLimit);
			s_BoundsOk.store(inBounds ? 1 : 0);
			s_Cpp.store(cppCatchesThrow(42));
			s_Seh.store(sehCatchesAccessViolation());
			void* trace[32] = {};
			s_Depth.store(static_cast<int>(RtlCaptureStackBackTrace(0, 32, trace, nullptr)));
		}), desc);
		CORIUM_ASSERT(created.has_value());

		Frame::FrameHandle* frame = created.value();
		Factory::DefaultThreadFactory factory{};
		const ThreadHandle t = factory.createAndStart(makeClosure<void()>([frame]() {
			Frame::this_frame::captureFrame(frame);
		}), "FrameException");
		CORIUM_ASSERT(NativeThread::joinThread(t));
		CORIUM_ASSERT(s_BoundsOk.load() == 1 && "The TEB must describe the frame's stack while it runs");
		CORIUM_ASSERT(s_Cpp.load() == 1 && "C++ exceptions must unwind inside a frame");
		CORIUM_ASSERT(s_Seh.load() == 1 && "SEH must catch an access violation inside a frame");
		CORIUM_ASSERT(s_Depth.load() > 0 && "Stack walks must work inside a frame");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
