#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumMemoryHandler.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>

class ExchangerTwoPartyExchangeSwapsValues final
	: public Hades::Runtime::IFixture<ExchangerTwoPartyExchangeSwapsValues, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::Exchanger<int> m_exchanger{};
	int m_resultA{ 0 };
	int m_resultB{ 0 };
	Corium::Core::Factory::DefaultThreadFactory m_factory{};
	Corium::Core::ThreadHandle m_handleA{};
	Corium::Core::ThreadHandle m_handleB{};
	bool m_joinedA{ false };
	bool m_joinedB{ false };

public:
	explicit ExchangerTwoPartyExchangeSwapsValues(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_resultA = 0;
		m_resultB = 0;
		m_handleA = {};
		m_handleB = {};
		m_joinedA = false;
		m_joinedB = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;
		using namespace Corium::Runtime::Sync;

		m_handleA = m_factory.createAndStart(
			createClosure<void()>([this]() {
				m_resultA = m_exchanger.exchange(1);
			}),
			"ExchangerPartyA"
		);

		m_handleB = m_factory.createAndStart(
			createClosure<void()>([this]() {
				m_resultB = m_exchanger.exchange(2);
			}),
			"ExchangerPartyB"
		);

		m_joinedA = NativeThread::joinThread(m_handleA);
		m_joinedB = NativeThread::joinThread(m_handleB);

		CORIUM_ASSERT(m_joinedA && "Thread A failed to join");
		CORIUM_ASSERT(m_joinedB && "Thread B failed to join");
		CORIUM_ASSERT(m_resultA == 2 && "Thread A did not receive 2 from Thread B");
		CORIUM_ASSERT(m_resultB == 1 && "Thread B did not receive 1 from Thread A");
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

