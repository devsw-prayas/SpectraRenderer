#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <Violation.h>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

class RegionOutOfBoundsAllocationRejected final
	: public Hades::Runtime::IFixture<RegionOutOfBoundsAllocationRejected, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionOutOfBoundsAllocationRejected(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Flush any stale violations on this thread before running test
		while (Kerbecs::popViolation(m_Drain)) {}

		m_BackingBuffer.assign(m_AllocatorCapacity, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_AllocatorCapacity);

		m_Region.emplace(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);

		m_RegionInitSuccess = m_Region->initialized();
		m_Popped = false;
		m_Violation = {};
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_RegionInitSuccess && "Region initialization failed");

		// Attempt to allocate count that exceeds small region capacity
		m_Handle = m_Region->allocate<uint64_t>(m_Count);

		// Assert handle is empty (null payload & invalid)
		KERBECS_ASSERT(!static_cast<bool>(m_Handle) && "Handle payload should be nullptr on out-of-bounds allocation");
		KERBECS_ASSERT(!static_cast<bool>(m_Handle) && "Handle should be invalid on out-of-bounds allocation");

		// Assert WildPointer violation was pushed
		m_Popped = Kerbecs::popViolation(m_Violation);

		KERBECS_ASSERT(m_Popped && "Expected a violation to be pushed on out-of-bounds allocation");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::WildPointer && "Popped violation kind should be WildPointer");
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
	static constexpr size_t m_RegionSize = 64;
	static constexpr size_t m_AllocatorCapacity = 2048;
	static constexpr size_t m_RegistryCapacity = 16;
	static constexpr size_t m_Count = 100;

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::optional<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	bool m_RegionInitSuccess{ false };
	Kerbecs::Violation m_Drain{};
	Kerbecs::ShadowedMemory<uint64_t> m_Handle{};
	Kerbecs::Violation m_Violation{};
	bool m_Popped{ false };
};
