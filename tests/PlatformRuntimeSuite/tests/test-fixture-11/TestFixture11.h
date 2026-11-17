#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <cstring>

class TestFixture11 final
	: public Hades::Runtime::IFixture<TestFixture11, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture11(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		const Environment::CpuInfo& cpu = Environment::PlatformProbe::getCpuInfo();
		PLATFORM_CHECK(cpu.m_LogicalCoreCount >= 1 && cpu.m_PhysicalCoreCount >= 1 && cpu.m_PhysicalCoreCount <= cpu.m_LogicalCoreCount);
		PLATFORM_CHECK(cpu.m_NumaNodeCount >= 1 && cpu.m_NumaNodeMasks[0] != 0);
		PLATFORM_CHECK(cpu.m_CachedLineSize == 64 || cpu.m_CachedLineSize == 128);
		PLATFORM_CHECK(cpu.m_L1CacheSize > 0 && cpu.m_L2CacheSize >= cpu.m_L1CacheSize);
		PLATFORM_CHECK(Environment::PlatformProbe::getCurrentNumaNode() < cpu.m_NumaNodeCount);

		const Environment::OsInfo& os = Environment::PlatformProbe::getOsInfo();
		PLATFORM_CHECK(os.m_MajorVersion >= 10 && os.m_BuildNumber > 0);

		const uint32_t pid = Environment::PlatformProcess::getCurrentProcessId();
		PLATFORM_CHECK(pid != 0 && Environment::PlatformProcess::isProcessAlive(pid));
		PLATFORM_CHECK(!Environment::PlatformProcess::isProcessAlive(0xFFFFFFF0u) && "A pid that cannot exist is not alive");

		const char* exe = Environment::PlatformProcess::getExecutablePath();
		const char* cwd = Environment::PlatformProcess::getWorkingDirectory();
		PLATFORM_CHECK(exe && std::strlen(exe) > 4 && std::strstr(exe, ".exe") != nullptr);
		PLATFORM_CHECK(cwd && std::strlen(cwd) > 0);
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
