#pragma once
#include "SpectraPlatformRuntime.h"

namespace Spectra::Platform::Runtime::Environment {
	struct SPECTRA_RUNTIME_API alignas(32) CpuInfo final {
		uint32_t m_LogicalCoreCount;
		uint32_t m_PhysicalCoreCount;
		uint32_t m_NumaNodeCount;
		uint32_t m_L1CacheSize;
		uint32_t m_L2CacheSize;
		uint32_t m_L3CacheSize;
		uint32_t m_CachedLineSize;

		CpuInfo() = default;
		~CpuInfo() = default;

		CpuInfo(const CpuInfo&) = default;
		CpuInfo& operator=(const CpuInfo&) = default;

		CpuInfo(CpuInfo&&) noexcept = default;
		CpuInfo& operator=(CpuInfo&&) noexcept = default;
	};

	struct SPECTRA_RUNTIME_API alignas(8) VectorizeCapabilities final {
		uint8_t m_VectorCapabilities;
		// bit-0 SSE
		// bit-1 SSE 4.1
		// bit-2 AVX
		// bit-3 AVX2
		// bit-4 AVX-512f

		VectorizeCapabilities() = default;
		~VectorizeCapabilities() = default;

		VectorizeCapabilities(const VectorizeCapabilities&) = default;
		VectorizeCapabilities& operator=(const VectorizeCapabilities&) = default;

		VectorizeCapabilities(VectorizeCapabilities&&) noexcept = default;
		VectorizeCapabilities& operator=(VectorizeCapabilities&&) noexcept = default;
	};

	struct SPECTRA_RUNTIME_API alignas(32) OsInfo final {
		const char* m_ProductName;
		uint32_t m_MinorVersion;
		uint32_t m_MajorVersion;
		uint32_t m_BuildNumber;

		OsInfo() = default;
		~OsInfo() = default;

		OsInfo(const OsInfo&) = default;
		OsInfo& operator=(const OsInfo&) = default;

		OsInfo(OsInfo&&) noexcept = default;
		OsInfo& operator=(OsInfo&&) noexcept = default;
	};

	class SPECTRA_RUNTIME_API PlatformProbe final {
	public:
		static void init();
		static const CpuInfo& getCpuInfo();
		static const VectorizeCapabilities& getVectorizeCapablities();
		static const OsInfo& getOsInfo();
	};

	class SPECTRA_RUNTIME_API PlatformProcess final {
	public:
		static uint32_t getCurrentProcessId();
		static const char* getExecutablePath();
		static const char* getWorkingDirectory();
		static size_t getEnvironmentVariable(const char* p_Name, char* p_Buffer, unsigned long v_BufferSize);
		static size_t setEnvironmentVariable(const char* p_Name, const char* p_Value);
	};

	class SPECTRA_RUNTIME_API PlatformTermination final {
	public:
		SPECTRA_NORETURN static void terminate();
	};

}
