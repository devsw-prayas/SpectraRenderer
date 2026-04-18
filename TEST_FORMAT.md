# Test Template

This document defines the standard structure for writing a test in SpectraRenderer. Every test combines three systems:
- **Stratum** — structured logging and exception reporting
- **Hades** — benchmark/test runner (fixture lifecycle, threading, stats)
- **SVK** — validation layer (buffer comparison, pass/fail emission)

---

## 1. File Layout

```
tests/
└── <LibraryValidation>/       ← static lib, one domain per .cpp
    └── src/
        ├── Public/
        │   └── <Library>Tests.h    ← exports submitXxxTests()
        └── Private/
            └── <Domain>Tests.cpp   ← one file per test domain
```

One `.cpp` per domain. No logic in headers. The runner exe calls `submitXxxTests()`.

---

## 2. Includes

```cpp
// Hades
#include "HadesEngine.h"       // HadesEngine, Config, makeFactory
#include "IFixture.h"          // IFixture<D, A>
#include "IStorageBackend.h"   // IStorageBackend<D>
#include "XorHashAccumulator.h"

// Stratum
#include "StratumOrchestrator.h"   // Orchestrator::getInstance()
#include "DefaultLogger.h"         // DefaultLogger<N>

// SVK
#include "Validator.h"             // Validator
#include "ValidationFixture.h"     // ValidationFixture<D, A>
```

---

## 3. Storage Backend

Implement once per domain (or share across a file). Receives one `BenchmarkResult` per completed run.

```cpp
struct <Domain>Storage : Hades::Runtime::IStorageBackend<<Domain>Storage>
{
    void storeImpl(const Hades::Runtime::BenchmarkResult& r_Result)
    {
        // Log, assert thresholds, or store for later inspection.
        // r_Result.deterministic  → false means hash diverged across slices
        // r_Result.meanThroughput → iterations/sec
        // GPU fields only valid when r_Result.isGpuRun == true
    }
};
```

---

## 4. Device Adapter

For CPU tests use a minimal no-op adapter. For GPU tests implement the full CUDA path.

```cpp
// CPU no-op adapter (copy-paste for CPU-only tests)
struct CpuAdapter : Hades::Runtime::IDeviceAdapter<CpuAdapter>
{
    using event_type = uint64_t;
    void   synchronizeImpl()                                        {}
    void   recordEventImpl(event_type&)                             {}
    float  elapsedTimeImpl(const event_type&, const event_type&)   { return 0.0f; }
    void   memcpyToHostImpl(void*, const void*, size_t)             {}
    void*  streamHandleImpl()                                       { return nullptr; }
};
```

---

## 5. Fixture

Derive from `ValidationFixture` (which itself derives from `IFixture`). Keep each fixture focused on one behaviour.

```cpp
struct <TestName>Fixture : Spectra::Validation::ValidationFixture<<TestName>Fixture, CpuAdapter>
{
    // ── Construction ────────────────────────────────────────────────────────
    explicit <TestName>Fixture(CpuAdapter& ro_Adapter, Spectra::Validation::Validator& ro_Validator)
        : ValidationFixture(ro_Adapter, ro_Validator)
    {}

    // ── Lifecycle ────────────────────────────────────────────────────────────
    // Called once before the run. Allocate and prepare inputs here.
    void startupImpl()
    {
    }

    // Called every iteration. This is the hot path — keep it tight.
    void executeImpl()
    {
    }

    // Called between iterations. Reset mutable state so the next execute
    // starts clean. Must not allocate.
    void resetImpl(CpuAdapter&)
    {
    }

    // Called once after the run. Release resources.
    void teardownImpl()
    {
    }

    // ── Determinism hash ─────────────────────────────────────────────────────
    // Return a value that uniquely represents the output state after execute().
    // Hades XORs hashes across threads per slice and compares slice-to-slice.
    // A changing hash → non-deterministic test → flagged in BenchmarkResult.
    uint64_t getDeterminismHashImpl()
    {
        return 0; // hash of output buffer, counter, etc.
    }

    // ── Validation ───────────────────────────────────────────────────────────
    // Call from executeImpl() or teardownImpl().
    //
    // Hard check — emits ExceptionEntry, triggers debug break on failure:
    //   verify(condition, "message")
    //
    // Soft check — emits LogEntry(Warning), no break:
    //   m_Validator.expect(condition, "message")
    //
    // Buffer checks (AVX2-accelerated):
    //   validateBitwise ("label", p_Ref, p_Test, byteCount)
    //   validateNumerical("label", p_Ref, p_Test, count, epsilon)
};
```

---

## 6. Hades Config

```cpp
Hades::Runtime::Config buildConfig()
{
    Hades::Runtime::Config cfg;
    cfg.m_ThreadCount   = 1;               // 0 = hardware_concurrency
    cfg.m_Iterations    = 100;             // 0 = auto-calibrated
    cfg.m_WarmupCount   = 2;
    cfg.m_CvThreshold   = 0.02;            // stop when stddev/mean < 2%
    cfg.m_MinSlices     = 4;
    cfg.m_MaxSlices     = 32;
    cfg.m_ChronoBackend = Hades::Runtime::ChronoBackend::SteadyClock;
    return cfg;
}
```

For functional correctness tests (not perf) use `m_Iterations = 1`, `m_MinSlices = 1`, `m_MaxSlices = 1`.

---

## 7. Submission Function

This is the entry point called by the runner exe.

```cpp
void submit<Domain>Tests(
    Hades::Runtime::HadesEngine<CpuAdapter,
                                Hades::Runtime::XorHashAccumulator,
                                <Domain>Storage>& ro_Engine,
    Spectra::Validation::Validator&               ro_Validator,
    CpuAdapter&                                   ro_Adapter)
{
    auto factory = Hades::Runtime::makeFactory<<TestName>Fixture, CpuAdapter>();
    ro_Engine.submit(factory, buildConfig());

    // Add more submissions here for additional test cases in this domain.
}
```

---

## 8. Logger & Validator Setup (Runner Side)

Done once in the runner exe, not per test.

```cpp
// Logger — one per validation run
static Stratum::Logging::DefaultLogger<512> s_Logger;
Stratum::Logging::Orchestrator::getInstance().registerLogger("<suite-name>", &s_Logger);

// Optional: restrict log level
s_Logger.filter().setMinLevel(Stratum::LogLevel::Warning);

// Validator — owns the logger name, passed into every fixture
Spectra::Validation::Validator validator("<suite-name>", &s_Logger);

// Flush after all runs complete
Stratum::Logging::Orchestrator::getInstance().flush();
```

---

## 9. Quick Reference

| What you want | What to call |
|---|---|
| Hard assert (breaks on fail) | `verify(cond, "msg")` |
| Soft warn (no break) | `m_Validator.expect(cond, "msg")` |
| Byte-exact buffer compare | `validateBitwise("label", ref, test, bytes)` |
| Float buffer compare | `validateNumerical("label", ref, test, count, eps)` |
| Emit info log | `Stratum::Logging::Orchestrator::getInstance().log(Info, ...)` |
| Scoped trace | `STRATUM_TRACE(name, "Component", "label")` ... `STRATUM_TRACE_END` |
| Check determinism | inspect `BenchmarkResult::deterministic` in storage backend |
| GPU timing | inspect `BenchmarkResult::deviceKernelTimeMean` (only when `isGpuRun`) |
