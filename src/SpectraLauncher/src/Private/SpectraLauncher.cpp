#include <atomic>
#include <atomic>

#include "SpectraInstrumentation.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <random>
#include <vector>
#include <atomic>
#include "ThreadPool.h"  // Your header
#include "ThreadFactory.h"  // Your header
using namespace spectra::core::concurrent;

void loggingTests() {
	spectra::instrumentation::Instrumentation::enableColoredConsole(true, spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "=== Starting Manual Tests for SpectraInstrumentation ===\n\n";

	// Test 1: Basic Log Creation and Formatting
	std::cout << "Test 1: Basic Log Creation and Formatting\n";
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::INFO_,
		"TestComponent",
		"TestSubComponent",
		"This is a basic log message",
		spectra::instrumentation::E_LogComponent::MATH,
		42, "Hello", nullptr, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "Check console and math_log.txt for a formatted INFO_ log with args (42, Hello, null).\n\n";

	// Test 2: Log Level Filtering
	std::cout << "Test 2: Log Level Filtering\n";
	spectra::instrumentation::Instrumentation::setMinLevel(
		spectra::instrumentation::E_LogLevel::WARNING_,
		spectra::instrumentation::E_LogComponent::MATH
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::INFO_,
		"TestComponent",
		"TestSubComponent",
		"This INFO_ log should be ignored due to min level WARNING_",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::WARNING_,
		"TestComponent",
		"TestSubComponent",
		"This WARNING_ log should be recorded",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "Check console and math_log.txt: Only the WARNING_ log should appear.\n\n";

	// Test 3: Output Destination Control (Console Only)
	std::cout << "Test 3: Output Destination Control (Console Only)\n";
	spectra::instrumentation::Instrumentation::setOutputDestinations(
		spectra::instrumentation::E_LogOutput::CONSOLE,
		spectra::instrumentation::E_LogComponent::MATH
	);
	spectra::instrumentation::Instrumentation::setMinLevel(
		spectra::instrumentation::E_LogLevel::DEBUG_,
		spectra::instrumentation::E_LogComponent::MATH
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::DEBUG_,
		"TestComponent",
		"TestSubComponent",
		"This DEBUG_ log should only go to console",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "Check console: DEBUG_ log should appear. Check math_log.txt: No new log should be added.\n\n";

	// Test 4: Output Destination Control (File Only)
	std::cout << "Test 4: Output Destination Control (File Only)\n";
	spectra::instrumentation::Instrumentation::setOutputDestinations(
		spectra::instrumentation::E_LogOutput::FILE,
		spectra::instrumentation::E_LogComponent::MATH
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::INFO_,
		"TestComponent",
		"TestSubComponent",
		"This INFO_ log should only go to file",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "Check console: No new log should appear. Check math_log.txt: INFO_ log should be added.\n\n";

	// Test 5: Log History
	std::cout << "Test 5: Log History\n";
	spectra::instrumentation::Instrumentation::setOutputDestinations(
		spectra::instrumentation::E_LogOutput::CONSOLE | spectra::instrumentation::E_LogOutput::FILE,
		spectra::instrumentation::E_LogComponent::MATH
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::INFO_,
		"TestComponent",
		"TestSubComponent",
		"Log 1 for history",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::log(
		spectra::instrumentation::E_LogLevel::WARNING_,
		"TestComponent",
		"TestSubComponent",
		"Log 2 for history",
		spectra::instrumentation::E_LogComponent::MATH, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::MATH);
	std::cout << "Check console and math_log.txt: Both logs should appear.\n";
	std::cout << "Log history should contain both logs (verify via debugger or add a getter if you’re feeling fancy).\n\n";

	// Test 6: Error Handling with LoggedRuntimeError
	std::cout << "Test 6: Error Handling with LoggedRuntimeError\n";
	try {
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR_,
			"TestComponent",
			"TestSubComponent",
			"This ERROR_ log should throw an exception",
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
				spectra::instrumentation::E_LogLevel::INFO_,
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
		spectra::instrumentation::E_LogLevel::INFO_,
		"BenchComp",
		"BenchSub",
		"Benchmarking something cool",
		spectra::instrumentation::E_LogComponent::BENCHMARK,
		1337, LOCATION
	);
	spectra::instrumentation::Instrumentation::synchronousFlush(spectra::instrumentation::E_LogComponent::BENCHMARK);
	std::cout << "Check console and benchmark_log.txt: INFO_ log with 1337 should appear.\n\n";

	std::cout << "=== Manual Tests Complete ===\n";
	std::cout << "Verify the output in console, math_log.txt, and benchmark_log.txt.\n";
}

using namespace spectra::core::concurrent;
using namespace std::chrono_literals;

// Quick helper to sleep and print
void sleepAndPrint(const std::string& msg, std::chrono::milliseconds duration = 100ms) {
	std::this_thread::sleep_for(duration);
	std::cout << msg << std::endl;
}

// Factory setup (assuming CPUThreadFactory works without NUMA)
DefaultCPUThreadFactory factory; // Adjust if your factory needs params

// Test 1: Basic Single Task Execution
void testBasicSingleTask() {
	ThreadPoolExecutor::DefaultThreadPool pool(2, factory);
	auto handle = pool.submit([]() {
		sleepAndPrint("Task running: I’m alive!");
		});
	sleepAndPrint("Submitted task with ID: " + std::to_string(handle->id));
	std::this_thread::sleep_for(200ms); // Let it finish
	auto state = pool.getTaskState(*handle);
	std::cout << "Task state: " << static_cast<int>(state) << " (0=Pending, 1=Running, 2=Completed)\n";
	// Expect: Task runs, state = Completed (2)
	std::cout << "Sassy check: Did this little task strut its stuff? Should say Completed!\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 2: Priority Sorting
void testPrioritySorting() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory); // Single worker for order
	TaskOptions lowPrio{ 10 };  // Low urgency
	TaskOptions highPrio{ 1 };  // High urgency
	auto h1 = pool.submit([]() { sleepAndPrint("Low priority task (should be last)"); }, lowPrio);
	auto h2 = pool.submit([]() { sleepAndPrint("High priority task (should be first)"); }, highPrio);
	sleepAndPrint("Submitted tasks: Low ID=" + std::to_string(h1->id) + ", High ID=" + std::to_string(h2->id));
	std::this_thread::sleep_for(300ms);
	// Expect: High priority (1) prints first, then Low priority (10)
	std::cout << "Sassy check: VIPs first, peasants last—did High beat Low?\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 3: External Cancellation Before Execution
void testExternalCancellation() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory);
	std::shared_ptr<ActionHandle> handle = std::make_shared<ActionHandle>(0, false);
	handle  = pool.submit([&handle]() {
		std::this_thread::sleep_for(1s); // Simulate long task
		if (handle->isCancelled) {
			std::cout << "Task cancelled externally, not running!\n";
			return;
		}
		sleepAndPrint("Task running: I should NOT see this!", 600ms);
		});
	sleepAndPrint("Submitted task ID: " + std::to_string(handle->id));
	handle->isCancelled.store(true); // External cancel
	sleepAndPrint("Cancelled task externally");
	std::this_thread::sleep_for(100ms);
	auto state = pool.getTaskState(*handle);
	std::cout << "Task state: " << static_cast<int>(state) << " (3=Cancelled)\n";
	// Expect: Task doesn’t run, state = Cancelled (3)
	std::cout << "Sassy check: Task got the boot—should be Cancelled, not a peep!\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 4: Internal Cancellation via `cancel`
void testInternalCancellation() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory);
	auto handle = std::make_shared<ActionHandle>(0, false);
	handle = pool.submit([&handle]() {
		std::this_thread::sleep_for(1s); // Simulate long task
		if (handle->isCancelled) {
			std::cout << "Task cancelled internally, not running!\n";
			return;
		}
		sleepAndPrint("Task running: I should NOT see this!");
		});
	sleepAndPrint("Submitted task ID: " + std::to_string(handle->id));
	bool cancelled = pool.cancel(*handle);
	sleepAndPrint("Cancel result: " + std::string(cancelled ? "true" : "false"));
	std::this_thread::sleep_for(200ms);
	auto state = pool.getTaskState(*handle);
	std::cout << "Task state: " << static_cast<int>(state) << " (3=Cancelled)\n";
	// Expect: Cancelled = true, task doesn’t run, state = Cancelled (3)
	std::cout << "Sassy check: Pool said ‘You’re outta here!’—should be Cancelled!\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 5: Batch Submission with Mixed Priorities
void testBatchWithPriorities() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory);
	std::vector<std::function<void()>> tasks = {
		[]() { sleepAndPrint("Task 1: Priority 5"); },
		[]() { sleepAndPrint("Task 2: Priority 1"); },
		[]() { sleepAndPrint("Task 3: Priority 10"); }
	};
	std::vector<TaskOptions> options = {{5}, {10}, {1}};
	auto handles = pool.submitBatch(std::move(tasks),options);
	sleepAndPrint("Submitted batch with IDs: " + std::to_string(handles[0]->id) + ", " +
		std::to_string(handles[1]->id) + ", " + std::to_string(handles[2]->id));
	std::this_thread::sleep_for(400ms);
	// Expect: Order: Task 2 (1), Task 1 (5), Task 3 (10)
	std::cout << "Sassy check: Batch priorities—did Task 2 cut the line?\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 6: Worker Distribution (No NUMA)
void testWorkerDistribution() {
	ThreadPoolExecutor::DefaultThreadPool pool(2, factory);
	auto h1 = pool.submit([]() { sleepAndPrint("Task 1 on some worker"); });
	auto h2 = pool.submit([]() { sleepAndPrint("Task 2 on some worker"); });
	sleepAndPrint("Submitted tasks: " + std::to_string(h1->id) + ", " + std::to_string(h2->id));
	std::this_thread::sleep_for(200ms);
	std::cout << "Active tasks: " << pool.getActiveTaskCount() << "\n";
	std::cout << "Completed tasks: " << pool.getCompletedTaskCount() << "\n";
	// Expect: Tasks split across 2 workers (round-robin), both complete
	std::cout << "Sassy check: No NUMA, just vibes—did both workers get a workout?\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 7: Shutdown Graceful
void testGracefulShutdown() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory);
	auto h1 = pool.submit([]() { sleepAndPrint("Task 1 running"); });
	auto h2 = pool.submit([]() { sleepAndPrint("Task 2 running"); });
	sleepAndPrint("Submitted tasks: " + std::to_string(h1->id) + ", " + std::to_string(h2->id));
	pool.shutdown();
	sleepAndPrint("Shutdown initiated—letting tasks finish");
	pool.awaitTermination(500ms);
	std::cout << "Pool terminated: " << (pool.isTerminated() ? "Yes" : "No") << "\n";
	// Expect: Both tasks run, then pool terminates
	std::cout << "Sassy check: ‘Finish up and get out!’—did they all wrap up?\n";
}

// Test 8: Shutdown Now with Cancellation
void testShutdownNow() {
	ThreadPoolExecutor::DefaultThreadPool pool(1, factory);
	auto h1 = pool.submit([]() { std::this_thread::sleep_for(1s); sleepAndPrint("Task 1 (shouldn’t finish)"); });
	auto h2 = pool.submit([]() { sleepAndPrint("Task 2 (won’t run)"); });
	sleepAndPrint("Submitted tasks: " + std::to_string(h1->id) + ", " + std::to_string(h2->id));
	std::this_thread::sleep_for(50ms); // Let one start
	pool.shutdownNow();
	sleepAndPrint("ShutdownNow called—killing everything");
	pool.awaitTermination(200ms);
	std::cout << "Task 1 state: " << static_cast<int>(pool.getTaskState(*h1)) << "\n";
	std::cout << "Task 2 state: " << static_cast<int>(pool.getTaskState(*h2)) << "\n";
	// Expect: Task 1 might run or cancel, Task 2 cancels, states = Cancelled (3)
	std::cout << "Sassy check: Pool yanked the plug—did tasks get the axe?\n";

}

// Test 9: Active and Completed Task Counts
void testTaskCounts() {
	ThreadPoolExecutor::DefaultThreadPool pool(2, factory);
	auto h1 = pool.submit([]() { std::this_thread::sleep_for(200ms); sleepAndPrint("Task 1 running"); });
	auto h2 = pool.submit([]() { sleepAndPrint("Task 2 running"); });
	sleepAndPrint("Submitted tasks: " + std::to_string(h1->id) + ", " + std::to_string(h2->id));
	std::this_thread::sleep_for(100ms);
	std::cout << "Active tasks: " << pool.getActiveTaskCount() << "\n";
	std::this_thread::sleep_for(200ms);
	std::cout << "Completed tasks: " << pool.getCompletedTaskCount() << "\n";
	// Expect: Active peaks at 1-2, Completed = 2 after finish
	std::cout << "Sassy check: Counting tasks like a bouncer—did we tally right?\n";
	pool.shutdown();
	pool.awaitTermination();
}

// Test 10: Stress Test with Batch
void testStressBatch() {
	ThreadPoolExecutor::DefaultThreadPool pool(4, factory);
	std::vector<std::function<void()>> tasks(100);
	std::vector<TaskOptions> options(100);
	for (int i = 0; i < 100; ++i) {
		tasks[i] = [i]() { sleepAndPrint("Task " + std::to_string(i) + " running"); };
		options[i] = { 1 }; // All same priority
	}
	auto handles = pool.submitBatch(std::move(tasks), options);
	sleepAndPrint("Submitted 100 tasks—buckle up!");
	std::this_thread::sleep_for(4s);
	size_t completed = pool.getCompletedTaskCount();
	std::cout << "Completed tasks: " << completed << " (expect ~100)\n";
	// Expect: All 100 tasks run across 4 workers
	std::cout << "Sassy check: 100 tasks stormed the pool—did it survive the chaos?\n";
	pool.shutdown();
	pool.awaitTermination();
}

int main() {
	// Uncomment one test to run manually
	std::cout << "Single task run\n";
	testBasicSingleTask();
	std::cout << "\n \nPriority Sorting\n";
	testPrioritySorting();
	std::cout << "\n \nExternal Cancellation\n";
	testExternalCancellation();
	std::cout << "\n \nInternal Cancellation\n";
	testInternalCancellation();
	std::cout << "\n \nBatch Runs with Priorities\n";
	testBatchWithPriorities();
	std::cout << "\n \nWorker Distribution";
	testWorkerDistribution();
	std::cout << "\n \nGraceful Shutdown\n";
	testGracefulShutdown();
	std::cout << "\n \nForced Shutdown\n";
	testShutdownNow();
	std::cout << "\n \nCounting tasks\n";
	testTaskCounts();
	std::cout << "\n \nBatch runs\n";
	testStressBatch();

	std::cout << "Pick a test, ya threading thrill-seeker! Uncomment and run!\n";
	return 0;
}