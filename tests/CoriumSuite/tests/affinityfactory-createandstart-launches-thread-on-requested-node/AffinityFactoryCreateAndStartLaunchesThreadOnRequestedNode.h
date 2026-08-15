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

class AffinityFactoryCreateAndStartLaunchesThreadOnRequestedNode final
	: public Hades::Runtime::IFixture<AffinityFactoryCreateAndStartLaunchesThreadOnRequestedNode, Hades::Runtime::NullDeviceAdapter> {
private:
	std::atomic<bool> m_threadRan{ false };
	uint32_t m_targetNode{ 0 };
	Corium::Core::AffinityMask m_mask{ 0x1 };
	std::optional<Corium::Core::Factory::AffinityFactory> m_factory;
	Corium::Core::ThreadHandle m_handle{};
	uint32_t m_actualNode{ 0 };
	bool m_joined{ false };

public:
	explicit AffinityFactoryCreateAndStartLaunchesThreadOnRequestedNode(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_threadRan.store(false, std::memory_order_relaxed);
		m_targetNode = 0;
		m_mask = 0x1;
		m_factory.emplace(m_targetNode, m_mask);
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_handle = m_factory->createAndStart(
			createClosure<void()>([this]() {
				m_threadRan.store(true, std::memory_order_release);
			}),
			"TestAffinityThread"
		);

		m_actualNode = NativeThread::getNumaNode(m_handle);
		CORIUM_ASSERT(m_actualNode == m_targetNode && "NativeThread::getNumaNode did not return requested node 0");

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


