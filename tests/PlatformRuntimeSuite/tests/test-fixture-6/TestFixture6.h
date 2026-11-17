#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformThread.h>
#include <atomic>

class TestFixture6 final
	: public Hades::Runtime::IFixture<TestFixture6, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture6(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		static std::atomic<uint64_t> s_Spins{ 0 };
		static std::atomic<bool> s_Stop{ false };
		s_Spins.store(0);
		s_Stop.store(false);

		Thread::ThreadLaunchDesc launch{};
		launch.launchFunction([](void*) {
			while (!s_Stop.load()) s_Spins.fetch_add(1);
		});
		Thread::ThreadAttrDesc attr{};
		Thread::initAttrDesc(attr);
		Thread::affinityMask(attr, static_cast<Thread::ProcessorIdx>(Environment::PlatformProbe::getCpuInfo().m_NumaNodeMasks[0]));
		const Thread::ThreadHandle handle = Thread::PlatformThread::createThread(launch, attr);
		while (s_Spins.load() == 0) Sleep(1);

		PLATFORM_CHECK(Thread::PlatformThread::setPriority(handle, Thread::Priority::PRIORITY_BELOW_NORMAL));
		PLATFORM_CHECK(Thread::PlatformThread::getPriority(handle) == Thread::Priority::PRIORITY_BELOW_NORMAL);
		PLATFORM_CHECK(Thread::PlatformThread::setPriority(handle, Thread::Priority::PRIORITY_NORMAL));

		PLATFORM_CHECK(Thread::PlatformThread::suspendThread(handle));
		Sleep(5);
		const uint64_t frozen = s_Spins.load();
		Sleep(20);
		PLATFORM_CHECK(s_Spins.load() == frozen && "A suspended thread makes no progress");
		PLATFORM_CHECK(Thread::PlatformThread::resumeThread(handle));
		while (s_Spins.load() == frozen) Sleep(1);

		s_Stop.store(true);
		PLATFORM_CHECK(Thread::PlatformThread::joinThread(handle));
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
