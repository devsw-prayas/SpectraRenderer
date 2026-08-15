#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

#include <chrono>

class ConditionTimedAwaitExpiresWithoutSignal final
	: public Hades::Runtime::IFixture<ConditionTimedAwaitExpiresWithoutSignal, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::ReentrantLock m_Lock;
	Corium::Runtime::Sync::Condition m_Cv{ m_Lock };
	bool m_Result{ false };
	std::chrono::steady_clock::time_point m_Start{};
	std::chrono::steady_clock::time_point m_End{};
	int64_t m_DurationMs{ 0 };

public:
	explicit ConditionTimedAwaitExpiresWithoutSignal(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono;
		using namespace Corium::Core::Chrono::Literals;

		m_Lock.lock();
		m_Start = std::chrono::steady_clock::now();
		m_Result = m_Cv.await(until(1_ms));
		m_End = std::chrono::steady_clock::now();
		m_Lock.unlock();

		CORIUM_ASSERT(!m_Result && "Condition::await timed out without signal should return false");

		m_DurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start).count();
		CORIUM_ASSERT(m_DurationMs < 5000);
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

