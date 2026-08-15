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

class QuarantineSaturationTriggersFailFast final
	: public Hades::Runtime::IFixture<QuarantineSaturationTriggersFailFast, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit QuarantineSaturationTriggersFailFast(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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

		m_OverflowOk = m_Queue.enqueue(
			&m_OverflowBlock,
			sizeof(int),
			m_Epoch,
			this,
			m_DummyThunk,
			&m_DummyRegistry
		);
		KERBECS_ASSERT(!m_OverflowOk && "Enqueue beyond capacity should fail when queue is saturated");
		KERBECS_ASSERT(m_Queue.full() && "Queue should remain full after rejected over-capacity enqueue");
		KERBECS_ASSERT(m_Queue.depth() == m_Capacity && "Queue depth should remain at capacity after rejected over-capacity enqueue");

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
	bool m_OverflowOk{ false };

	static inline auto m_DummyThunk = [](void* p_Allocator, void* p_BlockBase, size_t v_BlockSize) {
		KERBECS_UNUSED(p_Allocator);
		KERBECS_UNUSED(p_BlockBase);
		KERBECS_UNUSED(v_BlockSize);
	};
};

