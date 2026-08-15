#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>
#include <atomic>

class AThreadFactoryCreateAndStartIsReusableAcrossCalls final
	: public Hades::Runtime::IFixture<AThreadFactoryCreateAndStartIsReusableAcrossCalls, Hades::Runtime::NullDeviceAdapter> {
private:
	std::atomic<bool> m_firstRan{ false };
	std::atomic<bool> m_secondRan{ false };
	Corium::Core::Factory::DefaultThreadFactory m_factory;
	Corium::Core::ThreadHandle m_handle1{};
	Corium::Core::ThreadHandle m_handle2{};
	bool m_joined1{ false };
	bool m_joined2{ false };

public:
	explicit AThreadFactoryCreateAndStartIsReusableAcrossCalls(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_firstRan.store(false, std::memory_order_relaxed);
		m_secondRan.store(false, std::memory_order_relaxed);
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_handle1 = m_factory.createAndStart(
			createClosure<void()>([this]() {
				m_firstRan.store(true, std::memory_order_release);
			}),
			"TestReusableThread1"
		);

		m_joined1 = NativeThread::joinThread(m_handle1);
		CORIUM_ASSERT(m_joined1 && "First thread failed to join");

		m_handle2 = m_factory.createAndStart(
			createClosure<void()>([this]() {
				m_secondRan.store(true, std::memory_order_release);
			}),
			"TestReusableThread2"
		);

		m_joined2 = NativeThread::joinThread(m_handle2);
		CORIUM_ASSERT(m_joined2 && "Second thread failed to join");

		CORIUM_ASSERT(m_firstRan.load(std::memory_order_acquire) && "First thread closure was not executed");
		CORIUM_ASSERT(m_secondRan.load(std::memory_order_acquire) && "Second thread closure was not executed");
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


