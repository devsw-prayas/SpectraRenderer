#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <cstdint>
#include <optional>
#include <vector>

class RegionAllocateZeroCountFails final
	: public Hades::Runtime::IFixture<RegionAllocateZeroCountFails, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionAllocateZeroCountFails(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_Region->initialized() && "Region initialization failed");

		m_InitialLiveCount = m_Region->allocationRegistry().liveCount();

		m_Handle = m_Region->allocate<int>(0);

		KERBECS_ASSERT(!static_cast<bool>(m_Handle) && "allocate<int>(0) should return empty handle (null payload)");
		KERBECS_ASSERT(m_Region->allocationRegistry().liveCount() == m_InitialLiveCount && "allocate<int>(0) changed liveCount");
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
	std::optional<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	size_t m_InitialLiveCount{ 0 };
	Kerbecs::ShadowedMemory<int> m_Handle;
};
