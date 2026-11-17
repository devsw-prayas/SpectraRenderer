#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformAtomics.h>
#include <thread>
#include <vector>

class TestFixture13 final
	: public Hades::Runtime::IFixture<TestFixture13, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture13(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		using Order = Intrinsic::MemoryOrder;
		Atomic::AtomicValue32<uint32_t> a32{ 10u };
		PLATFORM_CHECK(a32.fetchAdd(5) == 10u && a32.load() == 15u);
		PLATFORM_CHECK(a32.increment() == 16u && a32.decrement() == 15u && "increment/decrement return the new value");
		PLATFORM_CHECK(a32.exchange(100u) == 15u && a32.load() == 100u);
		PLATFORM_CHECK(a32.fetchAnd(0x0Fu) == 100u && a32.load() == (100u & 0x0Fu));
		PLATFORM_CHECK(a32.fetchOr(0xF0u) == 4u && a32.load() == 0xF4u);

		uint32_t expected = 0xF4u;
		PLATFORM_CHECK(a32.compareExchange(&expected, 7u, Order::ACQ_REL, Order::RELAXED) == 0xF4u && a32.load() == 7u);
		expected = 99u;
		PLATFORM_CHECK(a32.compareExchange(&expected, 8u, Order::ACQ_REL, Order::RELAXED) == 7u && a32.load() == 7u);
		PLATFORM_CHECK(expected == 7u && "A failed compareExchange writes the observed value back");

		Atomic::AtomicValue64<uint64_t> a64{ 1ull << 40 };
		PLATFORM_CHECK(a64.fetchAdd(1) == (1ull << 40) && a64.load() == (1ull << 40) + 1);
		PLATFORM_CHECK(a64.exchange(3ull << 50) == (1ull << 40) + 1 && a64.load() == (3ull << 50));

		constexpr int kThreads = 8, kAdds = 100000;
		Atomic::AtomicValue64<uint64_t> shared{ 0ull };
		std::vector<std::thread> threads;
		for (int t = 0; t < kThreads; ++t)
			threads.emplace_back([&] { for (int i = 0; i < kAdds; ++i) shared.fetchAdd(1, Order::RELAXED); });
		for (auto& thread : threads) thread.join();
		PLATFORM_CHECK(shared.load() == uint64_t{ kThreads } * kAdds && "Concurrent fetchAdd loses nothing");
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
