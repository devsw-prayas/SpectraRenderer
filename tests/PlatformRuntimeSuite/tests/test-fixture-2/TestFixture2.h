#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformMemory.h>

class TestFixture2 final
	: public Hades::Runtime::IFixture<TestFixture2, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture2(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		using VM = Memory::PlatformVirtualMemory;
		const size_t page = VM::getMemoryInfo().m_PageSize;
		const size_t granularity = VM::getMemoryInfo().m_AllocationGranularity;
		PLATFORM_CHECK(page >= 4096 && (page & (page - 1)) == 0);
		PLATFORM_CHECK(granularity >= page && granularity % page == 0);

		PLATFORM_CHECK(VM::alignToPage(1) == page && VM::alignToPage(page) == page && VM::alignToPage(page + 1) == 2 * page);
		PLATFORM_CHECK(VM::alignToGranularity(1) == granularity && VM::alignToGranularity(granularity) == granularity);
		PLATFORM_CHECK(VM::isPageAligned(page * 3) && !VM::isPageAligned(page + 8));
		PLATFORM_CHECK(VM::isGranularityAligned(granularity * 2) && !VM::isGranularityAligned(granularity + page));

		auto* base = reinterpret_cast<void*>(uintptr_t{ granularity } * 16);
		auto* inside = reinterpret_cast<void*>(uintptr_t{ granularity } * 16 + 123);
		PLATFORM_CHECK(VM::isPointerPageAligned(base) && !VM::isPointerPageAligned(inside));
		PLATFORM_CHECK(VM::isPointerGranularityAligned(base) && !VM::isPointerGranularityAligned(inside));
		PLATFORM_CHECK(VM::isPointerPageAligned(VM::alignPointerToPage(inside)));
		PLATFORM_CHECK(VM::isPointerGranularityAligned(VM::alignPointerToGranularity(inside)));
		PLATFORM_CHECK(reinterpret_cast<uintptr_t>(VM::alignPointerToPage(inside)) >= reinterpret_cast<uintptr_t>(inside) && "Aligning rounds up");
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
