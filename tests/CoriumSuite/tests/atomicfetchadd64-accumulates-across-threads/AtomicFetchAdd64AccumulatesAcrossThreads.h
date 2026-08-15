#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <AtomicVariable.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>

class AtomicFetchAdd64AccumulatesAcrossThreads final
	: public Hades::Runtime::IFixture<AtomicFetchAdd64AccumulatesAcrossThreads, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Atomic::AtomicValue64<uint64_t> m_Counter{ 0 };
	static constexpr uint32_t m_numThreads = 4;
	static constexpr uint64_t m_incrementsPerThread = 10000;
	static constexpr uint64_t m_expectedTotal = static_cast<uint64_t>(m_numThreads) * m_incrementsPerThread;
	Corium::Core::ThreadHandle m_threads[m_numThreads]{};
	uint64_t m_finalValue{ 0 };

public:
	explicit AtomicFetchAdd64AccumulatesAcrossThreads(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_Counter.store(0);
		m_finalValue = 0;
	}

	void executeImpl() noexcept {
		m_Counter.store(0);
		Corium::Core::Factory::DefaultThreadFactory threadFactory;

		for (uint32_t i = 0; i < m_numThreads; ++i) {
			m_threads[i] = threadFactory.createAndStart(
				Corium::Core::createClosure<void()>([this]() {
					for (uint64_t j = 0; j < m_incrementsPerThread; ++j) {
						m_Counter.fetchAdd(1);
					}
				}),
				"AtomicAdd64Worker"
			);
		}

		for (uint32_t i = 0; i < m_numThreads; ++i) {
			Corium::Core::NativeThread::joinThread(m_threads[i]);
		}

		m_finalValue = m_Counter.load();
		CORIUM_ASSERT(m_finalValue == m_expectedTotal);
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
		m_Counter.store(0);
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return m_Counter.load();
	}
};


