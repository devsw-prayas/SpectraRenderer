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
#include <cstring>

class GeneralAllocatorAllocateDeallocateRoundTrip final
	: public Hades::Runtime::IFixture<GeneralAllocatorAllocateDeallocateRoundTrip, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0]) m_pAlloc{ nullptr };
	static constexpr size_t s_allocSize = 128;
	static constexpr uint8_t s_sentinel = 0xAB;
	void* m_ptr1{ nullptr };
	uint8_t* m_bytes{ nullptr };
	void* m_ptr2{ nullptr };

public:
	explicit GeneralAllocatorAllocateDeallocateRoundTrip(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];
		m_ptr1 = nullptr;
		m_bytes = nullptr;
		m_ptr2 = nullptr;
	}

	void executeImpl() noexcept {
		using namespace Corium::Memory::Internal;

		m_ptr1 = m_pAlloc->allocate(s_allocSize);
		CORIUM_ASSERT(m_ptr1 != nullptr);

		// Write sentinel pattern through the pointer and verify read-back
		std::memset(m_ptr1, s_sentinel, s_allocSize);

		m_bytes = static_cast<uint8_t*>(m_ptr1);
		for (size_t i = 0; i < s_allocSize; ++i) {
			CORIUM_ASSERT(m_bytes[i] == s_sentinel);
		}

		// Deallocate
		m_pAlloc->deallocate(m_ptr1, s_allocSize);

		// Allocate the same size again and assert it succeeds
		m_ptr2 = m_pAlloc->allocate(s_allocSize);
		CORIUM_ASSERT(m_ptr2 != nullptr);

		// Deallocate second allocation
		m_pAlloc->deallocate(m_ptr2, s_allocSize);
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

