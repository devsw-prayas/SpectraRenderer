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
#include <limits>
#include <optional>
#include <vector>

class RegionAllocateSizeOverflowRejected final
	: public Hades::Runtime::IFixture<RegionAllocateSizeOverflowRejected, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionAllocateSizeOverflowRejected(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Flush any stale violations on this thread before running test
		while (Kerbecs::popViolation(m_Drain)) {}

		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);

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

		// Attempt allocation with v_Count > SIZE_MAX / sizeof(uint64_t)
		m_Handle = m_Region->allocate<uint64_t>(m_HugeCount);

		// Assert handle is empty (null payload)
		KERBECS_ASSERT(!static_cast<bool>(m_Handle) && "Handle payload should be nullptr on size overflow");
		KERBECS_ASSERT(!static_cast<bool>(m_Handle) && "Handle should be invalid on size overflow");

		// Assert SizeOverflow violation was pushed
		m_Popped = Kerbecs::popViolation(m_Violation);

		KERBECS_ASSERT(m_Popped && "Expected a violation to be pushed on size overflow");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::SizeOverflow && "Popped violation kind should be SizeOverflow");
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
	static constexpr size_t m_RegionSize = 4096;
	static constexpr size_t m_RegistryCapacity = 64;
	static constexpr size_t m_HugeCount = (std::numeric_limits<size_t>::max() / sizeof(uint64_t)) + 1;

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::optional<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	bool m_RegionInitSuccess{ false };
	Kerbecs::Violation m_Drain{};
	Kerbecs::ShadowedMemory<uint64_t> m_Handle{};
	Kerbecs::Violation m_Violation{};
	bool m_Popped{ false };
};

