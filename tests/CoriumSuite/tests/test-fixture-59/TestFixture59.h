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
#include <type_traits>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class TestFixture59 final
	: public Hades::Runtime::IFixture<TestFixture59, Hades::Runtime::NullDeviceAdapter> {
private:
	static uint64_t threadCpuMs() noexcept {
		FILETIME creation, exit, kernel, user;
		GetThreadTimes(GetCurrentThread(), &creation, &exit, &kernel, &user);
		const uint64_t k = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		const uint64_t u = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		return (k + u) / 10'000;
	}

public:
	explicit TestFixture59(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Runtime::Sync;
		namespace Chrono = Corium::Core::Chrono;

		struct NoDefault {
			uint64_t m_Value;
			explicit NoDefault(uint64_t v_Value) : m_Value(v_Value) {}
			NoDefault(NoDefault&&) = default;
			NoDefault& operator=(NoDefault&&) = default;
		};
		static_assert(!std::is_default_constructible_v<NoDefault>);

		Exchanger<NoDefault> exchanger;
		uint64_t fromThread = 0;
		std::thread partner([&] { fromThread = exchanger.exchange(NoDefault{ 2 }).m_Value; });
		const uint64_t fromMain = exchanger.exchange(NoDefault{ 1 }).m_Value;
		partner.join();
		CORIUM_ASSERT(fromMain == 2 && fromThread == 1 && "Values must swap without default-constructing T");
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
