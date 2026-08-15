#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>

class ChronoUntilProducesFutureDeadline final
	: public Hades::Runtime::IFixture<ChronoUntilProducesFutureDeadline, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Chrono::Instant m_Deadline{};
	int64_t m_RemainMs{ 0 };

public:
	explicit ChronoUntilProducesFutureDeadline(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		using namespace Corium::Core::Chrono;
		using namespace Corium::Core::Chrono::Literals;

		// Create a deadline 10 ms in the future.
		m_Deadline = until(10_ms);
	}

	void executeImpl() noexcept {
		// Immediately after creation the deadline must not have expired yet.
		CORIUM_ASSERT(m_Deadline.isExpired() == false);

		// The remaining time must be positive and at most the requested 10 ms
		// (it can only have decreased from the moment until() was called).
		m_RemainMs = m_Deadline.remaining().toMilliseconds();
		CORIUM_ASSERT(m_RemainMs > 0);
		CORIUM_ASSERT(m_RemainMs <= 10);
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
