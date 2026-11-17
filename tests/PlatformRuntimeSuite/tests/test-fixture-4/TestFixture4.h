#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformMemory.h>

class TestFixture4 final
	: public Hades::Runtime::IFixture<TestFixture4, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture4(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		using VM = Memory::PlatformVirtualMemory;
		const Memory::PlatformMemoryInfo& info = VM::getMemoryInfo();
		PLATFORM_CHECK(info.m_TotalPhysicalMemory > 0 && info.m_AvailableMemory <= info.m_TotalPhysicalMemory);
		const size_t available = VM::queryAvailableMemory();
		PLATFORM_CHECK(available > 0 && available <= info.m_TotalPhysicalMemory);

		const Memory::PlatformMemoryCapabilities& caps = VM::getMemoryCapabilities();
		PLATFORM_CHECK(caps.m_ActiveNumaNodes >= 1);
		PLATFORM_CHECK(VM::supportsLargePages() == caps.m_SupportsLargePages);
		if (caps.m_SupportsLargePages)
			PLATFORM_CHECK(VM::getLargePageSize() > info.m_PageSize && VM::getLargePageSize() % info.m_PageSize == 0);
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
