#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformThread.h>
#include <PlatformChrono.h>
#include <atomic>

class TestFixture7 final
	: public Hades::Runtime::IFixture<TestFixture7, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture7(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		Thread::ParkHandle timed{ 0 };
		const auto start = Chrono::MonotonicClock::now();
		Thread::PlatformThread::waitOnAddressFor(timed, 50);
		const int64_t waitedMs = (Chrono::MonotonicClock::now() - start).m_Nanoseconds / 1'000'000;
		PLATFORM_CHECK(waitedMs >= 40 && "With no wake the timed wait runs out its timeout");

		static Thread::ParkHandle s_Permit{ 0 };
		static std::atomic<bool> s_Woken{ false };
		s_Permit.m_ParkingPermit.store(0);
		s_Woken.store(false);

		Thread::ThreadLaunchDesc launch{};
		launch.launchFunction([](void*) {
			Thread::PlatformThread::waitOnAddress(s_Permit);
			s_Woken.store(true);
		});
		Thread::ThreadAttrDesc attr{};
		Thread::initAttrDesc(attr);
		Thread::affinityMask(attr, static_cast<Thread::ProcessorIdx>(Environment::PlatformProbe::getCpuInfo().m_NumaNodeMasks[0]));
		const Thread::ThreadHandle handle = Thread::PlatformThread::createThread(launch, attr);
		Sleep(20);
		PLATFORM_CHECK(!s_Woken.load() && "The waiter parks while the permit is 0");
		Thread::PlatformThread::wakeOnAddress(s_Permit);
		PLATFORM_CHECK(Thread::PlatformThread::joinThread(handle));
		PLATFORM_CHECK(s_Woken.load() && s_Permit.m_ParkingPermit.load() == 1u && "wake sets the permit and releases the waiter");
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
