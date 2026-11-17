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

class TestFixture56 final
	: public Hades::Runtime::IFixture<TestFixture56, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture56(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		for (int round = 0; round < 2000; ++round) {
			Phaser phaser(2);
			std::atomic<bool> started{ false };
			std::thread waiter([&] { started.store(true); CORIUM_UNUSED(phaser.awaitAdvance(0)); });
			while (!started.load()) std::this_thread::yield();
			phaser.forceTermination();
			waiter.join();
			CORIUM_ASSERT(phaser.isTerminated());
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
