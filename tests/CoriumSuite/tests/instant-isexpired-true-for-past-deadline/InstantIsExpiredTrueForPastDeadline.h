#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>
#include <CoriumEnvironment.h>

class InstantIsExpiredTrueForPastDeadline final
	: public Hades::Runtime::IFixture<InstantIsExpiredTrueForPastDeadline, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Chrono::Instant m_Deadline{};
	bool m_Expired{ false };

public:
	explicit InstantIsExpiredTrueForPastDeadline(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		using namespace Corium::Core::Chrono;
		using namespace Corium::Core::Chrono::Literals;
		m_Deadline = until(0_ns);
		m_Expired = false;
	}

	void executeImpl() noexcept {
		// Brief busy-wait to guarantee the monotonic clock has advanced past the
		// deadline.  Even without the wait, until(0_ns) should be expired
		// immediately, but a tiny spin removes any doubt.
		volatile int sink = 0;
		for (int i = 0; i < 1000; ++i) {
			sink = i;
		}

		m_Expired = m_Deadline.isExpired();
		CORIUM_ASSERT(m_Expired);
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
		m_Expired = false;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		// Hash the observed boolean — deterministic for a correct implementation.
		return static_cast<uint64_t>(m_Expired ? 1 : 0);
	}
};
