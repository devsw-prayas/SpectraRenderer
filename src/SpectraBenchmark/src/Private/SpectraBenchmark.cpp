#include "SpectraBenchmark.h"
#include "SpectraInstrumentation.h"

void SPEC_BENCHMARK Init() {}

#include <chrono>
#include <vector>
#include <numeric>
#include <queue>

BenchmarkOptions::BenchmarkOptions() = default;

void BenchmarkOptions::toggleReadHeavy(bool enable) {
	readHeavy = enable;
	if (enable) writeHeavy = false; // No double-dipping, you hog
}

void BenchmarkOptions::toggleWriteHeavy(bool enable) {
	writeHeavy = enable;
	if (enable) readHeavy = false; // Pick a team, flip-flopper
}

void BenchmarkOptions::setNumThreads(int threads) {
	if (threads < 1)
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR,
			"SpectraBenchmark",
			"BenchmarkOptions",
			"Invalid number of threads: {}",
			spectra::instrumentation::E_LogComponent::BENCHMARK,
			threads
		);
	numThreads = threads;
}

void BenchmarkOptions::setWorkSize(int size) {
	if (size < 1)
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR,
			"SpectraBenchmark",
			"BenchmarkOptions",
			"Invalid work size: {}",
			spectra::instrumentation::E_LogComponent::BENCHMARK,
			size
		);
	workSize = size;
}

void BenchmarkOptions::setChunks(int chunkCount) {
	if (chunkCount < 1)
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR,
			"SpectraBenchmark",
			"BenchmarkOptions",
			"Invalid chunk count: {}",
			spectra::instrumentation::E_LogComponent::BENCHMARK,
			chunkCount
		);
	chunks = chunkCount;
}

void BenchmarkOptions::setIterations(int iters) {
	if (iters < 1)
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR,
			"SpectraBenchmark",
			"BenchmarkOptions",
			"Invalid iteration count: {}",
			spectra::instrumentation::E_LogComponent::BENCHMARK,
			iters
		);
	iterations = iters;
}

size_t BenchmarkOptions::getNumThreads() const { return numThreads; }
int BenchmarkOptions::getWorkSize() const { return workSize; }
int BenchmarkOptions::getChunks() const { return chunks; }
bool BenchmarkOptions::isReadHeavy() const { return readHeavy; }
bool BenchmarkOptions::isWriteHeavy() const { return writeHeavy; }
int BenchmarkOptions::getIterations() const { return iterations; }

void BenchmarkOptions::validate() const {
	if (readHeavy && writeHeavy)
		spectra::instrumentation::Instrumentation::log(
			spectra::instrumentation::E_LogLevel::ERROR,
			"SpectraBenchmark",
			"BenchmarkOptions",
			"Cannot be both read-heavy and write-heavy",
			spectra::instrumentation::E_LogComponent::BENCHMARK
		);
}

Benchmark::Benchmark(Harness& h, BenchmarkOptions o) : harness(h), opts(o) {
	opts.validate();
}

template<typename Func>
double Benchmark::measureTime(Func&& func) {
	std::vector<double> timings;
	timings.reserve(opts.getIterations());

	for (int i = 0; i < opts.getIterations(); ++i) {
		harness.initialize();
		auto start = std::chrono::high_resolution_clock::now();
		std::invoke(std::forward<Func>(func));
		auto end = std::chrono::high_resolution_clock::now();
		harness.teardown();
		double elapsed = std::chrono::duration<double, std::nano>(end - start).count();
		timings.push_back(elapsed);
	}

	double total = std::accumulate(timings.begin(), timings.end(), 0.0);
	return total / opts.getIterations();
}

void Benchmark::doWork() {
	if (opts.isReadHeavy()) harness.read();
	else if (opts.isWriteHeavy()) harness.write();
	else {
		harness.read();
		harness.write();
	}
}

double Benchmark::runSingleThreaded() {
	auto work = [&]() {
		for (int i = 0; i < opts.getWorkSize(); ++i) doWork();
		};
	return measureTime(work);
}

double Benchmark::runMultiThreaded() {
	auto work = [&]() {
		std::vector<std::thread> threads;
		for (int i = 0; i < opts.getNumThreads(); ++i) {
			threads.emplace_back([&]() {
				for (int j = 0; j < opts.getWorkSize() / opts.getNumThreads(); ++j) doWork();
				});
		}
		for (auto& t : threads) t.join();
		};
	return measureTime(work);
}

double Benchmark::runLocking() {
	auto work = [&]() {
		std::vector<std::thread> threads;
		counter = 0;
		for (int i = 0; i < opts.getNumThreads(); ++i) {
			threads.emplace_back([&]() {
				for (int j = 0; j < opts.getWorkSize() / opts.getNumThreads(); ++j) {
					std::lock_guard<std::mutex> lock(lockMutex);
					doWork();
					counter++;
				}
				});
		}
		for (auto& t : threads) t.join();
		};
	return measureTime(work);
}

double Benchmark::runForkJoin() {
	auto work = [&]() {
		int chunkSize = opts.getWorkSize() / opts.getChunks();
		std::vector<std::future<void>> futures;
		for (int i = 0; i < opts.getChunks(); ++i) {
			int start = i * chunkSize;
			int end = (i == opts.getChunks() - 1) ? opts.getWorkSize() : start + chunkSize;
			futures.push_back(std::async(std::launch::async, [&, start, end]() {
				for (int j = start; j < end; ++j) doWork();
				}));
		}
		for (auto& f : futures) f.get();
		};
	return measureTime(work);
}

double Benchmark::runWorkSteal() {
	std::queue<int> tasks;
	std::mutex queueMutex;
	std::condition_variable cv;
	std::atomic<bool> done{ false };

	auto worker = [&]() {
		while (!done) {
			int taskSize;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				if (tasks.empty()) return;
				taskSize = tasks.front();
				tasks.pop();
			}
			for (int i = 0; i < taskSize; ++i) doWork();
		}
		};

	auto work = [&]() {
		done = false;
		for (int i = 0; i < opts.getChunks(); ++i) {
			std::lock_guard<std::mutex> lock(queueMutex);
			tasks.push(opts.getWorkSize() / opts.getChunks());
		}
		std::vector<std::thread> threads;
		for (int i = 0; i < opts.getNumThreads(); ++i) {
			threads.emplace_back(worker);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		done = true;
		for (auto& t : threads) t.join();
		};
	return measureTime(work);
}