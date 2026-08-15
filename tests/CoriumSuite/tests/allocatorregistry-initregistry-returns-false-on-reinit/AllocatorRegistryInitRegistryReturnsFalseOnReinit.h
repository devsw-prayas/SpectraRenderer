#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumMemoryHandler.h>
#include <CoriumEnvironment.h>

class AllocatorRegistryInitRegistryReturnsFalseOnReinit final
	: public Hades::Runtime::IFixture<AllocatorRegistryInitRegistryReturnsFalseOnReinit, Hades::Runtime::NullDeviceAdapter> {
private:
	bool m_reinitResult{ false };

public:
	explicit AllocatorRegistryInitRegistryReturnsFalseOnReinit(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Ensure registry is initialized prior to re-init check
		Corium::Memory::Internal::AllocatorRegistry::initRegistry(1);
		m_reinitResult = false;
	}

	void executeImpl() noexcept {
		m_reinitResult = Corium::Memory::Internal::AllocatorRegistry::initRegistry(1);
		CORIUM_ASSERT(!m_reinitResult);
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


