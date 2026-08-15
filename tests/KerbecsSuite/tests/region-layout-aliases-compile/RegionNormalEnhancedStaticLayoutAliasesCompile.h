#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <MemoryLayouts.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <cstdint>
#include <vector>

class RegionNormalEnhancedStaticLayoutAliasesCompile final
	: public Hades::Runtime::IFixture<RegionNormalEnhancedStaticLayoutAliasesCompile, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionNormalEnhancedStaticLayoutAliasesCompile(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BackingBuffer.assign(m_RegionSize, 0);
	}

	void executeImpl() noexcept {
		testAlias<Kerbecs::NormalRegion>();
		testAlias<Kerbecs::NormalRegionStrict>();
		testAlias<Kerbecs::EnhancedRegion>();
		testAlias<Kerbecs::EnhancedRegionStrict>();
		testAlias<Kerbecs::StaticRegion>();
		testAlias<Kerbecs::StaticRegionStrict>();
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

	Kerbecs::ShadowedMemory<int> m_Handle{};
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
	Kerbecs::Shadow::Utils::MemoryState m_ShadowState{};

	template<template<typename> class RegionAlias>
	void testAlias() noexcept {
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);

		RegionAlias<Kerbecs::Allocators::StaticAllocator> region(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);

		KERBECS_ASSERT(region.initialized() && "Region initialization failed");

		m_Handle = region.allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		m_ConstructSuccess = region.construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

		KERBECS_ASSERT(*m_Handle == 42 && "Dereferenced handle value does not match constructed value");

		m_DestroySuccess = region.destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");

		m_ShadowState = Kerbecs::shadowStateOf(m_Handle, sizeof(int));
		KERBECS_ASSERT(m_ShadowState == Kerbecs::Shadow::Utils::MemoryState::DESTROYED && "Shadow state is not DESTROYED after destroy");
	}
};


