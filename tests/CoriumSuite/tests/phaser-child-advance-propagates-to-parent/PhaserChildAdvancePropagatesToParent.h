#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>
#include <memory>

class PhaserChildAdvancePropagatesToParent final
	: public Hades::Runtime::IFixture<PhaserChildAdvancePropagatesToParent, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit PhaserChildAdvancePropagatesToParent(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();

		m_Parent = std::make_unique<Corium::Runtime::Sync::Phaser>(1);
		m_Child = std::make_unique<Corium::Runtime::Sync::Phaser>(*m_Parent, 1);
	}

	void executeImpl() noexcept {
		const uint32_t initialParentPhase = m_Parent->getPhase();
		const uint32_t initialChildPhase = m_Child->getPhase();
		CORIUM_ASSERT(initialParentPhase == 0);
		CORIUM_ASSERT(initialChildPhase == 0);

		const uint32_t newChildPhase = m_Child->arriveAndAwaitAdvance();
		CORIUM_ASSERT(newChildPhase == 1);

		const uint32_t newParentPhase = m_Parent->getPhase();
		CORIUM_ASSERT(newParentPhase == 1);
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
	std::unique_ptr<Corium::Runtime::Sync::Phaser> m_Parent;
	std::unique_ptr<Corium::Runtime::Sync::Phaser> m_Child;
};

