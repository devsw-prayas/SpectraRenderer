# Spectra Testing Manifesto

## Why We Test This Way
Testing in a renderer isn't just about checking a box. It’s about **Replayability** and **Hardening**. We use a three-tier system—Stratum (Logs), Hades (Benchmarks), and SVK (Validation)—to make sure that if a component works once, it works every single time, exactly the same way.

## 1. File Layout: One Test, One File
We follow a strict "One test per .cpp" rule to keep the build isolated and the logic focused. The header file should contain all declarations for the domain, but the actual test implementation lives in its own dedicated source file.

```
tests/
└── <LibraryValidation>/       ← static lib, one domain per .cpp
    └── src/
        ├── Public/
        │   └── <Library>Tests.h    ← All declarations; exports submitXxxTests()
        └── Private/
            └── <SpecificTest>.cpp  ← One test implementation
```

## 2. Includes
Every test combines the three pillars of our validation system.

```cpp
// Hades — The Judge
#include "HadesEngine.h"       // HadesEngine, Config, makeFactory
#include "IFixture.h"          // IFixture<D, A>
#include "IStorageBackend.h"   // IStorageBackend<D>
#include "XorHashAccumulator.h"

// Stratum — The Observer
#include "StratumOrchestrator.h"   // Orchestrator::getInstance()
#include "DefaultLogger.h"         // DefaultLogger<N>

// SVK — The Validator
#include "Validator.h"             // Validator
#include "ValidationFixture.h"     // ValidationFixture<D, A>
```

## 3. The Goal: Determinism & Replayability
Every test in Spectra must be deterministic.
*   **The "Why":** If a test passes on my machine but fails on yours, it's useless. We use the Hades XOR hashing system to compare execution "slices." 
*   **Replayability:** I want every component to be replayable. If we find a bug, I want to be able to feed the exact same state back into the engine and get the exact same result.

### The Storage Backend
Implement once per domain. It receives the `BenchmarkResult` where you can verify the `deterministic` flag.

```cpp
struct <Domain>Storage : Hades::Runtime::IStorageBackend<<Domain>Storage> {
    void storeImpl(const Hades::Runtime::BenchmarkResult& r_Result) {
        // Log, assert thresholds, or store for later inspection.
        // r_Result.deterministic  → false means hash diverged across slices!
        // r_Result.meanThroughput → iterations/sec
        // GPU fields only valid when r_Result.isGpuRun == true
    }
};
```

## 4. The Hades Fixture
Hades handles the fixture lifecycle. Keep the `executeImpl` tight and the `resetImpl` clean for replayability.

```cpp
struct <TestName>Fixture : Spectra::Validation::ValidationFixture<<TestName>Fixture, CpuAdapter> {
    // ── Construction ────────────────────────────────────────────────────────
    explicit <TestName>Fixture(CpuAdapter& ro_Adapter, Spectra::Validation::Validator& ro_Validator)
        : ValidationFixture(ro_Adapter, ro_Validator) {}

    // ── Lifecycle ────────────────────────────────────────────────────────────
    // Called once before the run. Allocate and prepare inputs here.
    void startupImpl() {}

    // Called every iteration. This is the hot path — keep it tight.
    void executeImpl() {}

    // CRITICAL FOR REPLAYABILITY: Reset mutable state between iterations.
    // Must not allocate memory.
    void resetImpl(CpuAdapter&) {}

    // Called once after the run. Release resources.
    void teardownImpl() {}

    // ── Determinism hash ─────────────────────────────────────────────────────
    // Return a value that uniquely represents the output state after execute().
    // Changing hash → non-deterministic test → flagged in BenchmarkResult.
    uint64_t getDeterminismHashImpl() {
        return 0; // hash of output buffer, counter, etc.
    }
};
```

## 5. Device Adapters
For CPU tests, use a minimal no-op adapter. For GPU tests, implement the full CUDA path.

```cpp
// CPU no-op adapter
struct CpuAdapter : Hades::Runtime::IDeviceAdapter<CpuAdapter> {
    using event_type = uint64_t;
    void   synchronizeImpl()                                        {}
    void   recordEventImpl(event_type&)                             {}
    float  elapsedTimeImpl(const event_type&, const event_type&)   { return 0.0f; }
    void   memcpyToHostImpl(void*, const void*, size_t)             {}
    void*  streamHandleImpl()                                       { return nullptr; }
};
```

## 6. Validation: Observe then Harden
We distinguish between "Hard" and "Soft" checks. Use Soft checks to observe new features, then Hard checks to lock them in once stable.

### Submission & Config
The submission function is the entry point called by the runner.

```cpp
void submit<Domain>Tests(
    Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, <Domain>Storage>& ro_Engine,
    Spectra::Validation::Validator& ro_Validator,
    CpuAdapter& ro_Adapter) {
    
    Hades::Runtime::Config cfg;
    cfg.m_ThreadCount   = 1;               // 0 = hardware_concurrency
    cfg.m_Iterations    = 1;               // 0 = auto-calibrated; use 1 for functional correctness
    cfg.m_WarmupCount   = 2;
    cfg.m_CvThreshold   = 0.02;            // stop when stddev/mean < 2%
    cfg.m_MinSlices     = 1;               // use 1 for functional, higher for benchmarks
    cfg.m_MaxSlices     = 32;
    cfg.m_ChronoBackend = Hades::Runtime::ChronoBackend::SteadyClock;
    
    auto factory = Hades::Runtime::makeFactory<<TestName>Fixture, CpuAdapter>();
    ro_Engine.submit(factory, cfg);
}
```

### Logger & Validator Setup (Runner Side)
This is done once in the runner executable, not per test.

```cpp
// Logger — one per validation run
static Stratum::Logging::DefaultLogger<512> s_Logger;
Stratum::Logging::Orchestrator::getInstance().registerLogger("<suite-name>", &s_Logger);

// Validator — owns the logger name, passed into every fixture
Spectra::Validation::Validator validator("<suite-name>", &s_Logger);

// Flush after all runs complete
Stratum::Logging::Orchestrator::getInstance().flush();
```

## 7. Quick Reference for Validation
| Intent | Tool |
| :--- | :--- |
| **Harden** a requirement (breaks on fail) | `verify(cond, "msg")` |
| **Observe** a behavior (soft warn) | `m_Validator.expect(cond, "msg")` |
| **Compare** raw memory (byte-exact) | `validateBitwise("label", ref, test, bytes)` |
| **Verify** math/floats (epsilon match) | `validateNumerical("label", ref, test, count, eps)` |
| Emit info log | `Stratum::Logging::Orchestrator::getInstance().log(Info, ...)` |
| Scoped trace | `STRATUM_TRACE(name, "Comp", "label")` ... `STRATUM_TRACE_END` |

## The Golden Rule
A test that isn't deterministic isn't a test—it's a random number generator. If your hash is flickering, don't ignore it. Find the uninitialized variable or the race condition and kill it.
