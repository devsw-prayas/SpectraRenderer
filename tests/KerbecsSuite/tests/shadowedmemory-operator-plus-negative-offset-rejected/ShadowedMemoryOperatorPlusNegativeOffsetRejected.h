#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <Violation.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class ShadowedMemoryOperatorPlusNegativeOffsetRejected final
	: public Hades::Runtime::IFixture<ShadowedMemoryOperatorPlusNegativeOffsetRejected, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ShadowedMemoryOperatorPlusNegativeOffsetRejected(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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
		// Flush any stale violations on this thread before running test
		while (Kerbecs::popViolation(m_DrainViolation)) {}

		KERBECS_ASSERT(m_Region->initialized() && "Region initialization failed");

		// Allocate an array (count > 1)
		m_Handle = m_Region->allocate<uint64_t>(m_Count);
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		// Take handle + 1
		m_HandlePlus1 = m_Handle + 1;
		KERBECS_ASSERT(static_cast<bool>(m_HandlePlus1) && "handle + 1 should produce a valid handle");

		// Take (handle + 1) + (-2), resulting in net negative offset before buffer start
		m_HandleNeg = m_HandlePlus1 + (-2);

		// Assert that the result is empty (null payload)
		KERBECS_ASSERT(!static_cast<bool>(m_HandleNeg) && "handle + 1 + (-2) should be empty (rejected negative offset)");

		// Assert that a BufferOverflow violation was pushed
		m_Violation = {};
		m_Popped = Kerbecs::popViolation(m_Violation);

		KERBECS_ASSERT(m_Popped && "Expected a violation to be pushed on negative offset in operator+");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::BufferOverflow && "Popped violation kind should be BufferOverflow");
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
	static constexpr size_t m_Count{ 4 };

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::unique_ptr<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	Kerbecs::Violation m_DrainViolation{};
	Kerbecs::ShadowedMemory<uint64_t> m_Handle{};
	Kerbecs::ShadowedMemory<uint64_t> m_HandlePlus1{};
	Kerbecs::ShadowedMemory<uint64_t> m_HandleNeg{};
	Kerbecs::Violation m_Violation{};
	bool m_Popped{ false };
};


