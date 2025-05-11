#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "PlatformHandles.h"
#include "SpectraCore.h"
#include "ThreadFactory.h"
#include "ThreadSafePriorityQueue.h"
#include <shared_mutex>


namespace spectra::core::concurrent {
	// =============================================
	// Constants and Type Aliases
	// =============================================

	constexpr size_t CACHE_LINE_SIZE = 64;
	using TaskID = std::uint64_t;
	using WorkerID = std::uint64_t;
	using Nanoseconds = std::chrono::nanoseconds;
	using SteadyClock = std::chrono::steady_clock;
	using TimePoint = SteadyClock::time_point;

	// =============================================
	// Task and Work Management Types
	// =============================================

	enum class TaskState : uint8_t {
		Pending,
		Running,
		Completed,
		Cancelled,
		Failed,
		Unknown
	};

	enum class WorkerState : uint8_t {
		Idle,
		Active,
		Paused,
		Terminated
	};

	struct SPECTRA_CORE TaskHandle {
		TaskID id = 0;
		std::atomic<bool> isCancelled{ false };
		std::atomic<bool> forceCancel{ false };

		explicit operator bool() const noexcept {
			return id != 0;
		}

		TaskHandle(size_t id, bool cancelled) : id(id), isCancelled(cancelled) {}

		TaskHandle(TaskHandle&& other) noexcept {
			id = other.id;
			isCancelled = other.isCancelled.load();
			forceCancel = other.forceCancel.load();
		}

		TaskHandle& operator=(TaskHandle&& other) noexcept {
			id = other.id;
			isCancelled = other.isCancelled.load();
			forceCancel = other.forceCancel.load();
			return *this;
		}
	};

	struct SPECTRA_CORE alignas(CACHE_LINE_SIZE) WorkerHandle {
		WorkerID workerId = 0;         // Who am I?
		int coreId = -1;               // Where I live (core)
		int numaNode = -1;             // My NUMA hood
		std::atomic<size_t> activeTasks{ 0 };  // How busy I am
		std::atomic<size_t> completedTasks{ 0 }; // My brag sheet

		WorkerHandle() = default;
		WorkerHandle(WorkerID id, int core, int numa, size_t at, size_t ct)
			: workerId(id), coreId(core), numaNode(numa), activeTasks(at), completedTasks(ct) {
		}
	};

	struct SPECTRA_CORE TaskOptions {
		int priority = 0;
		std::optional<TimePoint> startAt;
		bool allowStealing = true;

		//Affinity
		int coreAffinity = -1;
		int numaNodeAffinity = -1;
		bool inheritCallerAffinity = false;
	};

	struct TaskEntry {
		std::shared_ptr<TaskHandle> handle;
		std::function<void()> task;
		int priority;

		TaskEntry(std::shared_ptr<TaskHandle> h, std::function<void()> t, int p)
			: handle(std::move(h)), task(std::move(t)), priority(p) {
		}

		bool operator>(const TaskEntry& rhs) const noexcept {
			return priority > rhs.priority;
		}
		bool operator<(const TaskEntry& rhs) const noexcept {
			return priority < rhs.priority;
		}
	};

	// =============================================
	// Base Executor Interfaces
	// =============================================

	class SPECTRA_CORE ThreadExecutor {
	public:
		virtual ~ThreadExecutor() = default;

		virtual void execute(std::function<void()> task) = 0;

		ThreadExecutor(const ThreadExecutor&) = delete;
		ThreadExecutor& operator=(const ThreadExecutor&) = delete;

		ThreadExecutor(ThreadExecutor&&) = default;
		ThreadExecutor& operator=(ThreadExecutor&&) = default;

	protected:
		ThreadExecutor() = default;
	};

	class SPECTRA_CORE ThreadExecutorService : public ThreadExecutor {
	public:
		~ThreadExecutorService() override = default;

		virtual std::shared_ptr<TaskHandle> submit(std::function<void()> task,
			const TaskOptions& options = {}) = 0;

		virtual bool cancel(TaskHandle& handle) = 0;
		virtual TaskState getTaskState(TaskHandle& handle) const = 0;

