#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <cstring>

class TestFixture9 final
	: public Hades::Runtime::IFixture<TestFixture9, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture9(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		PlatformSuite::ScratchDir scratch(L"native");
		wchar_t path[512];
		scratch.join(L"data.bin", path, 512);

		uint8_t out[4096], in[4096] = {};
		for (size_t i = 0; i < sizeof(out); ++i) out[i] = static_cast<uint8_t>(i * 7 + 3);

		File::NativeOpenDesc desc{ File::FileAccess::READ_WRITE, File::FileOpenMode::CREATE_ALWAYS_FILE, File::NativeShare::READ, File::NativeBuffering::BUFFERED };
		void* handle = File::Files::openNative(path, desc);
		PLATFORM_CHECK(handle != nullptr);

		size_t written = 0, read = 0;
		PLATFORM_CHECK(File::Files::writeAt(handle, out, sizeof(out), 1000, written) && written == sizeof(out));
		uint64_t size = 0;
		PLATFORM_CHECK(File::Files::getSize(handle, size) && size == 1000 + sizeof(out) && "Writing past the end extends the file");
		PLATFORM_CHECK(File::Files::readAt(handle, in, sizeof(in), 1000, read) && read == sizeof(in));
		PLATFORM_CHECK(std::memcmp(in, out, sizeof(out)) == 0);
		PLATFORM_CHECK(File::Files::readAt(handle, in, sizeof(in), size, read) && read == 0 && "Reading at end of file returns 0 bytes");

		PLATFORM_CHECK(File::Files::setSize(handle, 10) && File::Files::getSize(handle, size) && size == 10);
		PLATFORM_CHECK(File::Files::flushNative(handle));
		PLATFORM_CHECK(File::Files::sectorSize(handle) >= 512);
		PLATFORM_CHECK(File::Files::closeNative(handle));

		File::NativeOpenDesc missing{ File::FileAccess::READ, File::FileOpenMode::OPEN_EXISTING_FILE, File::NativeShare::READ, File::NativeBuffering::BUFFERED };
		wchar_t nope[512];
		scratch.join(L"nope.bin", nope, 512);
		PLATFORM_CHECK(File::Files::openNative(nope, missing) == nullptr && File::Files::lastError() == File::NativeError::NOT_FOUND);
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
