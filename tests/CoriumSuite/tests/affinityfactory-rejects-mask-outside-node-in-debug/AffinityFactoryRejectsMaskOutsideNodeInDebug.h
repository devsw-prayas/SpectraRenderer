#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumEnvironment.h>
#include <CoriumRuntime.h>
#include <atomic>
#include <optional>

class AffinityFactoryRejectsMaskOutsideNodeInDebug final
	: public Hades::Runtime::IFixture<AffinityFactoryRejectsMaskOutsideNodeInDebug, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::AffinityMask m_validMask{ 0 };
	std::optional<Corium::Core::Factory::AffinityFactory> m_factory;
	std::atomic<bool> m_threadRan{ false };
	Corium::Core::ThreadHandle m_handle{};
	bool m_joined{ false };

public:
	explicit AffinityFactoryRejectsMaskOutsideNodeInDebug(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		const auto& cpuInfo = Corium::Environment::EnvironmentProbe::getCpuInfo();
		m_validMask = cpuInfo.m_NumaNodeMasks[0];
		m_factory.emplace(0, m_validMask);
		m_threadRan.store(false, std::memory_order_relaxed);
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		const auto& cpuInfo = Corium::Environment::EnvironmentProbe::getCpuInfo();
		CORIUM_ASSERT(cpuInfo.m_NumaNodeCount > 0 && "No NUMA nodes detected");
		CORIUM_ASSERT(m_validMask != 0 && "NUMA node 0 mask is empty");

		m_handle = m_factory->createAndStart(
			createClosure<void()>([this]() {
				m_threadRan.store(true, std::memory_order_release);
			}),
			"TestAffinityThread"
		);

		m_joined = NativeThread::joinThread(m_handle);
		CORIUM_ASSERT(m_joined && "NativeThread::joinThread failed to join created thread");
		CORIUM_ASSERT(m_threadRan.load(std::memory_order_acquire) && "Thread closure was not executed");
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

