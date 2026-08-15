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

class AtomicFetchAdd32AccumulatesAcrossThreads final
	: public Hades::Runtime::IFixture<AtomicFetchAdd32AccumulatesAcrossThreads, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit AtomicFetchAdd32AccumulatesAcrossThreads(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		Corium::Core::Atomic::AtomicValue32<uint32_t> counter{ 0 };
		uint32_t numThreads = 4;
		uint32_t incrementsPerThread = 10000;
		uint32_t expectedTotal = numThreads * incrementsPerThread;
		Corium::Core::Factory::DefaultThreadFactory threadFactory;
		Corium::Core::ThreadHandle threads[4]{};

		for (uint32_t i = 0; i < numThreads; ++i) {
			threads[i] = threadFactory.createAndStart(
				Corium::Core::createClosure<void()>([&counter, incrementsPerThread]() {
					for (uint32_t j = 0; j < incrementsPerThread; ++j) {
						counter.fetchAdd(1);
					}
				}),
				"AtomicAdd32Worker"
			);
		}

		for (uint32_t i = 0; i < numThreads; ++i) {
			Corium::Core::NativeThread::joinThread(threads[i]);
		}

		uint32_t finalValue = counter.load();
		CORIUM_ASSERT(finalValue == expectedTotal);
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


