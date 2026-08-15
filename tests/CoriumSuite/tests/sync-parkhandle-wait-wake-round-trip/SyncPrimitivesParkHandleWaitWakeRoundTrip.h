#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <atomic>

#include <CoriumRuntime.h>

class SyncPrimitivesParkHandleWaitWakeRoundTrip final
	: public Hades::Runtime::IFixture<SyncPrimitivesParkHandleWaitWakeRoundTrip, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CountDownLatch m_Latch{ 1 };
	std::atomic<bool> m_WaiterReturned{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_Handle{};
	bool m_Joined{ false };

public:
	explicit SyncPrimitivesParkHandleWaitWakeRoundTrip(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_WaiterReturned.store(false, std::memory_order_relaxed);
		m_Joined = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_Handle = m_Factory.createAndStart(
			createClosure<void()>([this]() {
				m_Latch.await();
				m_WaiterReturned.store(true, std::memory_order_release);
			}),
			"LatchWaiter"
		);

		CORIUM_ASSERT(NativeThread::isAlive(m_Handle));

		m_Latch.countDown();

		m_Joined = NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_Joined);
		CORIUM_ASSERT(m_WaiterReturned.load(std::memory_order_acquire));
		CORIUM_ASSERT(m_Latch.getCount() == 0);
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


