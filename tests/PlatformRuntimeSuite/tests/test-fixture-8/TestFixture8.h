#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <cwchar>

class TestFixture8 final
	: public Hades::Runtime::IFixture<TestFixture8, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture8(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		PlatformSuite::ScratchDir scratch(L"dirs");
		wchar_t sub[512], nested[512], file[512];
		scratch.join(L"sub", sub, 512);
		std::swprintf(nested, 512, L"%s\\deeper", sub);
		scratch.join(L"leaf.bin", file, 512);

		PLATFORM_CHECK(File::Files::getAttribute(scratch.m_Path) == File::PathAttribute::DIRECTORY);
		PLATFORM_CHECK(File::Files::getAttribute(sub) == File::PathAttribute::MISSING);
		PLATFORM_CHECK(File::Files::createDirectory(sub) && File::Files::createDirectory(nested));
		PLATFORM_CHECK(File::Files::getAttribute(sub) == File::PathAttribute::DIRECTORY);

		File::NativeOpenDesc desc{ File::FileAccess::WRITE, File::FileOpenMode::CREATE_ALWAYS_FILE, File::NativeShare::NONE, File::NativeBuffering::BUFFERED };
		void* handle = File::Files::openNative(file, desc);
		PLATFORM_CHECK(handle && File::Files::closeNative(handle));
		PLATFORM_CHECK(File::Files::getAttribute(file) == File::PathAttribute::FILE);

		struct Seen { int m_Dirs = 0; int m_Files = 0; int m_Other = 0; } seen;
		PLATFORM_CHECK(File::Files::forEachChild(scratch.m_Path, [](const wchar_t* p_Name, bool v_IsDirectory, void* p_Ctx) {
			auto* s = static_cast<Seen*>(p_Ctx);
			if (v_IsDirectory && std::wcscmp(p_Name, L"sub") == 0) ++s->m_Dirs;
			else if (!v_IsDirectory && std::wcscmp(p_Name, L"leaf.bin") == 0) ++s->m_Files;
			else ++s->m_Other;
			return true;
		}, &seen));
		PLATFORM_CHECK(seen.m_Dirs == 1 && seen.m_Files == 1 && seen.m_Other == 0 && "Each direct child is visited once, without . or ..");

		PLATFORM_CHECK(!File::Files::removeDirectory(sub) && "A non-empty directory is not removed");
		PLATFORM_CHECK(File::Files::removeTree(sub));
		PLATFORM_CHECK(File::Files::getAttribute(sub) == File::PathAttribute::MISSING && File::Files::getAttribute(nested) == File::PathAttribute::MISSING);
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
