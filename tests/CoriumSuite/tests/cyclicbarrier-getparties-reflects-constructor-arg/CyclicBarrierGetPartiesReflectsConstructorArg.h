#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>

class CyclicBarrierGetPartiesReflectsConstructorArg final
	: public Hades::Runtime::IFixture<CyclicBarrierGetPartiesReflectsConstructorArg, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CyclicBarrier m_barrier{ 4 };

public:
	explicit CyclicBarrierGetPartiesReflectsConstructorArg(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_barrier.getParties() == 4);
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

