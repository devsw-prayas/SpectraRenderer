#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>
#include <CoriumEnvironment.h>

class ChronoUDLsProduceExpectedNanosecondCounts final
	: public Hades::Runtime::IFixture<ChronoUDLsProduceExpectedNanosecondCounts, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ChronoUDLsProduceExpectedNanosecondCounts(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono;
		using namespace Corium::Core::Chrono::Literals;

		CORIUM_ASSERT(1_ns == Interval{ 1 });
		CORIUM_ASSERT(1_us == Interval{ 1'000 });
		CORIUM_ASSERT(1_ms == Interval{ 1'000'000 });
		CORIUM_ASSERT(1_s == Interval{ 1'000'000'000 });
		CORIUM_ASSERT(1_min == Interval{ 60'000'000'000 });

		CORIUM_ASSERT((1_ns).toNanoseconds() == 1LL);
		CORIUM_ASSERT((1_us).toNanoseconds() == 1'000LL);
		CORIUM_ASSERT((1_ms).toNanoseconds() == 1'000'000LL);
		CORIUM_ASSERT((1_s).toNanoseconds() == 1'000'000'000LL);
		CORIUM_ASSERT((1_min).toNanoseconds() == 60'000'000'000LL);
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

