#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <Violation.h>

class ViolationStackLIFOOrderingAndOverflowEviction final
	: public Hades::Runtime::IFixture<ViolationStackLIFOOrderingAndOverflowEviction, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ViolationStackLIFOOrderingAndOverflowEviction(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// First, clear any residual violations on the current thread's stack.
		while (Kerbecs::popViolation(m_Drain)) {}
		m_Popped = Kerbecs::Violation{};
		m_Success = false;
		m_Extra = Kerbecs::Violation{};
		m_PoppedExtra = false;
	}

	void executeImpl() noexcept {
		// Push 40 distinct violations on this thread.
		// Note: Violation.m_Address stores the index (1..40) encoded as a pointer.
		for (size_t i = 1; i <= 40; ++i) {
			const Kerbecs::Violation v = Kerbecs::makeViolation(
				Kerbecs::ViolationKind::DoubleFree,
				reinterpret_cast<void*>(i),
				nullptr,
				0
			);
			Kerbecs::Internal::pushViolation(v);
		}

		// Since capacity is 32, the first 8 pushed entries (indices 1 through 8) are evicted.
		// The surviving 32 entries (indices 9 through 40) should pop in LIFO order (40 down to 9).
		for (size_t expectedIndex = 40; expectedIndex >= 9; --expectedIndex) {
			m_Success = Kerbecs::popViolation(m_Popped);
			KERBECS_ASSERT(m_Success);
			KERBECS_ASSERT(m_Popped.m_Address == reinterpret_cast<void*>(expectedIndex));
		}

		// The stack should now be completely empty.
		m_PoppedExtra = Kerbecs::popViolation(m_Extra);
		KERBECS_ASSERT(!m_PoppedExtra);
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
	Kerbecs::Violation m_Drain{};
	Kerbecs::Violation m_Popped{};
	bool m_Success{ false };
	Kerbecs::Violation m_Extra{};
	bool m_PoppedExtra{ false };
};
