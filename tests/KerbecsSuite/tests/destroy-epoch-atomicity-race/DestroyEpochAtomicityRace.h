#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <atomic>
#include <cstdint>
#include <optional>
#include <thread>
#include <vector>

// Layout-compatible inspector for Kerbecs::Quarantine::QuarantineQueue
struct QuarantineQueueInspector {
	Kerbecs::Quarantine::QuarantineEntry* m_Slots;
	size_t m_Capacity;
	std::atomic<size_t> m_Head;
	std::atomic<size_t> m_Tail;
	Kerbecs::Tracing::Internal::SpinLock m_FlushLock;
	Kerbecs::KerbecsStats* m_Stats;
};

class DestroyEpochAtomicityRace final
	: public Hades::Runtime::IFixture<DestroyEpochAtomicityRace, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit DestroyEpochAtomicityRace(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_RuntimeInitSuccess = Kerbecs::Runtime::instance().init();

		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);

		m_Region.emplace(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);

		m_RegionInitSuccess = m_Region->initialized();
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_RuntimeInitSuccess && "KerbecsRuntime init failed");
		KERBECS_ASSERT(m_RegionInitSuccess && "Region initialization failed");

		auto& runtime = Kerbecs::Runtime::instance();
		m_StopFlag.store(false, std::memory_order_relaxed);

		std::thread epochAdvancer([&]() {
			while (!m_StopFlag.load(std::memory_order_relaxed)) {
				runtime.m_Epoch.fetch_add(1, std::memory_order_acq_rel);
				std::this_thread::yield();
			}
		});

		m_Inspector = reinterpret_cast<const QuarantineQueueInspector*>(&Kerbecs::Runtime::quarantine());

		for (int i = 0; i < m_Iterations; ++i) {
			auto handle = m_Region->allocate<int>();
			if (!static_cast<bool>(handle)) {
				break;
			}

			m_Constructed = m_Region->construct(handle, i);
			KERBECS_ASSERT(m_Constructed && "Construct failed");

			m_PreEpoch = runtime.m_Epoch.load(std::memory_order_acquire);
			m_TailBefore = m_Inspector->m_Tail.load(std::memory_order_acquire);

			m_Destroyed = m_Region->destroy(handle);
			KERBECS_ASSERT(m_Destroyed && "Destroy failed");

			m_PostEpoch = runtime.m_Epoch.load(std::memory_order_acquire);

			m_TailAfter = m_Inspector->m_Tail.load(std::memory_order_acquire);
			if (m_TailAfter > m_TailBefore && m_Inspector->m_Slots != nullptr && m_Inspector->m_Capacity > 0) {
				size_t slotIdx = (m_TailAfter - 1) & (m_Inspector->m_Capacity - 1);
				uint64_t recordedEpoch = m_Inspector->m_Slots[slotIdx].m_Epoch;

				KERBECS_ASSERT(!(m_PreEpoch < m_PostEpoch && recordedEpoch > m_PreEpoch) &&
					"Quarantined block recorded epoch picked up concurrent epoch advance instead of pre-retiring epoch");
			}
		}

		m_StopFlag.store(true, std::memory_order_release);
		if (epochAdvancer.joinable()) {
			epochAdvancer.join();
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
	static constexpr size_t m_RegionSize = 64 * 1024;
	static constexpr size_t m_RegistryCapacity = 256;
	static constexpr int m_Iterations = 1000;

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::optional<Kerbecs::StaticRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	bool m_RuntimeInitSuccess{ false };
	bool m_RegionInitSuccess{ false };
	std::atomic<bool> m_StopFlag{ false };
	const QuarantineQueueInspector* m_Inspector{ nullptr };
	bool m_Constructed{ false };
	uint64_t m_PreEpoch{ 0 };
	size_t m_TailBefore{ 0 };
	bool m_Destroyed{ false };
	uint64_t m_PostEpoch{ 0 };
	size_t m_TailAfter{ 0 };
};

