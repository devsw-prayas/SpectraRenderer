#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>
#include <memory>

class TestFixture28 final
	: public Hades::Runtime::IFixture<TestFixture28, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture28(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Phaser = std::make_unique<Corium::Runtime::Sync::Phaser>();
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_Phaser->getRegisteredParties() == 0);
		m_Phaser->register_();
		CORIUM_ASSERT(m_Phaser->getRegisteredParties() == 1);
		m_Phaser->register_();
		CORIUM_ASSERT(m_Phaser->getRegisteredParties() == 2);
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

