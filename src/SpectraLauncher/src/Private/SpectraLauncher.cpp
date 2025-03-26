#include "SpectraInstrumentation.h"
#include <iostream>
#include <thread>
#include <chrono>


int main() {
    spectra::instrumentation::Instrumentation::enableColoredConsole(true, spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "=== Starting Manual Tests for SpectraInstrumentation ===\n\n";

    // Test 1: Basic Log Creation and Formatting
    std::cout << "Test 1: Basic Log Creation and Formatting\n";
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::INFO,
        "TestComponent",
        "TestSubComponent",
        "This is a basic log message",
        spectra::instrumentation::E_LogComponent::MATH,
        42, "Hello", nullptr, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console and math_log.txt for a formatted INFO log with args (42, Hello, null).\n\n";

    // Test 2: Log Level Filtering
    std::cout << "Test 2: Log Level Filtering\n";
    spectra::instrumentation::Instrumentation::setMinLevel(
        spectra::instrumentation::E_LogLevel::WARNING,
        spectra::instrumentation::E_LogComponent::MATH
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::INFO,
        "TestComponent",
        "TestSubComponent",
        "This INFO log should be ignored due to min level WARNING",
        spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::WARNING,
        "TestComponent",
        "TestSubComponent",
        "This WARNING log should be recorded",
        spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console and math_log.txt: Only the WARNING log should appear.\n\n";

    // Test 3: Output Destination Control (Console Only)
    std::cout << "Test 3: Output Destination Control (Console Only)\n";
    spectra::instrumentation::Instrumentation::setOutputDestinations(
        spectra::instrumentation::E_LogOutput::CONSOLE,
        spectra::instrumentation::E_LogComponent::MATH
    );
    spectra::instrumentation::Instrumentation::setMinLevel(
        spectra::instrumentation::E_LogLevel::DEBUG,
        spectra::instrumentation::E_LogComponent::MATH
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::DEBUG,
        "TestComponent",
        "TestSubComponent",
        "This DEBUG log should only go to console",
        spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console: DEBUG log should appear. Check math_log.txt: No new log should be added.\n\n";

    // Test 4: Output Destination Control (File Only)
    std::cout << "Test 4: Output Destination Control (File Only)\n";
    spectra::instrumentation::Instrumentation::setOutputDestinations(
        spectra::instrumentation::E_LogOutput::FILE,
        spectra::instrumentation::E_LogComponent::MATH
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::INFO,
        "TestComponent",
        "TestSubComponent",
        "This INFO log should only go to file",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console: No new log should appear. Check math_log.txt: INFO log should be added.\n\n";

    // Test 5: Log History
    std::cout << "Test 5: Log History\n";
    spectra::instrumentation::Instrumentation::setOutputDestinations(
        spectra::instrumentation::E_LogOutput::CONSOLE | spectra::instrumentation::E_LogOutput::FILE,
        spectra::instrumentation::E_LogComponent::MATH
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::INFO,
        "TestComponent",
        "TestSubComponent",
        "Log 1 for history",
        spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::WARNING,
        "TestComponent",
        "TestSubComponent",
        "Log 2 for history",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console and math_log.txt: Both logs should appear.\n";
    std::cout << "Log history should contain both logs (verify via debugger or add a getter if you�re feeling fancy).\n\n";

    // Test 6: Error Handling with LoggedRuntimeError
    std::cout << "Test 6: Error Handling with LoggedRuntimeError\n";
    try {
        spectra::instrumentation::Instrumentation::log(
            spectra::instrumentation::E_LogLevel::ERROR,
            "TestComponent",
            "TestSubComponent",
            "This ERROR log should throw an exception",
			spectra::instrumentation::E_LogComponent::MATH, LOCATION
        );
    }
    catch (const spectra::instrumentation::LoggedRuntimeError& e) {
        std::cout << "Caught LoggedRuntimeError:\n" << e.getFullMessage() << "\n";
    }
    std::cout << "Check the exception message: It should include the error log and history.\n\n";

    // Test 7: Thread Safety (Basic)
    std::cout << "Test 7: Thread Safety (Basic)\n";
    auto logThread = [](int threadId) {
        for (int i = 0; i < 5; ++i) {
            spectra::instrumentation::Instrumentation::log(
                spectra::instrumentation::E_LogLevel::INFO,
                "ThreadTest",
                "Thread" + std::to_string(threadId),
                "Log from thread " + std::to_string(threadId) + " #" + std::to_string(i),
                spectra::instrumentation::E_LogComponent::MATH, LOCATION
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate some chaos
        }
        };
    std::thread t1(logThread, 1);
    std::thread t2(logThread, 2);
    t1.join();
    t2.join();
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
    std::cout << "Check console and math_log.txt: Expect 10 logs (5 from each thread) with no gibberish.\n\n";

    // Test 8: Benchmark Logger (because why not?)
    std::cout << "Test 8: Benchmark Logger\n";
    spectra::instrumentation::Instrumentation::setOutputDestinations(
        spectra::instrumentation::E_LogOutput::CONSOLE | spectra::instrumentation::E_LogOutput::FILE,
        spectra::instrumentation::E_LogComponent::BENCHMARK
    );
    spectra::instrumentation::Instrumentation::log(
        spectra::instrumentation::E_LogLevel::INFO,
        "BenchComp",
        "BenchSub",
        "Benchmarking something cool",
		spectra::instrumentation::E_LogComponent::BENCHMARK,
		1337, LOCATION
    );
    spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::BENCHMARK);
    std::cout << "Check console and benchmark_log.txt: INFO log with 1337 should appear.\n\n";

    std::cout << "=== Manual Tests Complete ===\n";
    std::cout << "Verify the output in console, math_log.txt, and benchmark_log.txt.\n";

    return 0;
}