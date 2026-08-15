#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>
#include <atomic>

class DefaultThreadFactoryCreateAndStartLaunchesRealThread final
	: public Hades::Runtime::IFixture<DefaultThreadFactoryCreateAndStartLaunchesRealThread, Hades::Runtime::NullDeviceAdapter> {
private:
	std::atomic<bool> m_threadRan{ false };
	Corium::Core::Factory::DefaultThreadFactory m_factory{};
	Corium::Core::ThreadHandle m_handle{};
	bool m_joined{ false };

public:
	explicit DefaultThreadFactoryCreateAndStartLaunchesRealThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_threadRan.store(false, std::memory_order_relaxed);
		m_handle = {};
		m_joined = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		Factory::AThreadFactory& baseFactory = m_factory;

		m_handle = baseFactory.createAndStart(
			createClosure<void()>([this]() {
				m_threadRan.store(true, std::memory_order_release);
			}),
			"TestDefaultThread"
		);

		m_joined = NativeThread::joinThread(m_handle);
		CORIUM_ASSERT(m_joined && "NativeThread::joinThread failed to join created thread");
		CORIUM_ASSERT(m_threadRan.load(std::memory_order_acquire) && "Thread closure was not executed");
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

