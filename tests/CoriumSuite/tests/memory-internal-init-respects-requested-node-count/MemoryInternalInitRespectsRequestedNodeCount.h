#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumAddrSpace.h>
#include <CoriumMemoryHandler.h>
#include <CoriumEnvironment.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>

class MemoryInternalInitRespectsRequestedNodeCount final
	: public Hades::Runtime::IFixture<MemoryInternalInitRespectsRequestedNodeCount, Hades::Runtime::NullDeviceAdapter> {
private:
	uint32_t m_DetectedNodeCount{ 0 };

public:
	explicit MemoryInternalInitRespectsRequestedNodeCount(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_DetectedNodeCount = Corium::Environment::EnvironmentProbe::getCpuInfo().m_NumaNodeCount;
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(Corium::Memory::Internal::AllocatorRegistry::s_NodeCount > 0);
		CORIUM_ASSERT(Corium::Memory::Internal::AllocatorRegistry::s_NodeCount <= Corium::Memory::Internal::MAX_NUMA_NODES);

		if (m_DetectedNodeCount > 0 && m_DetectedNodeCount <= Corium::Memory::Internal::MAX_NUMA_NODES) {
			CORIUM_ASSERT(Corium::Memory::Internal::AllocatorRegistry::s_NodeCount == m_DetectedNodeCount);
		}
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

