#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <AllocationRegistry.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsStats.h>
#include <QuarantineQueue.h>
#include <cstdint>

class QuarantineEnqueueFlushEligibleRoundTrip final
	: public Hades::Runtime::IFixture<QuarantineEnqueueFlushEligibleRoundTrip, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit QuarantineEnqueueFlushEligibleRoundTrip(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = m_Queue.init(m_Capacity, &m_Stats);
		m_DummyBlock = 0x1234;
		m_PBlockBase = &m_DummyBlock;
		m_BlockSize = sizeof(m_DummyBlock);
		m_Epoch = 1;
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "QuarantineQueue init failed");

		m_ThunkCallCount = 0;

		m_EnqueueSuccess = m_Queue.enqueue(
			m_PBlockBase,
			m_BlockSize,
			m_Epoch,
			&m_ThunkCallCount,
			m_CustomThunk,
			&m_DummyRegistry
		);
		KERBECS_ASSERT(m_EnqueueSuccess && "QuarantineQueue enqueue failed");

		KERBECS_ASSERT(m_Queue.depth() == 1 && "Queue depth should be 1 after enqueue");
		KERBECS_ASSERT(!m_Queue.empty() && "Queue should not be empty after enqueue");

		m_FlushedCount = m_Queue.flushEligible(SIZE_MAX, true);
		KERBECS_ASSERT(m_FlushedCount == 1 && "flushEligible should have flushed 1 entry");

		KERBECS_ASSERT(m_ThunkCallCount == 1 && "Deallocation thunk should have fired exactly once");
		KERBECS_ASSERT(m_Queue.depth() == 0 && "Queue depth should be 0 after flush");
		KERBECS_ASSERT(m_Queue.empty() && "Queue should be empty after flush");

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
	static constexpr size_t m_Capacity{ 16 };
	Kerbecs::Quarantine::QuarantineQueue m_Queue;
	Kerbecs::KerbecsStats m_Stats{};
	bool m_InitSuccess{ false };
	int m_DummyBlock{ 0x1234 };
	void* m_PBlockBase{ nullptr };
	size_t m_BlockSize{ 0 };
	uint64_t m_Epoch{ 1 };
	Kerbecs::Tracing::AllocationRegistry m_DummyRegistry{ 16 };
	uint32_t m_ThunkCallCount{ 0 };
	bool m_EnqueueSuccess{ false };
	size_t m_FlushedCount{ 0 };

	static inline auto m_CustomThunk = [](void* p_Allocator, void* p_BlockBase, size_t v_BlockSize) {
		auto* countPtr = static_cast<uint32_t*>(p_Allocator);
		if (countPtr) {
			(*countPtr)++;
		}
		KERBECS_UNUSED(p_BlockBase);
		KERBECS_UNUSED(v_BlockSize);
	};
};

