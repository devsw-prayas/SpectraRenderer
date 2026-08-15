#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <cstdint>
#include <optional>
#include <thread>
#include <vector>

class AllocateNodeExhaustsCapacityDespiteDeadSlots final
	: public Hades::Runtime::IFixture<AllocateNodeExhaustsCapacityDespiteDeadSlots, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit AllocateNodeExhaustsCapacityDespiteDeadSlots(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);

		m_Region.emplace(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);

		m_RegionInitSuccess = m_Region->initialized();

		// Initialize runtime shadowzone for quarantine / drain thread processing
		m_RuntimeInitSuccess = Kerbecs::Runtime::instance().init();
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_RegionInitSuccess && "Region initialization failed");
		KERBECS_ASSERT(m_RuntimeInitSuccess && "KerbecsRuntime init failed");

		// Perform 8 allocate/construct/destroy iterations
		for (size_t i = 0; i < m_RegistryCapacity; ++i) {
			auto handle = m_Region->allocate<int>();
			KERBECS_ASSERT(static_cast<bool>(handle) && "Allocation within capacity failed unexpectedly");

			m_ConstructSuccess = m_Region->construct(handle, static_cast<int>(i));
			KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

			m_DestroySuccess = m_Region->destroy(handle);
			KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");
		}

		// Allow drain thread / epoch progression time to retire quarantined blocks to Dead state
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		Kerbecs::Runtime::quarantine().flushEligible(SIZE_MAX, true);

		// Attempt (capacity + 1)th allocation
		m_OverflowHandle = m_Region->allocate<int>();

		// AllocationRegistry node pool does not recycle Dead slots, so capacity exhaustion occurs despite dead slots
		KERBECS_ASSERT(!static_cast<bool>(m_OverflowHandle) && "Expected allocation failure due to node pool exhaustion");

		Kerbecs::Runtime::teardownShadowzone();
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
	static constexpr size_t m_RegionSize = 65536;
	static constexpr size_t m_RegistryCapacity = 8;

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::optional<Kerbecs::StaticRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	bool m_RegionInitSuccess{ false };
	bool m_RuntimeInitSuccess{ false };
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
	Kerbecs::ShadowedMemory<int> m_OverflowHandle{};
};

