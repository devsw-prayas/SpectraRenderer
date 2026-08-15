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

class RegionLeakAssertOnUnfreedAllocation final
	: public Hades::Runtime::IFixture<RegionLeakAssertOnUnfreedAllocation, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionLeakAssertOnUnfreedAllocation(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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
		KERBECS_ASSERT(m_Region->allocationRegistry().liveCount() == 0 && "Initial liveCount must be 0");

		m_Handle = m_Region->allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		m_ConstructSuccess = m_Region->construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

		// Assert that liveCount() != 0 when an allocation is un-freed / leaked.
		// Note: The Region destructor has KERBECS_ASSERT(m_AllocationRegistry.liveCount() == 0)
		// which would trigger an assertion if allowed to run with unfreed allocations.
		KERBECS_ASSERT(m_Region->allocationRegistry().liveCount() != 0 && "liveCount must be non-zero for unfreed allocation");

		// Clean up allocation before destruction so fixture execution completes without trapping in ~Region()
		m_DestroySuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Cleanup destroy failed");
		KERBECS_ASSERT(m_Region->allocationRegistry().liveCount() == 0 && "liveCount must return to 0 after destroy");
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

	Kerbecs::ShadowedMemory<int> m_Handle{};
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
};


