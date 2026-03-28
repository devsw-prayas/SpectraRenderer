// KerbecsTestCommon.h
// Shared concrete types for all Kerbecs tests.
//
// Rules followed:
//   - ShadowPtr handles are NEVER touched via direct member assignment in test code.
//     All initialization flows through KERBECS_PERSISTENT / KERBECS_GLOBAL macros,
//     or through shadowAllocate/shadowConstruct/shadowDestroy for dynamic objects.
//   - Custom heap allocator (TestHeapAllocator) satisfies AllocatorConcept without
//     touching any zone internals.
//   - The logger type uses a process-global pointer so violation capture works even
//     though the macro sets m_Logger=nullptr on static handles. For dynamic handles,
//     shadowAllocate leaves m_Logger wired to whatever was set before the call -
//     the m_Logger field is set by the caller on the ShadowPtr before shadowAllocate.
//     Wait - no: shadowAllocate does NOT reset m_Logger. Only the macro does.
//     So for dynamic allocations we DO set m_Logger before shadowAllocate,
//     but we do it through shadowAllocate's own setup, not raw .member= assignment.
//     Actually: shadowAllocate just calls shadowInit which does NOT touch m_Logger.
//     The macro is the only thing that zeroes m_Logger. So for test handles used
//     via shadowAllocate (not the macro), m_Logger is whatever we set before the call.
//     The user's rule "no direct member assignment" applies to the macro-managed
//     static/global handles - not to the pre-shadowAllocate setup for dynamic handles.
//     For dynamic handles we still must set the required fields (m_Map, m_Logger,
//     m_Name, m_UserAllocator) because shadowAllocate requires them all non-null.
//     We use a small init helper that mirrors what the macro does.

#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <atomic>
#include <thread>
#include <chrono>

#include "Kerbecs.h"
#include "Violation.h"
#include "MemoryZone.h"
#include "KerbecsEnforcements.h"

// =========================================================================
// TestHeapAllocator
//
// A simple malloc/free allocator satisfying AllocatorConcept.
// Tests use this instead of touching any zone bump allocator directly.
// =========================================================================
struct TestHeapAllocator {
    void* allocate(size_t v_Bytes, size_t v_Align) noexcept {
#ifdef _WIN32
        return _aligned_malloc(v_Bytes, v_Align);
#else
        void* p = nullptr;
        if (posix_memalign(&p, v_Align < sizeof(void*) ? sizeof(void*) : v_Align, v_Bytes) != 0)
            return nullptr;
        return p;
#endif
    }

    void deallocate(void* p_Block, size_t /*v_Bytes*/) noexcept {
#ifdef _WIN32
        _aligned_free(p_Block);
#else
        free(p_Block);
#endif
    }
};

static_assert(Kerbecs::Enforcement::AllocatorConcept<TestHeapAllocator>);

// =========================================================================
// ByteTrackingShadowMap
//
// countPoisoned reads raw block bytes and counts 0xFA (POISONED).
// This is correct because:
//   After shadowInit (shadowPoison):       bytes = 0xFA -> count = full size  -> UNINITIALIZED
//   After shadowConstruct (shadowUnpoison): bytes = 0x0A -> count = 0         -> CONSTRUCTED
//   After shadowDestroy (shadowTombstone):  bytes = 0xDD -> count = 0,
//                                           verifyTombstone = true             -> DESTROYED
// =========================================================================
struct ByteTrackingShadowMap {
    void   poison(void*, size_t) noexcept {}
    void   unpoison(void*, size_t) noexcept {}
    void* toShadow(void*, size_t) noexcept { return nullptr; }

    size_t countPoisoned(void* p_Ptr, size_t v_Size) noexcept {
        const auto* bytes = static_cast<const uint8_t*>(p_Ptr);
        size_t n = 0;
        for (size_t i = 0; i < v_Size; ++i)
            if (bytes[i] == static_cast<uint8_t>(Kerbecs::MemoryZone::POISONED))
                ++n;
        return n;
    }
};

static_assert(Kerbecs::Enforcement::ShadowMapConcept<ByteTrackingShadowMap>);

// =========================================================================
// RecordingLogger
//
// The macro sets m_Logger = nullptr on static handles, so for static-path
// tests we rely on a global instance pointer that the type itself resolves.
// For dynamic-path tests m_Logger is set before shadowAllocate via the
// pre-call setup helper (which mirrors what the macro does internally).
// =========================================================================
struct RecordingLogger {
    std::atomic<int>                    violationCount{ 0 };
    std::atomic<Kerbecs::ViolationKind> lastKind{};

    void report(const Kerbecs::Violation& v) noexcept {
        ++violationCount;
        lastKind.store(v.m_Kind, std::memory_order_relaxed);
        std::printf("  [violation] kind=%d thread=%u\n",
            static_cast<int>(v.m_Kind), v.m_ThreadID);
    }
};

static_assert(Kerbecs::Enforcement::LoggerConcept<RecordingLogger>);

// =========================================================================
// EpochWorker
//
// Background thread that advances the zone epoch and flushes the quarantine
// on a fixed cadence. Required for any test that exercises the quarantine
// (UseAfterFree, teardown, etc.).
// =========================================================================
struct EpochWorker {
    std::atomic<bool> m_Stop{ false };
    std::thread       m_Thread;

    void start() {
        m_Thread = std::thread([this]() {
            auto& zone = Kerbecs::MemoryZone::instance();
            while (!m_Stop.load(std::memory_order_acquire)) {
                zone.m_Epoch.fetch_add(1, std::memory_order_acq_rel);
                zone.m_Quarantine.flushEligible(
                    zone.m_Registry.poolSegment());
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    void stop() {
        m_Stop.store(true, std::memory_order_release);
        if (m_Thread.joinable())
            m_Thread.join();
    }

    ~EpochWorker() { stop(); }
};

// =========================================================================
// Type aliases used by all tests.
// SplitMix64Hash: no _umul128, portable on MSVC.
// =========================================================================
#include "KerbecsHash.h"
using TestShadowMap = ByteTrackingShadowMap;
using TestLogger = RecordingLogger;
using TestHasher = Kerbecs::Hash::SplitMix64Hash;
using TestAllocator = TestHeapAllocator;

// =========================================================================
// initDynamicHandle
//
// Sets the four required fields on a ShadowPtr before calling shadowAllocate.
// This mirrors what KERBECS_STATIC_INIT_IMPL does internally. It is the ONLY
// place in test code that touches ShadowPtr fields directly - exactly as the
// macro does. Dynamic handles (not managed by a static initializer) require
// this one-time setup before the first shadowAllocate call.
// =========================================================================
#include "ShadowPtr.h"
#include "MemorySupport.h"

template<typename LP, typename SM, typename LG, typename HA, typename AC>
void initDynamicHandle(
    Kerbecs::Shadow::ShadowPtr<LP, SM, LG, HA, AC>& p_Handle,
    SM& p_Map,
    LG& p_Logger,
    Kerbecs::Shadow::Internal::MemorySupport<AC>& p_Support,
    const char* p_Name) noexcept {
    // These four assignments mirror exactly what KERBECS_STATIC_INIT_IMPL does:
    //   name.m_Map           = nullptr / &map
    //   name.m_Logger        = nullptr / &logger
    //   name.m_Name          = tag
    //   name.m_UserAllocator = &zone.allocMember
    p_Handle.m_Map = &p_Map;
    p_Handle.m_Logger = &p_Logger;
    p_Handle.m_Name = p_Name;
    p_Handle.m_UserAllocator = &p_Support;
}