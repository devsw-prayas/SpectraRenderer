#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <AllocationRegistry.h>
#include <KerbecsCompiler.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsStats.h>
#include <QuarantineQueue.h>
#include <cstdint>

#if defined(_MSC_VER)
// Saturation is fail-fast (KERBECS_TRAP then abort); catching the trap stops it before abort.
KERBECS_NOINLINE static bool triggerAndCatchSaturation(
	Kerbecs::Quarantine::QuarantineQueue* p_Queue, void* p_Block, uint64_t v_Epoch, void* p_Allocator,
	void (*p_Thunk)(void*, void*, size_t), Kerbecs::Tracing::AllocationRegistry* p_Registry) noexcept {
	__try {
		(void)p_Queue->enqueue(p_Block, sizeof(int), v_Epoch, p_Allocator, p_Thunk, p_Registry);
		return false;
	}
	__except (1) {
		return true;
	}
}
#endif

class TestFixture20 final
	: public Hades::Runtime::IFixture<TestFixture20, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture20(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = m_Queue.init(m_Capacity, &m_Stats);
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "QuarantineQueue init failed");

		for (size_t i = 0; i < m_Capacity; ++i) {
			m_DummyBlocks[i] = static_cast<int>(i);
			KERBECS_ASSERT(m_Queue.depth() == i && "Queue depth should match enqueued count");
			KERBECS_ASSERT(!m_Queue.full() && "Queue should not be full before capacity is reached");

			m_Ok = m_Queue.enqueue(
				&m_DummyBlocks[i],
				sizeof(int),
				m_Epoch,
				this,
				m_DummyThunk,
				&m_DummyRegistry
			);
			KERBECS_ASSERT(m_Ok && "Enqueue should succeed when queue is not saturated");
		}

		KERBECS_ASSERT(m_Queue.full() && "Queue should be full after capacity enqueues");
		KERBECS_ASSERT(m_Queue.depth() == m_Capacity && "Queue depth should equal capacity");

#if defined(_MSC_VER)
		m_OverflowTrapped = triggerAndCatchSaturation(&m_Queue, &m_OverflowBlock, m_Epoch, this, m_DummyThunk, &m_DummyRegistry);
		KERBECS_ASSERT(m_OverflowTrapped && "Enqueue beyond capacity should trap (fail-fast)");
#endif

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
	static constexpr size_t m_Capacity{ 4 };
	Kerbecs::Quarantine::QuarantineQueue m_Queue;
	Kerbecs::KerbecsStats m_Stats{};
	Kerbecs::Tracing::AllocationRegistry m_DummyRegistry{ 16 };
	bool m_InitSuccess{ false };
	uint64_t m_Epoch{ 1 };
	int m_DummyBlocks[m_Capacity]{};
	int m_OverflowBlock{ 999 };
	bool m_Ok{ false };
	bool m_OverflowTrapped{ false };

	static inline auto m_DummyThunk = [](void* p_Allocator, void* p_BlockBase, size_t v_BlockSize) {
		KERBECS_UNUSED(p_Allocator);
		KERBECS_UNUSED(p_BlockBase);
		KERBECS_UNUSED(v_BlockSize);
	};
};

