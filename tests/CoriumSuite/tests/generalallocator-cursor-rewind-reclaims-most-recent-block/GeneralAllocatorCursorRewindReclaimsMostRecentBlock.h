#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumEnvironment.h>
#include <CoriumMemoryHandler.h>
#include <CoriumRuntime.h>
#include <EngineAllocators.h>
#include <cstdint>

class GeneralAllocatorCursorRewindReclaimsMostRecentBlock final
	: public Hades::Runtime::IFixture<GeneralAllocatorCursorRewindReclaimsMostRecentBlock, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0]) m_Alloc{ nullptr };
	void* m_PtrA{ nullptr };
	void* m_PtrB{ nullptr };
	void* m_PtrC{ nullptr };

public:
	explicit GeneralAllocatorCursorRewindReclaimsMostRecentBlock(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_Alloc = &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];
		m_PtrA = nullptr;
		m_PtrB = nullptr;
		m_PtrC = nullptr;
	}

	void executeImpl() noexcept {
		// Allocate block A then B
		m_PtrA = m_Alloc->allocate(128);
		CORIUM_ASSERT(m_PtrA != nullptr);

		m_PtrB = m_Alloc->allocate(256);
		CORIUM_ASSERT(m_PtrB != nullptr);

		// Deallocate B (most recent allocation)
		m_Alloc->deallocate(m_PtrB, 256);

		// Allocate C of the exact same size as B
		m_PtrC = m_Alloc->allocate(256);
		CORIUM_ASSERT(m_PtrC != nullptr);

		// Assert C's pointer equals B's original pointer due to cursor rewind
		CORIUM_ASSERT(m_PtrC == m_PtrB);

		// Clean up allocations
		m_Alloc->deallocate(m_PtrC, 256);
		m_Alloc->deallocate(m_PtrA, 128);
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

