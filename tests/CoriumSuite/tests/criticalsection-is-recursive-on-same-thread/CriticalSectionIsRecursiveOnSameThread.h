#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>

class CriticalSectionIsRecursiveOnSameThread final
	: public Hades::Runtime::IFixture<CriticalSectionIsRecursiveOnSameThread, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CriticalSection m_CS;

public:
	explicit CriticalSectionIsRecursiveOnSameThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		m_CS.lock();
		m_CS.lock();
		m_CS.unlock();
		m_CS.unlock();
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

