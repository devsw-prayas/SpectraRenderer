#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumSync.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>

class NativeThreadIsAliveTrueImmediatelyAfterCreate final
	: public Hades::Runtime::IFixture<NativeThreadIsAliveTrueImmediatelyAfterCreate, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CountDownLatch m_Latch{ 1 };
	Corium::Core::Factory::DefaultThreadFactory m_Factory{};
	Corium::Core::ThreadHandle m_Handle{};
	bool m_IsAlive{ false };
	bool m_Joined{ false };

public:
	explicit NativeThreadIsAliveTrueImmediatelyAfterCreate(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_IsAlive = false;
		m_Joined = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_Handle = m_Factory.createAndStart(
			createClosure<void()>([this]() {
				m_Latch.await();
			}),
			"IsAliveTestThread"
		);

		m_IsAlive = NativeThread::isAlive(m_Handle);
		CORIUM_ASSERT(m_IsAlive && "NativeThread::isAlive(handle) must be true immediately after create");

		m_Latch.countDown();
		m_Joined = NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_Joined && "Failed to join thread after releasing latch");
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

