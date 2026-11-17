#pragma once
#include <ProcessEnvironment.h>
#include <PlatformMemory.h>
#include <PlatformFileSystem.h>
#include <SpectraDiagnostics.h>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Prints which check failed before trapping, since a bare breakpoint exit code says nothing.
#define PLATFORM_CHECK(expr) \
	do { \
		if (!(expr)) { \
			std::fprintf(stderr, "PLATFORM_CHECK failed %s:%d: %s\n", __FILE__, __LINE__, #expr); \
			std::fflush(stderr); \
			__debugbreak(); \
		} \
	} while (0)

namespace PlatformSuite {
	// Same three subsystems SpectraCore boots before anything else.
	inline bool boot() noexcept {
		static const bool s_Booted = [] {
			Spectra::Platform::Runtime::Environment::PlatformProbe::init();
			Spectra::Platform::Runtime::Memory::PlatformVirtualMemory::init();
			Spectra::Platform::Runtime::File::Files::init();
			return true;
		}();
		return s_Booted;
	}

	// A fresh per-process directory under %TEMP%, removed and recreated so a crashed earlier run can't leak into this one.
	struct ScratchDir {
		wchar_t m_Path[512];

		ScratchDir(const wchar_t* p_Name) noexcept {
			using namespace Spectra::Platform::Runtime;
			uint32_t length = 0;
			PLATFORM_CHECK(File::Files::getKnownFolder(File::KnownFolder::TEMP, m_Path, 512, length));
			const uint32_t pid = Environment::PlatformProcess::getCurrentProcessId();
			std::swprintf(m_Path + length, 512 - length, L"%sspectra-platform-%u-%s", (length && m_Path[length - 1] == L'\\') ? L"" : L"\\", pid, p_Name);
			(void)File::Files::removeTree(m_Path);
			PLATFORM_CHECK(File::Files::createDirectory(m_Path));
		}

		~ScratchDir() noexcept {
			(void)Spectra::Platform::Runtime::File::Files::removeTree(m_Path);
		}

		void join(const wchar_t* p_Leaf, wchar_t* p_Out, size_t v_Capacity) const noexcept {
			std::swprintf(p_Out, v_Capacity, L"%s\\%s", m_Path, p_Leaf);
		}
	};
}
