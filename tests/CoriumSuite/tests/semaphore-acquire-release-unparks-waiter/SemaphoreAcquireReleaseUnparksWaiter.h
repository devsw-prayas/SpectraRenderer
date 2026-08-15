#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>
#include <atomic>

class SemaphoreAcquireReleaseUnparksWaiter final
	: public Hades::Runtime::IFixture<SemaphoreAcquireReleaseUnparksWaiter, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::Semaphore m_Sem{ 0 };
	std::atomic<bool> m_Acquired{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_Handle{};
	bool m_Joined{ false };

public:
	explicit SemaphoreAcquireReleaseUnparksWaiter(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_Acquired.store(false, std::memory_order_relaxed);
		m_Joined = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_Handle = m_Factory.createAndStart(
			createClosure<void()>([this]() {
				m_Sem.acquire();
				m_Acquired.store(true, std::memory_order_release);
			}),
			"SemaphoreWaiter"
		);

		// Give the spawned thread a moment to block on acquire()
		m_Sem.release();

		m_Joined = NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_Joined && "NativeThread::joinThread failed");
		CORIUM_ASSERT(m_Acquired.load(std::memory_order_acquire) && "Waiter thread did not set acquired flag");
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


