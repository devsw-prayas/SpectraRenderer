#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformThread.h>
#include <atomic>

class TestFixture5 final
	: public Hades::Runtime::IFixture<TestFixture5, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture5(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		static std::atomic<int> s_Ran{ 0 };
		static std::atomic<bool> s_Release{ false };
		s_Ran.store(0);
		s_Release.store(false);

		Thread::ThreadLaunchDesc launch{};
		launch.launchFunction([](void* p_Ctx) {
			static_cast<std::atomic<int>*>(p_Ctx)->fetch_add(1);
			while (!s_Release.load()) Sleep(1);
		}, &s_Ran);
		Thread::ThreadAttrDesc attr{};
		Thread::initAttrDesc(attr);
		Thread::affinityMask(attr, static_cast<Thread::ProcessorIdx>(Environment::PlatformProbe::getCpuInfo().m_NumaNodeMasks[0]));
		PLATFORM_CHECK(Thread::validateAttrDesc(attr));

		const Thread::ThreadHandle handle = Thread::PlatformThread::createThread(launch, attr);
		while (s_Ran.load() == 0) Sleep(1);
		PLATFORM_CHECK(Thread::PlatformThread::isAlive(handle) && "A running thread reads alive");

		s_Release.store(true);
		PLATFORM_CHECK(Thread::PlatformThread::joinThread(handle));
		PLATFORM_CHECK(s_Ran.load() == 1 && "The launch function runs exactly once");
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
