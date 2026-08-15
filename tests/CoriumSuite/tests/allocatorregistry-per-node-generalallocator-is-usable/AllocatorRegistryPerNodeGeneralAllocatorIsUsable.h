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

class AllocatorRegistryPerNodeGeneralAllocatorIsUsable final
	: public Hades::Runtime::IFixture<AllocatorRegistryPerNodeGeneralAllocatorIsUsable, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0]) m_pAlloc{ nullptr };
	void* m_ptr{ nullptr };
	uint8_t* m_bytes{ nullptr };

public:
	explicit AllocatorRegistryPerNodeGeneralAllocatorIsUsable(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];
		m_ptr = nullptr;
		m_bytes = nullptr;
	}

	void executeImpl() noexcept {
		m_ptr = m_pAlloc->allocate(32);
		CORIUM_ASSERT(m_ptr != nullptr);

		// Verify pointer is writable by filling memory and reading back sentinel values
		std::memset(m_ptr, 0xAB, 32);
		m_bytes = static_cast<uint8_t*>(m_ptr);
		for (size_t i = 0; i < 32; ++i) {
			CORIUM_ASSERT(m_bytes[i] == 0xAB);
		}

		m_pAlloc->deallocate(m_ptr, 32);
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


