#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class TestFixture51 final
	: public Hades::Runtime::IFixture<TestFixture51, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture51(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		constexpr int kPairs = 4, kRounds = 5000;
		std::vector<std::unique_ptr<Exchanger<uint64_t>>> exchangers;
		for (int p = 0; p < kPairs; ++p) exchangers.push_back(std::make_unique<Exchanger<uint64_t>>());
		std::atomic<bool> mismatch{ false };
		std::vector<std::thread> threads;
		for (int t = 0; t < kPairs * 2; ++t) {
			threads.emplace_back([&, t] {
				Exchanger<uint64_t>& exchanger = *exchangers[t / 2];
				const uint64_t partner = uint64_t((t ^ 1) + 1) << 32;
				for (int r = 0; r < kRounds; ++r) {
					const uint64_t mine = (uint64_t(t + 1) << 32) | uint64_t(r + 1);
					if (exchanger.exchange(uint64_t(mine)) != (partner | uint64_t(r + 1))) mismatch.store(true);
				}
			});
		}
		for (auto& thread : threads) thread.join();
		CORIUM_ASSERT(!mismatch.load() && "Each round must receive exactly the partner's value");
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
