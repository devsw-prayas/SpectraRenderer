#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>
#include <memory>

class PhaserForceTerminationSetsIsTerminated final
	: public Hades::Runtime::IFixture<PhaserForceTerminationSetsIsTerminated, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit PhaserForceTerminationSetsIsTerminated(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Phaser = std::make_unique<Corium::Runtime::Sync::Phaser>(1);
	}

	void executeImpl() noexcept {
		m_Phaser->forceTermination();
		CORIUM_ASSERT(m_Phaser->isTerminated() == true);
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
	std::unique_ptr<Corium::Runtime::Sync::Phaser> m_Phaser;
};

