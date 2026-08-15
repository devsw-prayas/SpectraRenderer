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

class GeneralAllocatorAllocateHonorsRequestedAlignment final
	: public Hades::Runtime::IFixture<GeneralAllocatorAllocateHonorsRequestedAlignment, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0]) m_pAlloc{ nullptr };
	void* m_ptr1{ nullptr };
	void* m_ptr2{ nullptr };

public:
	explicit GeneralAllocatorAllocateHonorsRequestedAlignment(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];
		m_ptr1 = nullptr;
		m_ptr2 = nullptr;
	}

	void executeImpl() noexcept {
		using namespace Corium::Memory::Internal;

		m_ptr1 = m_pAlloc->allocate(64, 64);
		CORIUM_ASSERT(m_ptr1 != nullptr);
		CORIUM_ASSERT(reinterpret_cast<uintptr_t>(m_ptr1) % 64 == 0);

		m_ptr2 = m_pAlloc->allocate(128, 128);
		CORIUM_ASSERT(m_ptr2 != nullptr);
		CORIUM_ASSERT(reinterpret_cast<uintptr_t>(m_ptr2) % 128 == 0);

		m_pAlloc->deallocate(m_ptr2, 128);
		m_pAlloc->deallocate(m_ptr1, 64);
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

