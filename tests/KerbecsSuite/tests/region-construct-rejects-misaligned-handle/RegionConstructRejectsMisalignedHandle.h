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
#include <cstdint>
#include <optional>
#include <vector>

class RegionConstructRejectsMisalignedHandle final
	: public Hades::Runtime::IFixture<RegionConstructRejectsMisalignedHandle, Hades::Runtime::NullDeviceAdapter> {
public:
	struct alignas(8) AlignedStruct {
		uint64_t data;
	};

	explicit RegionConstructRejectsMisalignedHandle(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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
		m_ConstructSuccess = false;
		m_Popped = false;
		m_Violation = {};
		m_DestroySuccess = false;
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_RegionInitSuccess && "Region initialization failed");

		// Allocate space for char array to shift offset byte-by-byte
		auto charHandle = m_Region->allocate<char>(m_AllocationCount);
		KERBECS_ASSERT(static_cast<bool>(charHandle) && "Allocation returned null handle payload");

		// Shift pointer by 1 byte to make it unaligned for AlignedStruct (alignof(AlignedStruct) == 8)
		auto misalignedHandle = charHandle + 1;
		auto* misalignedTypedHandle = reinterpret_cast<Kerbecs::ShadowedMemory<AlignedStruct>*>(&misalignedHandle);

		m_ConstructSuccess = m_Region->construct(*misalignedTypedHandle, AlignedStruct{ 123 });
		KERBECS_ASSERT(!m_ConstructSuccess && "Construct on misaligned handle should have failed");

		m_Popped = Kerbecs::popViolation(m_Violation);
		KERBECS_ASSERT(m_Popped && "Expected a violation to be pushed");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::AlignmentViolation && "Expected AlignmentViolation");

		// Clean up all allocated elements so m_LiveCount reaches 0 and ~Region() does not assert on unfreed allocations
		for (size_t i = 0; i < m_AllocationCount; ++i) {
			m_DestroySuccess = m_Region->destroy(charHandle + static_cast<ptrdiff_t>(i));
			KERBECS_ASSERT(m_DestroySuccess && "Cleanup destroy failed");
		}
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
	static constexpr size_t m_AllocationCount = 16;

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::optional<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	bool m_RegionInitSuccess{ false };
	Kerbecs::Violation m_Drain{};
	bool m_ConstructSuccess{ false };
	Kerbecs::Violation m_Violation{};
	bool m_Popped{ false };
	bool m_DestroySuccess{ false };
};
