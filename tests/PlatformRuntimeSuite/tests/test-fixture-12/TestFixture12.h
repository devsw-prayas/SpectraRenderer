#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <cstring>

class TestFixture12 final
	: public Hades::Runtime::IFixture<TestFixture12, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture12(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		char buffer[256] = {};
		const char* name = "SPECTRA_PLATFORM_SUITE_VAR";
		PLATFORM_CHECK(Environment::PlatformProcess::getEnvironmentVariable("SPECTRA_PLATFORM_SUITE_MISSING", buffer, sizeof(buffer)) == 0);

		PLATFORM_CHECK(Environment::PlatformProcess::setEnvironmentVariable(name, "spectral") != 0);
		const size_t length = Environment::PlatformProcess::getEnvironmentVariable(name, buffer, sizeof(buffer));
		PLATFORM_CHECK(length == 8 && std::strcmp(buffer, "spectral") == 0);

		PLATFORM_CHECK(Environment::PlatformProcess::setEnvironmentVariable(name, "x") != 0);
		PLATFORM_CHECK(Environment::PlatformProcess::getEnvironmentVariable(name, buffer, sizeof(buffer)) == 1 && std::strcmp(buffer, "x") == 0);
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
