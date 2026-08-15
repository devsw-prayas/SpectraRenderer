#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <chrono>
#include <cstdint>
#include <thread>

class DrainThreadAdvancesEpochAutomatically final
	: public Hades::Runtime::IFixture<DrainThreadAdvancesEpochAutomatically, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit DrainThreadAdvancesEpochAutomatically(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = Kerbecs::Runtime::instance().init();
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "Runtime initialization failed");

		m_InitialEpoch = Kerbecs::Runtime::instance().m_Epoch.load(std::memory_order_acquire);

		// Sleep ~5x the drain interval to allow the drain thread to advance the epoch
		std::this_thread::sleep_for(5 * Kerbecs::Runtime::DRAIN_INTERVAL);

		m_NewEpoch = Kerbecs::Runtime::instance().m_Epoch.load(std::memory_order_acquire);

		KERBECS_ASSERT(m_NewEpoch > m_InitialEpoch && "Drain thread failed to automatically advance epoch");

		Kerbecs::Runtime::teardownShadowzone();
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
	uint64_t m_InitialEpoch{ 0 };
	uint64_t m_NewEpoch{ 0 };
};

