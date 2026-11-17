#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <memory>
#include <thread>

class TestFixture61 final
	: public Hades::Runtime::IFixture<TestFixture61, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture61(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		bool attached = false, nativeBound = false, handleValid = false, closeRefused = false;
		bool nested = false, stillAttached = false, released = false;
		std::thread([&]() {
			attached = this_thread::attach();
			nativeBound = Frame::this_frame::currentFrame() == std::addressof(Frame::this_frame::nativeContext());
			handleValid = NativeThread::getNumaNode(this_thread::currentHandle()) == 0u;
			closeRefused = !NativeThread::closeHandle(this_thread::currentHandle());
			nested = this_thread::attach();
			this_thread::detach();
			stillAttached = Frame::this_frame::currentFrame() != nullptr;
			this_thread::detach();
			released = Frame::this_frame::currentFrame() == nullptr;
		}).join();
		CORIUM_ASSERT(attached && nativeBound && handleValid && "attach binds a native context and a handle");
		CORIUM_ASSERT(closeRefused && "An adopted thread's handle cannot be closed");
		CORIUM_ASSERT(nested && stillAttached && released && "attach is counted: only the last detach releases");

		constexpr int kThreads = 300;
		int exitAttached = 0;
		for (int i = 0; i < kThreads; ++i) {
			bool ok = false;
			std::thread([&ok] { ok = this_thread::attach() && Frame::this_frame::currentFrame() != nullptr; }).join();
			exitAttached += ok ? 1 : 0;
		}
		CORIUM_ASSERT(exitAttached == kThreads && "Threads exiting attached must free their registry slot");
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
