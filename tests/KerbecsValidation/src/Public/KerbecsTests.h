#pragma once

#include "HadesEngine.h"
#include "Validator.h"
#include "Fixture.h"
#include "Storage.h"
#include "KerbecsDiagnostics.h"
#include "HadesHash.h"
#include <atomic>
#include <iostream>

#ifdef KerbecsValidation_EXPORTS
#   define KERBECSVALIDATION_API __declspec(dllexport)
#else
#   define KERBECSVALIDATION_API __declspec(dllimport)
#endif

namespace KerbecsValidation {
    inline constexpr uint32_t kCombinatorialRunCount = 1440;

    // CPU no-op adapter
    struct CpuAdapter : Hades::Runtime::IDeviceAdapter<CpuAdapter> {
        using event_type = uint64_t;
        void   synchronizeImpl()                                        {}
        void   recordEventImpl(event_type&)                             {}
        float  elapsedTimeImpl(const event_type&, const event_type&) const { return 0.0f; }
        void   memcpyToHostImpl(void*, const void*, size_t)             {}
        void*  streamHandleImpl()                                       { return nullptr; }
    };

    // Shared storage backend for Kerbecs tests
    struct KerbecsStorage : Hades::Runtime::IStorageBackend<KerbecsStorage> {
        void setExpectedRuns(uint32_t expectedRuns) noexcept {
            m_ExpectedRuns.store(expectedRuns, std::memory_order_release);
            m_CompletedRuns.store(0, std::memory_order_release);
        }

        void storeImpl(const Hades::Runtime::BenchmarkResult& r_Result) {
            // Functional correctness tests do not strictly require processing the results here unless specified
            KERBECS_UNUSED(r_Result);

            const uint32_t completed =
                m_CompletedRuns.fetch_add(1, std::memory_order_acq_rel) + 1;
            const uint32_t expected =
                m_ExpectedRuns.load(std::memory_order_acquire);

            if (expected > 0 &&
                (completed == 1 || (completed % 50) == 0 || completed == expected)) {
                std::cout
                    << "[KerbecsValidation] progress "
                    << completed << '/' << expected
                    << " runs completed\n";
            }
        }

    private:
        std::atomic<uint32_t> m_ExpectedRuns{ 0 };
        std::atomic<uint32_t> m_CompletedRuns{ 0 };
    };

    // Submits the Combinatorial Test Matrix (~600 Tests)
    KERBECSVALIDATION_API void submitCombinatorialTests(
        Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& ro_Engine,
        Spectra::Validation::Validator&               ro_Validator,
        CpuAdapter&                                   ro_Adapter);

} // namespace KerbecsValidation
