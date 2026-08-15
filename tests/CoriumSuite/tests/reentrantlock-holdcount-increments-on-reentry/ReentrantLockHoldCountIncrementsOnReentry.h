#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>
#include <memory>

class ReentrantLockHoldCountIncrementsOnReentry final
	: public Hades::Runtime::IFixture<ReentrantLockHoldCountIncrementsOnReentry, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ReentrantLockHoldCountIncrementsOnReentry(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Lock = std::make_unique<Corium::Runtime::Sync::ReentrantLock>();
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_Lock->getHoldCount() == 0);
		CORIUM_ASSERT(!m_Lock->isHeldByCurrentThread());

		m_Lock->lock();
		CORIUM_ASSERT(m_Lock->getHoldCount() == 1);
		CORIUM_ASSERT(m_Lock->isHeldByCurrentThread());

		m_Lock->lock();
		CORIUM_ASSERT(m_Lock->getHoldCount() == 2);
		CORIUM_ASSERT(m_Lock->isHeldByCurrentThread());

		m_Lock->unlock();
		CORIUM_ASSERT(m_Lock->getHoldCount() == 1);
		CORIUM_ASSERT(m_Lock->isHeldByCurrentThread());

		m_Lock->unlock();
		CORIUM_ASSERT(m_Lock->getHoldCount() == 0);
		CORIUM_ASSERT(!m_Lock->isHeldByCurrentThread());
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
	std::unique_ptr<Corium::Runtime::Sync::ReentrantLock> m_Lock;
};

