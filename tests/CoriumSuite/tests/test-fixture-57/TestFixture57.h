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

class TestFixture57 final
	: public Hades::Runtime::IFixture<TestFixture57, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture57(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		CyclicBarrier barrier(2);
		CORIUM_ASSERT(!barrier.await(Chrono::until(Chrono::milliseconds(1))) && "No partner, so the timeout breaks it");
		CORIUM_ASSERT(barrier.isBroken());
		CORIUM_ASSERT(barrier.await() == CyclicBarrier::BROKEN && "A broken barrier must reject untimed awaits");
		CORIUM_ASSERT(!barrier.await(Chrono::until(Chrono::milliseconds(50))) && "A broken barrier must reject timed awaits");
		CORIUM_ASSERT(barrier.isBroken() && "Only reset() clears a break");

		barrier.reset();
		CORIUM_ASSERT(!barrier.isBroken());
		uint32_t fromThread = 0;
		std::thread partner([&] { fromThread = barrier.await(); });
		const uint32_t fromMain = barrier.await();
		partner.join();
		CORIUM_ASSERT(fromMain != CyclicBarrier::BROKEN && fromThread != CyclicBarrier::BROKEN && "A reset barrier must trip normally");
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
