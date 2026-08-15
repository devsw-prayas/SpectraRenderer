#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumAllocator.h>
#include <EngineAllocators.h>
#include <CoriumMemoryHandler.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>

class BumpAllocatorAllocateImplAdvancesWatermark final
	: public Hades::Runtime::IFixture<BumpAllocatorAllocateImplAdvancesWatermark, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_ClosureAllocator[0]) m_pClosureAlloc{ nullptr };
	void* m_p1{ nullptr };
	void* m_p2{ nullptr };
	uintptr_t m_addr1{ 0 };
	uintptr_t m_addr2{ 0 };

public:
	explicit BumpAllocatorAllocateImplAdvancesWatermark(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pClosureAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_ClosureAllocator[0];
		m_p1 = nullptr;
		m_p2 = nullptr;
		m_addr1 = 0;
		m_addr2 = 0;
	}

	void executeImpl() noexcept {
		m_p1 = m_pClosureAlloc->allocate(64, 8);
		m_p2 = m_pClosureAlloc->allocate(64, 8);

		CORIUM_ASSERT(m_p1 != nullptr);
		CORIUM_ASSERT(m_p2 != nullptr);

		m_addr1 = reinterpret_cast<uintptr_t>(m_p1);
		m_addr2 = reinterpret_cast<uintptr_t>(m_p2);

		CORIUM_ASSERT(m_addr2 >= m_addr1 + 64);
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


