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

class TestFixture58 final
	: public Hades::Runtime::IFixture<TestFixture58, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture58(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		constexpr uint32_t kParties = 4;
		for (int round = 0; round < 200; ++round) {
			CyclicBarrier barrier(kParties);
			std::atomic<uint32_t> seen{ 0 };
			std::vector<std::thread> parties;
			for (uint32_t i = 0; i < kParties; ++i)
				parties.emplace_back([&] {
					const uint32_t index = barrier.await();
					if (index < kParties) seen.fetch_or(1u << index);
				});
			for (auto& party : parties) party.join();
			CORIUM_ASSERT(seen.load() == (1u << kParties) - 1 && "Every arrival index 0..parties-1 must be handed out once");
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
