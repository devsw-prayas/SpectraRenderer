#include "TestValidationRunner.h"
#include "KerbecsTests.h"
#include "Orchestrator.h"
#include "Logger.h"
#include "HadesEngine.h"
#include "MemoryZone.h"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

#include "Orchestrator.h"
#include "Validator.h"

int main() {
    // 1. Logger Setup
    static Stratum::Logging::DefaultLogger<512> s_Logger;
    static TestRunner::FileSinkRouter s_FileSink("kerbecs_validation.log");
    
    // 2. Initialize Kerbecs Memory Zone (starts Epoch thread/reserves VA)
    Kerbecs::MemoryZone::initShadowzone();

    std::atomic<bool>     epochThreadRunning{ true };
    std::atomic<uint64_t> epochHeartbeat{ 0 };

    std::thread epochThread([&epochThreadRunning, &epochHeartbeat] {
        using namespace std::chrono_literals;

        while (epochThreadRunning.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(1ms);

            auto& zone = Kerbecs::MemoryZone::instance();
            zone.m_Epoch.fetch_add(1, std::memory_order_acq_rel);
            zone.m_Quarantine.flushEligible(zone.m_Registry.poolSegment());
            epochHeartbeat.fetch_add(1, std::memory_order_release);
        }
    });

    // Confirm epoch thread is alive before submitting any work.
    {
        using namespace std::chrono_literals;
        constexpr auto k_Timeout = std::chrono::seconds(2);
        auto deadline = std::chrono::steady_clock::now() + k_Timeout;
        while (epochHeartbeat.load(std::memory_order_acquire) == 0) {
            if (std::chrono::steady_clock::now() >= deadline) {
                std::cerr << "[FATAL] Epoch thread did not start within 2s — aborting.\n";
                epochThreadRunning.store(false, std::memory_order_release);
                epochThread.join();
                return 1;
            }
            std::this_thread::sleep_for(1ms);
        }
        std::cout << "[Epoch] Thread confirmed alive.\n";
    }

    // 3. Setup Hades Engine & Validation dependencies
    KerbecsValidation::CpuAdapter adapter;
    KerbecsValidation::KerbecsStorage storage;
    storage.setExpectedRuns(KerbecsValidation::kCombinatorialRunCount);
    Spectra::Validation::Validator validator("KerbecsValidation", &s_Logger);
    s_Logger.addRoute("KerbecsValidation", &s_FileSink);
    
    Hades::Runtime::HadesEngine<
        KerbecsValidation::CpuAdapter,
        Hades::Runtime::XorHashAccumulator,
        KerbecsValidation::KerbecsStorage> engine(storage, 1);

    std::thread engineThread([&engine] {
        engine.run();
    });

    // 4. Submit Domain 1: Combinatorial Tests 
    KerbecsValidation::submitCombinatorialTests(engine, validator, adapter);

    // 5. Drain the submitted workload and stop the consumer loop cleanly.
    engine.shutdown();
    engineThread.join();

    epochThreadRunning.store(false, std::memory_order_release);
    epochThread.join();

    // 6. Finalize & Flush
    Kerbecs::MemoryZone::teardownShadowzone();
    Stratum::Logging::Orchestrator::getInstance().flush();

    std::cout << "Validation run complete. Check Stratum logs for results.\n";
    return 0;
}
