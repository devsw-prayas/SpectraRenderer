#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <chrono>
#include <future>
#include <thread>

class TeardownShadowzoneJoinsDrainThreadCleanly final
	: public Hades::Runtime::IFixture<TeardownShadowzoneJoinsDrainThreadCleanly, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TeardownShadowzoneJoinsDrainThreadCleanly(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Initialize the shadowzone runtime and drain thread
		m_InitSuccess = Kerbecs::Runtime::initShadowzone();

		// Let the drain thread run for a short duration
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "initShadowzone failed during test setup");

		// Execute teardown in a separate thread so we can enforce a strict timeout against hangs/deadlocks
		m_TeardownFuture = std::async(std::launch::async, []() noexcept {
			return Kerbecs::Runtime::teardownShadowzone();
		});

		// Assert that teardownShadowzone completed within 5 seconds
		m_Status = m_TeardownFuture.wait_for(std::chrono::seconds(5));
		KERBECS_ASSERT(m_Status == std::future_status::ready && "teardownShadowzone timed out or deadlocked joining drain thread");

		m_TeardownSuccess = m_TeardownFuture.get();
		KERBECS_ASSERT(m_TeardownSuccess && "teardownShadowzone returned false");
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
	bool m_InitSuccess{ false };
	std::future<bool> m_TeardownFuture{};
	std::future_status m_Status{ std::future_status::timeout };
	bool m_TeardownSuccess{ false };
};

