#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <cstring>

class TestFixture10 final
	: public Hades::Runtime::IFixture<TestFixture10, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture10(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		PlatformSuite::ScratchDir scratch(L"ops");
		wchar_t a[512], b[512], c[512];
		scratch.join(L"a.bin", a, 512);
		scratch.join(L"b.bin", b, 512);
		scratch.join(L"c.bin", c, 512);

		File::NativeOpenDesc desc{ File::FileAccess::WRITE, File::FileOpenMode::CREATE_ALWAYS_FILE, File::NativeShare::NONE, File::NativeBuffering::BUFFERED };
		void* handle = File::Files::openNative(a, desc);
		const char payload[] = "spectra";
		size_t written = 0;
		PLATFORM_CHECK(handle && File::Files::writeAt(handle, payload, sizeof(payload), 0, written) && File::Files::closeNative(handle));

		File::NativeStat st{};
		PLATFORM_CHECK(File::Files::stat(a, st) && st.m_Size == sizeof(payload) && !st.m_IsDirectory);
		PLATFORM_CHECK(File::Files::stat(scratch.m_Path, st) && st.m_IsDirectory);

		PLATFORM_CHECK(File::Files::renameFile(a, b));
		PLATFORM_CHECK(File::Files::getAttribute(a) == File::PathAttribute::MISSING && File::Files::getAttribute(b) == File::PathAttribute::FILE);

		PLATFORM_CHECK(File::Files::copyFile(b, c, false));
		PLATFORM_CHECK(!File::Files::copyFile(b, c, false) && "Copy without overwrite refuses an existing target");
		PLATFORM_CHECK(File::Files::copyFile(b, c, true));
		PLATFORM_CHECK(File::Files::stat(c, st) && st.m_Size == sizeof(payload));

		PLATFORM_CHECK(File::Files::deleteFile(b) && File::Files::deleteFile(c));
		PLATFORM_CHECK(!File::Files::deleteFile(c) && "Deleting a missing file fails");
		PLATFORM_CHECK(File::Files::getAttribute(c) == File::PathAttribute::MISSING);
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
