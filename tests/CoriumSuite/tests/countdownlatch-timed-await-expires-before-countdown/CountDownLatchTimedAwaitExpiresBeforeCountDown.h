#pragma once

#include <chrono>

#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

class CountDownLatchTimedAwaitExpiresBeforeCountDown final
	: public Hades::Runtime::IFixture<CountDownLatchTimedAwaitExpiresBeforeCountDown, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CountDownLatch m_Latch{ 1 };
	bool m_Result{ false };
	std::chrono::steady_clock::time_point m_Start{};
	std::chrono::steady_clock::time_point m_End{};
	std::chrono::milliseconds m_Duration{ 0 };

public:
	explicit CountDownLatchTimedAwaitExpiresBeforeCountDown(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono;
		using namespace Corium::Core::Chrono::Literals;

		m_Start = std::chrono::steady_clock::now();
		m_Result = m_Latch.await(until(1_ms));
		m_End = std::chrono::steady_clock::now();

		CORIUM_ASSERT(!m_Result);

		m_Duration = std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start);
		CORIUM_ASSERT(m_Duration.count() < 5000);
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

