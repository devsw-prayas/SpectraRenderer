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
#include <memory>
#include <thread>
#include <vector>

class RegionDestroyWrongThreadStrictPolicy final
	: public Hades::Runtime::IFixture<RegionDestroyWrongThreadStrictPolicy, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit RegionDestroyWrongThreadStrictPolicy(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);
		m_Region = std::make_unique<Kerbecs::NormalRegionStrict<Kerbecs::Allocators::StaticAllocator>>(
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

		m_DestroySuccess = false;
		m_PoppedViolationOnB = false;
		m_Violation = {};

		std::thread threadB([&]() {
			m_DestroySuccess = m_Region->destroy(m_Handle);
			m_PoppedViolationOnB = Kerbecs::popViolation(m_Violation);
		});
		threadB.join();

		KERBECS_ASSERT(!m_DestroySuccess && "destroy on thread B should have failed under Strict policy");
		KERBECS_ASSERT(m_PoppedViolationOnB && "Thread B should have recorded a violation");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::ThreadOwnership && "Violation kind should be ThreadOwnership");

		// Clean up allocation on original thread so region destructor doesn't assert liveCount() == 0
		m_CleanupDestroy = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_CleanupDestroy && "Cleanup destroy on thread A failed");
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
	std::unique_ptr<Kerbecs::NormalRegionStrict<Kerbecs::Allocators::StaticAllocator>> m_Region;

	Kerbecs::ShadowedMemory<int> m_Handle{};
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
	bool m_PoppedViolationOnB{ false };
	Kerbecs::Violation m_Violation{};
	bool m_CleanupDestroy{ false };
};


