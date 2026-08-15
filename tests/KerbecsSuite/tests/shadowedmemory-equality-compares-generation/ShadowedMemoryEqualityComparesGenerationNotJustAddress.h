#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <cstdint>
#include <memory>
#include <vector>

class ShadowedMemoryEqualityComparesGenerationNotJustAddress final
	: public Hades::Runtime::IFixture<ShadowedMemoryEqualityComparesGenerationNotJustAddress, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ShadowedMemoryEqualityComparesGenerationNotJustAddress(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);
		m_Region = std::make_unique<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>>(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_Region->initialized() && "Region initialization failed");

		// Allocate and construct initial object to get first handle
		m_OldHandle = m_Region->allocate<int>();
		KERBECS_ASSERT(m_OldHandle.operator int* () != nullptr && "First allocation failed");

		m_Construct1Success = m_Region->construct(m_OldHandle, 100);
		KERBECS_ASSERT(m_Construct1Success && "First construct failed");

		m_FirstPayloadPtr = static_cast<void*>(m_OldHandle.operator int* ());

		// Destroy the allocation
		m_DestroySuccess = m_Region->destroy(m_OldHandle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy oldHandle failed");

		// Reset/re-init the allocator so subsequent allocations reuse the same backing address offset
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);

		// Allocate until we get a handle reusing the exact same virtual address (m_PayloadPtr)
		m_NewHandle = {};
		m_FoundMatchingAddress = false;

		for (size_t i = 0; i < m_RegistryCapacity; ++i) {
			m_CurrentHandle = m_Region->allocate<int>();
			if (m_CurrentHandle.operator int* () == nullptr) {
				break;
			}

			if (static_cast<void*>(m_CurrentHandle.operator int* ()) == m_FirstPayloadPtr) {
				m_NewHandle = m_CurrentHandle;
				m_FoundMatchingAddress = true;
				break;
			}
		}

		KERBECS_ASSERT(m_FoundMatchingAddress && "Failed to obtain a handle reusing the same payload address");

		// Assert operator== and operator!= behavior
		// oldHandle and newHandle have equal m_PayloadPtr but different m_Generation.
		// operator== must evaluate to false, and operator!= to true.
		KERBECS_ASSERT(!(m_OldHandle == m_NewHandle) && "ShadowedMemory equality returned true for handles with different generations");
		KERBECS_ASSERT(m_OldHandle != m_NewHandle && "ShadowedMemory inequality returned false for handles with different generations");
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
	static constexpr size_t m_RegionSize{ 4096 };
	static constexpr size_t m_RegistryCapacity{ 64 };

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::unique_ptr<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	Kerbecs::ShadowedMemory<int> m_OldHandle{};
	bool m_Construct1Success{ false };
	void* m_FirstPayloadPtr{ nullptr };
	bool m_DestroySuccess{ false };
	Kerbecs::ShadowedMemory<int> m_NewHandle{};
	bool m_FoundMatchingAddress{ false };
	Kerbecs::ShadowedMemory<int> m_CurrentHandle{};
};


