#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumMemoryHandler.h>
#include <CoriumEnvironment.h>
#include <CoriumChrono.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>
#include <atomic>
#include <thread>
#include <chrono>

class ExchangerHighContentionThroughput final
	: public Hades::Runtime::IFixture<ExchangerHighContentionThroughput, Hades::Runtime::NullDeviceAdapter> {
private:
	static constexpr uint32_t s_numThreadPairs = 4;
	static constexpr uint32_t s_totalThreads = s_numThreadPairs * 2;

	std::atomic<uint64_t> m_CompletedExchanges{ 0 };
	Corium::Runtime::Sync::Exchanger<uint64_t> m_exchanger{};
	std::atomic<bool> m_stopFlag{ false };
	Corium::Core::Factory::DefaultThreadFactory m_factory{};
	Corium::Core::ThreadHandle m_handles[s_totalThreads]{};

public:
	explicit ExchangerHighContentionThroughput(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_CompletedExchanges.store(0, std::memory_order_relaxed);
		m_stopFlag.store(false, std::memory_order_relaxed);
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;
		using namespace Corium::Runtime::Sync;
		using namespace Corium::Core::Chrono::Literals;

		for (uint32_t i = 0; i < s_totalThreads; ++i) {
			m_handles[i] = m_factory.createAndStart(
				createClosure<void()>([this, i]() {
					uint64_t myVal = static_cast<uint64_t>(i + 1);
					uint64_t localCount = 0;
					while (!m_stopFlag.load(std::memory_order_relaxed)) {
						myVal = m_exchanger.exchange(std::move(myVal));
						++localCount;
					}
					m_CompletedExchanges.fetch_add(localCount, std::memory_order_relaxed);
				}),
				"ExchangerHighContentionWorker"
			);
		}

		// Run for fixed duration
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		m_stopFlag.store(true, std::memory_order_relaxed);

		for (uint32_t i = 0; i < s_totalThreads; ++i) {
			bool joined = NativeThread::joinThread(m_handles[i]);
			CORIUM_ASSERT(joined && "Thread failed to join");
		}

		CORIUM_ASSERT(m_CompletedExchanges.load(std::memory_order_relaxed) > 0 && "Completed exchanges should be > 0");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
		m_CompletedExchanges.store(0, std::memory_order_relaxed);
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return m_CompletedExchanges.load(std::memory_order_relaxed);
	}
};

