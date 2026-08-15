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
#include <Violation.h>
#include <cstdint>
#include <optional>
#include <vector>

class NodeForRejectsCrossRegionHandle final
	: public Hades::Runtime::IFixture<NodeForRejectsCrossRegionHandle, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit NodeForRejectsCrossRegionHandle(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BufferA.assign(m_RegionSize, 0);
		m_BufferB.assign(m_RegionSize, 0);
		m_AllocatorA.init(m_BufferA.data(), m_RegionSize);
		m_AllocatorB.init(m_BufferB.data(), m_RegionSize);
		m_RegionA.emplace(
			m_AllocatorA,
			m_RegionSize,
			m_RegistryCapacity,
			m_BufferA.data()
		);
		m_RegionB.emplace(
			m_AllocatorB,
			m_RegionSize,
			m_RegistryCapacity,
			m_BufferB.data()
		);
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_RegionA->initialized() && "Region A initialization failed");
		KERBECS_ASSERT(m_RegionB->initialized() && "Region B initialization failed");

		m_HandleA = m_RegionA->allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_HandleA) && "Allocation in Region A returned null payload");

		m_ConstructOnA = m_RegionA->construct(m_HandleA, 100);
		KERBECS_ASSERT(m_ConstructOnA && "Construct on Region A failed");
		KERBECS_ASSERT(*m_HandleA == 100 && "Dereferenced handle A value does not match");

		// Flush any leftover violations on thread
		while (Kerbecs::popViolation(m_DummyViolation)) {}

		// Attempt to call regionB.construct using handleFromA
		m_ConstructOnBWithA = m_RegionB->construct(m_HandleA, 200);
		KERBECS_ASSERT(!m_ConstructOnBWithA && "regionB.construct accepted handle from regionA");

		m_PoppedConstructViolation = Kerbecs::popViolation(m_VConstruct);
		KERBECS_ASSERT(m_PoppedConstructViolation && "Expected violation when calling construct with cross-region handle");
		KERBECS_ASSERT((m_VConstruct.m_Kind == Kerbecs::ViolationKind::WildPointer || m_VConstruct.m_Kind == Kerbecs::ViolationKind::DoubleFree)
			&& "Violation kind for construct with cross-region handle was neither WildPointer nor DoubleFree");

		// Attempt to call regionB.destroy using handleFromA
		m_DestroyOnBWithA = m_RegionB->destroy(m_HandleA);
		KERBECS_ASSERT(!m_DestroyOnBWithA && "regionB.destroy accepted handle from regionA");

		m_PoppedDestroyViolation = Kerbecs::popViolation(m_VDestroy);
		KERBECS_ASSERT(m_PoppedDestroyViolation && "Expected violation when calling destroy with cross-region handle");
		KERBECS_ASSERT((m_VDestroy.m_Kind == Kerbecs::ViolationKind::WildPointer || m_VDestroy.m_Kind == Kerbecs::ViolationKind::DoubleFree)
			&& "Violation kind for destroy with cross-region handle was neither WildPointer nor DoubleFree");

		// Assert regionA's original node / allocation is untouched
		KERBECS_ASSERT(*m_HandleA == 100 && "Original allocation in regionA was modified by cross-region operation");
		m_ShadowStateA = Kerbecs::shadowStateOf(m_HandleA, sizeof(int));
		KERBECS_ASSERT(m_ShadowStateA == Kerbecs::Shadow::Utils::MemoryState::CONSTRUCTED && "Shadow state of regionA allocation changed unexpectedly");

		// Cleanup regionA handle properly
		m_DestroyOnA = m_RegionA->destroy(m_HandleA);
		KERBECS_ASSERT(m_DestroyOnA && "Destroy on Region A failed");
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
	std::vector<uint8_t> m_BufferA;
	std::vector<uint8_t> m_BufferB;
	Kerbecs::Allocators::StaticAllocator m_AllocatorA;
	Kerbecs::Allocators::StaticAllocator m_AllocatorB;
	std::optional<Kerbecs::StaticRegion<Kerbecs::Allocators::StaticAllocator>> m_RegionA;
	std::optional<Kerbecs::StaticRegion<Kerbecs::Allocators::StaticAllocator>> m_RegionB;
	Kerbecs::ShadowedMemory<int> m_HandleA;
	bool m_ConstructOnA{ false };
	Kerbecs::Violation m_DummyViolation{};
	bool m_ConstructOnBWithA{ false };
	Kerbecs::Violation m_VConstruct{};
	bool m_PoppedConstructViolation{ false };
	bool m_DestroyOnBWithA{ false };
	Kerbecs::Violation m_VDestroy{};
	bool m_PoppedDestroyViolation{ false };
	Kerbecs::Shadow::Utils::MemoryState m_ShadowStateA{ Kerbecs::Shadow::Utils::MemoryState::UNINITIALIZED };
	bool m_DestroyOnA{ false };
};

