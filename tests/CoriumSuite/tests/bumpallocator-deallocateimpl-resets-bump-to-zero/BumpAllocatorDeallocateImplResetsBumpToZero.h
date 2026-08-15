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

class BumpAllocatorDeallocateImplResetsBumpToZero final
	: public Hades::Runtime::IFixture<BumpAllocatorDeallocateImplResetsBumpToZero, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_TaskPayloadAllocator[0]) m_pAlloc{ nullptr };
	void* m_ptr1{ nullptr };
	void* m_arenaBase{ nullptr };
	void* m_ptr2{ nullptr };
	void* m_ptr3{ nullptr };

public:
	explicit BumpAllocatorDeallocateImplResetsBumpToZero(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_TaskPayloadAllocator[0];
		m_ptr1 = nullptr;
		m_arenaBase = nullptr;
		m_ptr2 = nullptr;
		m_ptr3 = nullptr;
	}

	void executeImpl() noexcept {
		// Initial allocation from fresh/reset arena: this should start at the arena base
		m_ptr1 = m_pAlloc->allocate(64, 8);
		CORIUM_ASSERT(m_ptr1 != nullptr);

		// Record the base pointer of the arena by using the first allocated address
		m_arenaBase = m_ptr1;

		// Allocate more bytes to advance the bump watermark further
		m_ptr2 = m_pAlloc->allocate(128, 8);
		CORIUM_ASSERT(m_ptr2 != nullptr);
		CORIUM_ASSERT(m_ptr2 > m_ptr1);

		// Reset/deallocate the whole arena
		m_pAlloc->reset();

		// Allocate again after reset; the new pointer must equal the original arena base
		m_ptr3 = m_pAlloc->allocate(64, 8);
		CORIUM_ASSERT(m_ptr3 != nullptr);
		CORIUM_ASSERT(m_ptr3 == m_arenaBase);

		// Cleanup arena state
		m_pAlloc->reset();
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

