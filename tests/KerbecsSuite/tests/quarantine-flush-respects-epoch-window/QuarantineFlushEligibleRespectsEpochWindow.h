#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <AllocationRegistry.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <KerbecsStats.h>
#include <QuarantineQueue.h>
#include <cstdint>

class QuarantineFlushEligibleRespectsEpochWindow final
	: public Hades::Runtime::IFixture<QuarantineFlushEligibleRespectsEpochWindow, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit QuarantineFlushEligibleRespectsEpochWindow(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_InitSuccess = m_Queue.init(m_Capacity, &m_Stats);
		m_DummyBlock = 0x5678;
		m_PBlockBase = &m_DummyBlock;
		m_BlockSize = sizeof(m_DummyBlock);
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_InitSuccess && "QuarantineQueue init failed");

		m_ThunkCallCount = 0;

		// Ensure global runtime epoch is initialized/reset to a known starting state if needed
		// or fetch current runtime epoch as baseline.
		m_InitialEpoch = Kerbecs::Runtime::instance().m_Epoch.load();

		m_EnqueueSuccess = m_Queue.enqueue(
			m_PBlockBase,
			m_BlockSize,
			m_InitialEpoch,
			&m_ThunkCallCount,
			m_CustomThunk,
			&m_DummyRegistry
		);
		KERBECS_ASSERT(m_EnqueueSuccess && "QuarantineQueue enqueue failed");

		KERBECS_ASSERT(m_Queue.depth() == 1 && "Queue depth should be 1 after enqueue");

		// 1. Immediately call flushEligible(SIZE_MAX, false) (not forced)
		//    Because current runtime epoch == initialEpoch (window delta < 2), entry is not released.
		m_FlushedCount = m_Queue.flushEligible(SIZE_MAX, false);
		KERBECS_ASSERT(m_FlushedCount == 0 && "flushEligible should not release entry immediately when epoch window has not elapsed");
		KERBECS_ASSERT(m_ThunkCallCount == 0 && "Deallocation thunk must not fire before epoch window elapses");
		KERBECS_ASSERT(m_Queue.depth() == 1 && "Entry should remain in quarantine queue");

		// Advance epoch once
		Kerbecs::Runtime::instance().m_Epoch.fetch_add(1);

		// 2. Call flushEligible after 1 epoch advance - still ineligible (delta is 1, window requires >= 2)
		m_FlushedCount = m_Queue.flushEligible(SIZE_MAX, false);
		KERBECS_ASSERT(m_FlushedCount == 0 && "flushEligible should not release entry after only 1 epoch advance");
		KERBECS_ASSERT(m_ThunkCallCount == 0 && "Deallocation thunk must not fire after 1 epoch advance");
		KERBECS_ASSERT(m_Queue.depth() == 1 && "Entry should still remain in quarantine queue");

		// Advance epoch a second time (delta is now 2)
		Kerbecs::Runtime::instance().m_Epoch.fetch_add(1);

		// 3. Call flushEligible after 2 epoch advances - entry should now be released
		m_FlushedCount = m_Queue.flushEligible(SIZE_MAX, false);
		KERBECS_ASSERT(m_FlushedCount == 1 && "flushEligible should release entry after 2 epoch advances");
		KERBECS_ASSERT(m_ThunkCallCount == 1 && "Deallocation thunk should have fired exactly once");
		KERBECS_ASSERT(m_Queue.depth() == 0 && "Queue depth should be 0 after successful flush");

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
	int m_DummyBlock{ 0x5678 };
	void* m_PBlockBase{ nullptr };
	size_t m_BlockSize{ 0 };
	Kerbecs::Tracing::AllocationRegistry m_DummyRegistry{ 16 };

	uint32_t m_ThunkCallCount{ 0 };
	uint64_t m_InitialEpoch{ 0 };
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