		virtual void shutdown() = 0;
		virtual void shutdownNow() = 0;
		virtual bool awaitTermination(Nanoseconds timeout = Nanoseconds::max()) = 0;

		virtual bool isRunning() const noexcept = 0;
		virtual bool isShutdown() const noexcept = 0;
		virtual bool isTerminated() const noexcept = 0;
		virtual size_t getActiveTaskCount() const noexcept = 0;
		virtual size_t getCompletedTaskCount() const noexcept = 0;

		virtual std::vector<std::shared_ptr<TaskHandle>> submitBatch(std::vector<std::function<void()>> tasks, 
			const std::vector<TaskOptions>& options) = 0;

		ThreadExecutorService(const ThreadExecutorService&) = delete;
		ThreadExecutorService& operator=(const ThreadExecutorService&) = delete;

		ThreadExecutorService(ThreadExecutorService&&) = default;
		ThreadExecutorService& operator=(ThreadExecutorService&&) = default;

	protected:
		ThreadExecutorService() = default;

		virtual TaskID generateTaskId() noexcept {
			static std::atomic<TaskID> nextId{ 1 };
			return nextId.fetch_add(1, std::memory_order_relaxed);
		}
	};

	// =============================================
	// MACROS 
	// =============================================
	//TODO
	//


	// =============================================
	// Thread Pool Implementations Encased in an Executor
	// =============================================

	class SPECTRA_CORE ThreadPoolExecutor {
	public:
		class SPECTRA_CORE DefaultThreadPool final : public ThreadExecutorService {
		public:
			DefaultThreadPool(size_t numThreads, CPUThreadFactory& factory);
			~DefaultThreadPool() override;

			// ThreadExecutorService overrides via Impl
			void execute(std::function<void()> task) override;
			std::shared_ptr<TaskHandle> submit(std::function<void()> task, const TaskOptions& options = {}) override;
			std::vector<std::shared_ptr<TaskHandle>> submitBatch(std::vector<std::function<void()>> tasks, const std::vector<TaskOptions>& options) override;
			bool cancel(TaskHandle& handle) override;
			TaskState getTaskState(TaskHandle& handle) const override;
			void shutdown() override;
			void shutdownNow() override;
			bool awaitTermination(Nanoseconds timeout = Nanoseconds::max()) override;
			bool isRunning() const noexcept override;
			bool isShutdown() const noexcept override;
			bool isTerminated() const noexcept override;
			size_t getActiveTaskCount() const noexcept override;
			size_t getCompletedTaskCount() const noexcept override;

		private:
			struct Worker {
				alignas(CACHE_LINE_SIZE)
					WorkerHandle handle;              // ID, core, NUMA, counters
				std::condition_variable cv;           // Nap time trigger
				std::mutex mutex;                     // Queue guard
				ThreadSafePriorityQueue<TaskEntry> queue; // Task stash
				std::unordered_map<TaskID, TaskState> taskStates; // Task tracker
				WorkerState state = WorkerState::Idle; // Nap or grind?

				Worker(int core, int numa) : handle{ 0, core, numa, 0, 0 } {}
				Worker() = default;
			};

			void workerFunction(size_t workerIndex) const;
			size_t findLeastBusyWorker() const;
			size_t selectWorkerByNumaNode(int numaNode) const;
			void buildNumaWorkerMap();
			TaskID generateTaskId() noexcept override;

			// Core state
			std::vector<std::unique_ptr<Worker>> workers_;
			std::vector<std::pair<WorkerID, handles::THREAD_VARIANT>> threads_;
			std::unordered_map<int, std::vector<size_t>> numaToWorkers_; // NUMA node -> worker indices
			mutable std::shared_mutex numaMapMutex_; // NUMA map guard
			std::atomic<bool> isRunning_{ true };      //Still kicking?
			std::atomic<bool> isShutdown_{ false };    //Polite shutdown?
			std::atomic<bool> isTerminated_{ false };  //All done?
			std::atomic<size_t> activeWorkers_{ 0 };   //Who is awake?
			std::atomic<uint64_t> taskIdCounter_{ 0 }; //Task ID counter
			std::mutex shutdownMutex_;                 //Shutdown guard
			std::condition_variable shutdownCV_;       //Shutdown trigger
		};
	};
} 