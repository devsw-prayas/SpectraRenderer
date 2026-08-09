#include "SpectraPlatformRuntime.h"
#include "ProcessEnvironment.h"

#define ALLOW_SYSCALL
#include "SpectraDiagnostics.h"
#include "SpectraIntrin.h"
#include "SpectraSyscalls.h"

namespace Spectra::Platform::Runtime::Environment {
	namespace {
		bool g_IsProcessInfoInitialized;
		CpuInfo g_CpuInfo;
		OsInfo g_OsInfo;
		VectorizeCapabilities g_VectorizeCapabilities;
	}

    void PlatformProbe::init() {
        if (g_IsProcessInfoInitialized) return;

        g_CpuInfo = {};
        g_VectorizeCapabilities = {};
        g_OsInfo = {};

        // =========================
        // CPU TOPOLOGY + CACHE
        // =========================
        DWORD size = 0;
        GetLogicalProcessorInformationEx(RelationAll, nullptr, &size);

        BYTE* buffer = static_cast<BYTE*>(malloc(size));
        if (!buffer) return;

        if (!GetLogicalProcessorInformationEx(RelationAll,
                                              reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer),
                                              &size)) {
            free(buffer);
            return;
        }

        BYTE* p = buffer;
        BYTE* end = buffer + size;

        while (p < end) {
            auto* entry = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(p);

            switch (entry->Relationship) {
            case RelationProcessorCore:
                {
                    g_CpuInfo.m_PhysicalCoreCount++;

                    const auto& proc = entry->Processor;
                    for (WORD i = 0; i < proc.GroupCount; ++i) {
                        g_CpuInfo.m_LogicalCoreCount +=
                            Spec_POPCOUNT64(proc.GroupMask[i].Mask);
                    }
                    break;
                }

            case RelationNumaNode:
                {
                    const auto& numa = entry->NumaNode;
                    if (numa.NodeNumber < SPECTRA_PLATFORM_MAX_NUMA)
                        g_CpuInfo.m_NumaNodeMasks[numa.NodeNumber] = numa.GroupMask.Mask;
                    g_CpuInfo.m_NumaNodeCount++;
                    break;
                }

            case RelationCache:
                {
                    const auto& cache = entry->Cache;

                    if (cache.Level == 1)
                        g_CpuInfo.m_L1CacheSize += cache.CacheSize;
                    else if (cache.Level == 2)
                        g_CpuInfo.m_L2CacheSize += cache.CacheSize;
                    else if (cache.Level == 3)
                        g_CpuInfo.m_L3CacheSize += cache.CacheSize;

                    break;
                }

            default:
                break;
            }

            p += entry->Size;
        }

        free(buffer);
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);

        g_CpuInfo.m_CachedLineSize =
            ((cpuInfo[1] >> 8) & 0xFF) * 8;

        // Base features
        const bool hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
        const bool hasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;
        const bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;

        uint64_t xcr0 = 0;
#ifdef _XCR_XFEATURE_ENABLED_MASK
        xcr0 = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
#else
        xcr0 = _xgetbv(0);
#endif

        const bool avxOS =
            (xcr0 & 0x6) == 0x6;

        int cpuInfoEx[4] = {};
        __cpuidex(cpuInfoEx, 7, 0);

        const bool hasAVX2 =
            (cpuInfoEx[1] & (1 << 5)) != 0;

        const bool hasAVX512 =
            (cpuInfoEx[1] & (1 << 16)) != 0 &&
            ((xcr0 & 0xE0) == 0xE0);

        uint8_t caps = 0;
        if (hasSSE)    caps |= (1 << 0);
        if (hasSSE41)  caps |= (1 << 1);
        if (hasAVX && avxOS) caps |= (1 << 2);
        if (hasAVX2 && avxOS) caps |= (1 << 3);
        if (hasAVX512) caps |= (1 << 4);

        g_VectorizeCapabilities.m_VectorCapabilities = caps;

        typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

        HMODULE hMod = GetModuleHandleA("ntdll.dll");
        if (hMod) {
            // Makes clang shut up
            union {
                FARPROC raw;
                RtlGetVersionPtr typed;
            } fn_cast;

            fn_cast.raw = GetProcAddress(hMod, "RtlGetVersion");
            auto fn = fn_cast.typed;

            if (fn) {
                RTL_OSVERSIONINFOW ver = {};
                ver.dwOSVersionInfoSize = sizeof(ver);                          

                if (fn(&ver) == 0) {
                    g_OsInfo.m_MajorVersion = ver.dwMajorVersion;
                    g_OsInfo.m_MinorVersion = ver.dwMinorVersion;
                    g_OsInfo.m_BuildNumber = ver.dwBuildNumber;
                }
            }
        }

        // Simple product name mapping
        static char s_ProductName[64] = "Windows";

        if (g_OsInfo.m_MajorVersion == 10) {
            if (g_OsInfo.m_BuildNumber >= 22000)
                strcpy_s(s_ProductName, "Windows 11");
            else
                strcpy_s(s_ProductName, "Windows 10");
        }

        g_OsInfo.m_ProductName = s_ProductName;

        g_IsProcessInfoInitialized = true;
    }

    const CpuInfo& PlatformProbe::getCpuInfo() {
        SPECTRA_ASSERT(g_IsProcessInfoInitialized);
        return g_CpuInfo;
    }

    const OsInfo& PlatformProbe::getOsInfo() {
        SPECTRA_ASSERT(g_IsProcessInfoInitialized);
        return g_OsInfo;
    }

    const VectorizeCapabilities& PlatformProbe::getVectorizeCapablities() {
        SPECTRA_ASSERT(g_IsProcessInfoInitialized);
        return g_VectorizeCapabilities;
	}

    uint32_t PlatformProcess::getCurrentProcessId() {
        return GetCurrentProcessId();
    }

    const char* PlatformProcess::getExecutablePath() {
        static char s_Path[MAX_PATH];
        GetModuleFileNameA(nullptr, s_Path, MAX_PATH);
        return s_Path;
    }

    const char* PlatformProcess::getWorkingDirectory() {
        static char s_Dir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, s_Dir);
        return s_Dir;
    }

    size_t PlatformProcess::getEnvironmentVariable(const char* p_Name, char* p_Buffer, unsigned long v_BufferSize) {
        return GetEnvironmentVariableA(p_Name, p_Buffer, v_BufferSize);
    }

    size_t PlatformProcess::setEnvironmentVariable(const char* p_Name, const char* p_Value) {
        return SetEnvironmentVariableA(p_Name, p_Value);
    }

    void PlatformTermination::terminate() {
        TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
        SPECTRA_UNREACHABLE();
    }
}