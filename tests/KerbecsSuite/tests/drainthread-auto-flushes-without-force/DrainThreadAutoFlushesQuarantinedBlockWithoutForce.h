#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <QuarantineQueue.h>
#include <Region.h>
#include <chrono>
#include <cstdint>
#include <optional>
#include <ShadowedMemory.h>
#include <thread>
#include <vector>

class DrainThreadAutoFlushesQuarantinedBlockWithoutForce final
	: public Hades::Runtime::IFixture<DrainThreadAutoFlushesQuarantinedBlockWithoutForce, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit DrainThreadAutoFlushesQuarantinedBlockWithoutForce(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = Kerbecs::Runtime::initShadowzone();
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
		KERBECS_ASSERT(m_InitSuccess && "KerbecsRuntime initialization failed");
		KERBECS_ASSERT(m_Region->initialized() && "Region initialization failed");

		m_Handle = m_Region->template allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		m_ConstructSuccess = m_Region->construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

		m_DestroySuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");

		KERBECS_ASSERT(Kerbecs::Runtime::quarantine().depth() == 1 && "Quarantine depth should be 1 right after destroy");

		// Sleep past 2.5x the drain interval (~125ms > 100ms) to allow the drain thread to automatically advance epoch and flush eligible entries without force
		std::this_thread::sleep_for(Kerbecs::Runtime::DRAIN_INTERVAL * 2 + std::chrono::milliseconds(25));

		KERBECS_ASSERT(Kerbecs::Runtime::quarantine().depth() == 0 && "Quarantine depth should automatically drop to 0 without explicit flushEligible or force calls");

		m_Region.reset();

		m_TeardownSuccess = Kerbecs::Runtime::teardownShadowzone();
		KERBECS_ASSERT(m_TeardownSuccess && "KerbecsRuntime teardown failed");
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
	bool m_InitSuccess{ false };
	Kerbecs::ShadowedMemory<int> m_Handle;
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
	bool m_TeardownSuccess{ false };
};

