#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
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
	using Duration = SteadyClock::duration;

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

	// =============================================
	// Thread Handles
	// =============================================

	struct SPECTRA_CORE IHandle {
	protected:

		TaskID id = 0;
		std::atomic<bool> isCancelled{ false };
		std::shared_ptr<TaskState> taskState;
	public:
		virtual ~IHandle() = default;
		virtual explicit operator bool() const noexcept = 0;
		IHandle(size_t id, bool cancelled) : id(id), isCancelled(cancelled),
			taskState(std::make_shared<TaskState>(TaskState::Unknown)) {
		}

		IHandle(const IHandle&) = delete;
		IHandle& operator=(const IHandle&) = delete;

		IHandle(IHandle&& other) noexcept {
			id = other.id;
			isCancelled.store(other.isCancelled.load());
		}

		IHandle& operator=(IHandle&& other) noexcept {
			id = other.id;
			isCancelled.store(other.isCancelled.load());
			return *this;
		}

		TaskID getId() const noexcept {
			return id;
		}

		bool getIsCancelled() const noexcept {
			return isCancelled.load();
		}

		void setCancelled(bool cancelled) noexcept {
			isCancelled.store(cancelled);
		}
	};

	struct SPECTRA_CORE ActionHandle final : IHandle {
		ActionHandle(const ActionHandle&) = delete;
		ActionHandle& operator=(const ActionHandle&) = delete;

		explicit operator bool() const noexcept override {
			return id != 0;
		}

		~ActionHandle() override = default;
		ActionHandle(size_t id, bool cancelled) : IHandle(id, cancelled) {}
		ActionHandle(ActionHandle&& other) noexcept : IHandle(std::move(other)) {}

		ActionHandle& operator=(ActionHandle&& other) noexcept {
			if (this != &other) {
				IHandle::operator=(std::move(other));
			}
			return *this;
		}
	};

	struct SPECTRA_CORE TaskHandle final : IHandle {
		std::any result;
		TaskHandle(const TaskHandle&) = delete;
		TaskHandle& operator=(const TaskHandle&) = delete;

		explicit operator bool() const noexcept override {
			return id != 0;
		}
		~TaskHandle() override = default;
		TaskHandle(size_t iD, bool cancelled, std::any res) : IHandle(iD, cancelled), result(res) {}

		TaskHandle(TaskHandle&& other) noexcept : IHandle(std::move(other)), result(std::move(other.result)) {}

		TaskHandle& operator=(TaskHandle&& other) noexcept {
			if (this != &other) {
				result = std::move(other.result);
				IHandle::operator=(std::move(other));
			}
			return *this;
		}

		template<typename T >
		T* getResult() {
			if (*taskState == TaskState::Completed) return std::any_cast<T>(result);
			return nullptr;
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
		bool allowStealing = true;
		bool isCallable = false;
		bool isScheduled = false;
		bool isRepeatable = false;

		//Affinity
		int coreAffinity = -1;
		int numaNodeAffinity = -1;
		bool inheritCallerAffinity = false;
		Duration delay = Duration::zero();
		unsigned int repeat = 0; //  0 for infinite times, > 0 for specific times
	};

	using TASK_VARIANT = std::variant<std::function<void(std::any)>, std::function<void()>>;

	struct TaskEntry {
		std::shared_ptr<IHandle> handle;
		TASK_VARIANT task;
		int priority;
		std::any params;

		TaskEntry(std::shared_ptr<IHandle> h, TASK_VARIANT t, int p, const std::any&& params = nullptr)
			: handle(std::move(h)), task(std::move(t)), priority(p), params(params) {
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

		virtual std::shared_ptr<IHandle> submit(std::function<void()> task,
			const TaskOptions& options = {}) = 0;

		virtual bool cancel(IHandle& handle) = 0;
		virtual TaskState getTaskState(IHandle& handle) const = 0;

		virtual void shutdown() = 0;
		virtual void shutdownNow() = 0;
		virtual bool awaitTermination(Nanoseconds timeout = Nanoseconds::max()) = 0;

		[[nodiscard]] virtual bool isRunning() const noexcept = 0;
		[[nodiscard]] virtual bool isShutdown() const noexcept = 0;
		[[nodiscard]] virtual bool isTerminated() const noexcept = 0;
		[[nodiscard]] virtual size_t getActiveTaskCount() const noexcept = 0;
		[[nodiscard]] virtual size_t getCompletedTaskCount() const noexcept = 0;

		virtual std::vector<std::shared_ptr<IHandle>> submitBatch(std::vector<std::function<void()>> tasks,
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

		virtual std::shared_ptr<IHandle> submitCallable(std::function<std::any()> task,
			const TaskOptions& options = {}) = 0;
		virtual std::vector<std::shared_ptr<IHandle>> submitBatchCallable(std::vector<std::function<std::any()>> tasks,
			std::vector<TaskOptions>& options) = 0;

	};

	class SPECTRA_CORE ScheduledThreadExecutorService : public ThreadExecutor {
	public:
		~ScheduledThreadExecutorService() override = default;
		virtual std::shared_ptr<IHandle> schedule(std::function<void()> task,
			const TaskOptions& options) = 0;

		virtual bool cancel(IHandle& handle) = 0;
		virtual bool delayAndCancel(IHandle& handle, Duration duration) = 0;
		virtual TaskState getTaskState(IHandle& handle) const = 0;

		virtual void shutdown() = 0;
		virtual void delayAndShutdown(Duration duration) = 0;
		virtual void shutdownNow() = 0;
		virtual bool awaitTermination(Nanoseconds timeout = Nanoseconds::max()) = 0;

		[[nodiscard]] virtual bool isRunning() const noexcept = 0;
		[[nodiscard]] virtual bool isShutdown() const noexcept = 0;
		[[nodiscard]] virtual bool isTerminated() const noexcept = 0;
		[[nodiscard]] virtual size_t getActiveTaskCount() const noexcept = 0;
		[[nodiscard]] virtual size_t getCompletedTaskCount() const noexcept = 0;

		virtual std::vector<std::shared_ptr<IHandle>> scheduleBatch(std::vector<std::function<void()>> tasks,
			const std::vector<TaskOptions>& options) = 0;

		ScheduledThreadExecutorService(const ScheduledThreadExecutorService&) = delete;
		ScheduledThreadExecutorService& operator=(const ScheduledThreadExecutorService&) = delete;

		ScheduledThreadExecutorService(ScheduledThreadExecutorService&&) = default;
		ScheduledThreadExecutorService& operator=(ScheduledThreadExecutorService&&) = default;

	protected:
		ScheduledThreadExecutorService() = default;

		virtual TaskID generateTaskId() noexcept {
			static std::atomic<TaskID> nextId{ 1 };
			return nextId.fetch_add(1, std::memory_order_relaxed);
		}

		virtual std::shared_ptr<IHandle> scheduleCallable(std::function<std::any()> task,
			const TaskOptions& options = {}) = 0;
		virtual std::vector<std::shared_ptr<IHandle>> scheduleCallableBatch(std::vector<std::function<std::any()>> tasks,
			std::vector<TaskOptions>& options) = 0;
	};

	// =============================================
	// MACROS
	// =============================================
	//TODO
	//

	// =============================================
	// Thread Pool Implementations
	// =============================================

	class SPECTRA_CORE DefaultThreadPool final : public ThreadExecutorService {
	public:
		DefaultThreadPool(size_t numThreads, CPUThreadFactory& factory);
		~DefaultThreadPool() override;

		// ThreadExecutorService overrides via Impl
		void execute(std::function<void()> task) override;
		std::shared_ptr<IHandle> submit(std::function<void()> task, const TaskOptions& options = {}) override;
		std::vector<std::shared_ptr<IHandle>> submitBatch(std::vector<std::function<void()>> tasks, const std::vector<TaskOptions>& options) override;

		template<typename T>
		std::shared_ptr<IHandle> submit(std::function<T()> task, const TaskOptions& options = {}) {
			auto handle = std::make_shared<TaskHandle<T>>(generateTaskId(), false, T{});
			return submitCallable(task, options);
		}

		template<typename T >
		std::shared_ptr<IHandle> submitBatch(std::vector<std::function<T()>> tasks,
			std::vector<TaskOptions>& options) {
			return submitBatchCallable(tasks, options);
		}

		bool cancel(IHandle& handle) override;
		TaskState getTaskState(IHandle& handle) const override;
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
			std::unordered_map<TaskID, std::weak_ptr<TaskState>> taskStates; // Task tracker
			WorkerState state = WorkerState::Idle; // Nap or grind?

			Worker(int core, int numa) : handle{ 0, core, numa, 0, 0 } {}
			Worker() = default;
		};

		void workerFunction(size_t workerIndex) const;
		size_t findLeastBusyWorker() const;
		size_t selectWorkerByNumaNode(int numaNode) const;
		void buildNumaWorkerMap();
		TaskID generateTaskId() noexcept override;
		
		std::shared_ptr<IHandle> submitCallable(std::function<std::any()> task, const TaskOptions& options = {}) override;
		std::vector<std::shared_ptr<IHandle>> submitBatchCallable(std::vector<std::function<std::any()>> tasks,
			std::vector<TaskOptions>& options) override;

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
}