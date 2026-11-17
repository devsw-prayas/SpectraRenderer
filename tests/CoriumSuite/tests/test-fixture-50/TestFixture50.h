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

class TestFixture50 final
	: public Hades::Runtime::IFixture<TestFixture50, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture50(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		constexpr int kRounds = 500, kWaiters = 4;
		for (int round = 0; round < kRounds; ++round) {
			Semaphore sem(0);
			std::atomic<int> started{ 0 };
			std::vector<std::thread> waiters;
			for (int i = 0; i < kWaiters; ++i)
				waiters.emplace_back([&] { started.fetch_add(1); sem.acquire(); });
			while (started.load() < kWaiters) std::this_thread::yield();
			for (int i = 0; i < kWaiters; ++i) sem.release();
			for (auto& waiter : waiters) waiter.join();
			CORIUM_ASSERT(sem.availablePermits() == 0 && "Every release must be consumed by a waiter");
		}
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
