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
#include <memory>

class PriorityFactoryCreateAndStartAppliesRequestedPriority final
	: public Hades::Runtime::IFixture<PriorityFactoryCreateAndStartAppliesRequestedPriority, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit PriorityFactoryCreateAndStartAppliesRequestedPriority(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();

		m_ThreadRan = std::make_unique<std::atomic<bool>>(false);
		m_Factory = std::make_unique<Corium::Core::Factory::PriorityFactory>(0, static_cast<Corium::Core::AffinityMask>(1), Corium::Core::ThreadPriority::HIGH);
	}

	void executeImpl() noexcept {
		Corium::Core::ThreadHandle handle = m_Factory->createAndStart(
			Corium::Core::createClosure<void()>([this]() {
				m_ThreadRan->store(true, std::memory_order_release);
			}),
			"TestPriorityThread"
		);

		bool joined = Corium::Core::NativeThread::joinThread(handle);
		CORIUM_ASSERT(joined && "NativeThread::joinThread failed to join PriorityFactory thread");
		CORIUM_ASSERT(m_ThreadRan->load(std::memory_order_acquire) && "PriorityFactory thread closure was not executed");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}

private:
	std::unique_ptr<std::atomic<bool>> m_ThreadRan;
	std::unique_ptr<Corium::Core::Factory::PriorityFactory> m_Factory;
};

