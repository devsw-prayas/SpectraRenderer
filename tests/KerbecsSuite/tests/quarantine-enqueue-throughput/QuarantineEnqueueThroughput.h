#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsStats.h>
#include <QuarantineQueue.h>
#include <AllocationRegistry.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

class QuarantineEnqueueThroughput final
	: public Hades::Runtime::IFixture<QuarantineEnqueueThroughput, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit QuarantineEnqueueThroughput(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = m_Queue.init(m_Capacity, &m_Stats);
		m_RegInit = m_DummyRegistry.init();
		const unsigned int hwThreads = std::thread::hardware_concurrency();
		m_NumProducers = (hwThreads > 1) ? std::min<size_t>(hwThreads, 8) : 4;
		m_ExpectedTotal = m_NumProducers * m_EnqueuesPerThread;
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "QuarantineQueue init failed");
		KERBECS_ASSERT(m_RegInit && "AllocationRegistry init failed");

		m_StopFlusher.store(false, std::memory_order_relaxed);
		m_StartProducers.store(false, std::memory_order_relaxed);
		m_TotalSuccessfulEnqueues.store(0, std::memory_order_relaxed);

		// Background flusher thread draining concurrently to maintain throughput
		m_Flusher = std::thread([&]() {
			while (!m_StopFlusher.load(std::memory_order_relaxed)) {
				m_Queue.flushEligible(SIZE_MAX, true);
				std::this_thread::yield();
			}
			m_Queue.flushEligible(SIZE_MAX, true);
		});

		m_Producers.clear();
		m_Producers.reserve(m_NumProducers);

		for (size_t t = 0; t < m_NumProducers; ++t) {
			m_Producers.emplace_back([&, t]() {
				while (!m_StartProducers.load(std::memory_order_acquire)) {
					std::this_thread::yield();
				}

				uint64_t localSuccess = 0;
				for (size_t i = 0; i < m_EnqueuesPerThread; ++i) {
					void* pBlock = reinterpret_cast<void*>(uintptr_t(0x1000 + (t * m_EnqueuesPerThread + i) * 8));
					const bool ok = m_Queue.enqueue(
						pBlock,
						64,
						1,
						this,
						m_DummyThunk,
						&m_DummyRegistry
					);
					if (ok) {
						localSuccess++;
					} else {
						std::this_thread::yield();
					}
				}
				m_TotalSuccessfulEnqueues.fetch_add(localSuccess, std::memory_order_relaxed);
			});
		}

		m_StartTime = std::chrono::high_resolution_clock::now();
		m_StartProducers.store(true, std::memory_order_release);

		for (auto& p : m_Producers) {
			p.join();
		}

		m_EndTime = std::chrono::high_resolution_clock::now();

		m_StopFlusher.store(true, std::memory_order_release);
		m_Flusher.join();

		m_DurationMs = m_EndTime - m_StartTime;

		m_TotalEnqueued = m_TotalSuccessfulEnqueues.load(std::memory_order_relaxed);

		KERBECS_ASSERT(m_TotalEnqueued == m_ExpectedTotal && "All producer enqueues should succeed");

		m_Seconds = m_DurationMs.count() / 1000.0;
		m_RateOpsPerSec = (m_Seconds > 0.0) ? (static_cast<double>(m_TotalEnqueued) / m_Seconds) : 0.0;
		KERBECS_UNUSED(m_RateOpsPerSec);

		m_DummyRegistry.shutdown();
		m_Queue.shutdown();
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
	static constexpr size_t m_Capacity{ 65536 };
	static constexpr size_t m_EnqueuesPerThread{ 25000 };
	Kerbecs::Quarantine::QuarantineQueue m_Queue;
	Kerbecs::KerbecsStats m_Stats{};
	Kerbecs::Tracing::AllocationRegistry m_DummyRegistry{ 256 };
	bool m_InitSuccess{ false };
	bool m_RegInit{ false };
	size_t m_NumProducers{ 0 };
	size_t m_ExpectedTotal{ 0 };

	std::atomic<bool> m_StopFlusher{ false };
	std::atomic<bool> m_StartProducers{ false };
	std::atomic<uint64_t> m_TotalSuccessfulEnqueues{ 0 };
	std::thread m_Flusher;
	std::vector<std::thread> m_Producers;
	std::chrono::high_resolution_clock::time_point m_StartTime;
	std::chrono::high_resolution_clock::time_point m_EndTime;
	std::chrono::duration<double, std::milli> m_DurationMs{ 0 };
	uint64_t m_TotalEnqueued{ 0 };
	double m_Seconds{ 0.0 };
	double m_RateOpsPerSec{ 0.0 };

	static inline auto m_DummyThunk = [](void* p_Allocator, void* p_BlockBase, size_t v_BlockSize) {
		KERBECS_UNUSED(p_Allocator);
		KERBECS_UNUSED(p_BlockBase);
		KERBECS_UNUSED(v_BlockSize);
	};
};
