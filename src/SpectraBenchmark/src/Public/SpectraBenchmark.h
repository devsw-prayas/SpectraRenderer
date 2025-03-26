#pragma once
#ifndef SPEC_BENCHMARK
#define SPEC_BENCHMARK __declspec(dllexport)
#endif
void SPEC_BENCHMARK Init();

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>

class SPEC_BENCHMARK BenchmarkOptions {
	size_t numThreads = std::thread::hardware_concurrency();
	int workSize = 1000000;
	int chunks = 10;
	bool readHeavy = false;
	bool writeHeavy = false;
	int iterations = 100;

public:
	BenchmarkOptions();

	void toggleReadHeavy(bool enable);
	void toggleWriteHeavy(bool enable);
	void setNumThreads(int threads);
	void setWorkSize(int size);
	void setChunks(int chunkCount);
	void setIterations(int iters);

	[[nodiscard]] size_t getNumThreads() const;
	[[nodiscard]] int getWorkSize() const;
	[[nodiscard]] int getChunks() const;
	[[nodiscard]] bool isReadHeavy() const;
	[[nodiscard]] bool isWriteHeavy() const;
	[[nodiscard]] int getIterations() const;

	void validate() const;
};

class SPEC_BENCHMARK Harness {
public:
	virtual void initialize() = 0;
	virtual void read() = 0;
	virtual void write() = 0;
	virtual void teardown() = 0;
	virtual ~Harness() = default;
};

class SPEC_BENCHMARK Benchmark {
	Harness& harness;
	BenchmarkOptions opts;
	std::mutex lockMutex;
	std::atomic<int> counter{ 0 };

	template<typename Func>
	double measureTime(Func&& func);

	void doWork();

public:
	Benchmark(Harness& h, BenchmarkOptions o);

	double runSingleThreaded();
	double runMultiThreaded();
	double runLocking();
	double runForkJoin();
	double runWorkSteal();
};