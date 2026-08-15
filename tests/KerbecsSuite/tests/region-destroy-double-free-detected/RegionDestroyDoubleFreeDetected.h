#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <Violation.h>
#include <cstdint>
#include <memory>
#include <vector>

class RegionDestroyDoubleFreeDetected final
	: public Hades::Runtime::IFixture<RegionDestroyDoubleFreeDetected, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionDestroyDoubleFreeDetected(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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

		m_Handle = m_Region->allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		m_ConstructSuccess = m_Region->construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

		m_DestroyFirstSuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroyFirstSuccess && "First destroy failed");

		m_DestroySecondSuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(!m_DestroySecondSuccess && "Second destroy call should have failed");

		m_ViolationPushed = Kerbecs::popViolation(m_Violation);
		KERBECS_ASSERT(m_ViolationPushed && "Expected a violation to be pushed on double free");

		m_ExpectedKind = (m_Violation.m_Kind == Kerbecs::ViolationKind::DoubleFree ||
		                  m_Violation.m_Kind == Kerbecs::ViolationKind::RetiredBoundaryViolation);
		KERBECS_ASSERT(m_ExpectedKind && "Expected ViolationKind to be DoubleFree or RetiredBoundaryViolation");
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
	bool m_DestroyFirstSuccess{ false };
	bool m_DestroySecondSuccess{ false };
	Kerbecs::Violation m_Violation{};
	bool m_ViolationPushed{ false };
	bool m_ExpectedKind{ false };
};


