#pragma once
#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <Windows.h>

#include "SpectraPlatformRuntime.h"
#include <mutex>
#include <shared_mutex>

namespace spectra::runtime {
	enum class ThreadPriority : uint8_t {
		Lowest, BelowNormal, Normal, AboveNormal, Highest, TimeCritical, Idle
	};

	enum class ThreadState : uint8_t {
		Running, Terminated, Waiting, Suspended
	};

	inline int getThreadPriority(const ThreadPriority priority) {
		switch (priority) {
		case ThreadPriority::Lowest: return THREAD_PRIORITY_LOWEST;
		case ThreadPriority::BelowNormal: return THREAD_PRIORITY_BELOW_NORMAL;
		case ThreadPriority::AboveNormal: return THREAD_PRIORITY_ABOVE_NORMAL;
		case ThreadPriority::Normal: return THREAD_PRIORITY_NORMAL;
		case ThreadPriority::Highest: return THREAD_PRIORITY_HIGHEST;
		case ThreadPriority::TimeCritical: return THREAD_PRIORITY_TIME_CRITICAL;
		case ThreadPriority::Idle: return THREAD_PRIORITY_IDLE;
		}
		return INT_MIN;
	}

	struct SpectraPlatformRuntime SpectraThreadHandle {
		uint32_t id;
		uint32_t generation;
		bool operator==(const SpectraThreadHandle& other) const {
			return id == other.id && generation == other.generation;
		}
	};


	class SpectraPlatformRuntime SpectraThread  {
		friend class SpectraThreadManager;
		HANDLE win32Handle;
		DWORD threadID;
		std::string name;
		ThreadPriority priority;
		uint64_t affinityMask;
		ThreadState state;
		SpectraThreadHandle threadHandle;

	public:
		SpectraThread();
		~SpectraThread();

		SpectraThread(const SpectraThread&) = delete;
		SpectraThread(SpectraThread&& other) noexcept;
		SpectraThread& operator=(const SpectraThread&) = delete;
		SpectraThread& operator=(SpectraThread&& other) noexcept;

		[[nodiscard]] const std::string& getName() const { return name; }
		[[nodiscard]] ThreadPriority getPriority() const { return priority; }
		[[nodiscard]] ThreadState getState() const { return state; }
		[[nodiscard]] DWORD getOSID() const { return threadID; }
		[[nodiscard]] SpectraThreadHandle getHandle() const { return threadHandle; }

		struct Metadata {
			DWORD osThreadID;
			std::string name;
			ThreadPriority priority;
			ThreadState state;
			uint64_t affinity;
			SpectraThreadHandle handle;
		};

		[[nodiscard]] Metadata getMetadata() const {
			return {
				.osThreadID = threadID,
				.name = name,
				.priority = priority,
				.state = state,
				.affinity = affinityMask,
				.handle = threadHandle
			};
		}
	};

	class SpectraPlatformRuntime SpectraThreadManager {
		using EntryPoint = void* (*)(void*);

		std::vector<uint32_t> freeList;
		struct ThreadSlot {
			std::unique_ptr<SpectraThread> thread;
			uint32_t generation;
		};
		std::vector<ThreadSlot> activeThreads;
		std::mutex globalLock;
		std::shared_mutex lookupLock;


		void registerThread(const SpectraThread* thread);
		SpectraThread* deregisterThread(const SpectraThreadHandle& handle);

		SpectraThreadHandle createRuntimeThread(
			LPTHREAD_START_ROUTINE entryPointThunk,
			void* userData,
			std::string name,
			DWORD creationFlags,
			SIZE_T stackSize,
			ThreadPriority priority,
			std::optional<uint64_t> affinityMask
		);

	public:
		static SpectraThreadManager& getInstance() {
			static SpectraThreadManager instance;
			return instance;
		}

		template<typename Fn, typename...Args>
		[[nodiscard]] static SpectraThreadHandle createThread(
			ThreadPriority priority = ThreadPriority::Normal,
			long stackSize = 0L,
			uint64_t affinityMask = 0,
			Fn&& function,
			Args&& ...args);

		static const SpectraThread* get(SpectraThreadHandle handle);

		static void suspend(SpectraThreadHandle handle);
		static void resume(SpectraThreadHandle handle);
		static void terminate(SpectraThreadHandle handle);
		static void join(SpectraThreadHandle handle);

		static bool isAlive(SpectraThreadHandle handle);
		static bool isSuspended(SpectraThreadHandle handle);
		static DWORD getExitCode(SpectraThreadHandle handle);
		static std::string getName(SpectraThreadHandle handle);

		static ThreadPriority getPriority(SpectraThreadHandle handle);
		static void setPriority(SpectraThreadHandle handle, ThreadPriority priority);
		static uint32_t getAffinity(SpectraThreadHandle handle);
		static void setAffinity(SpectraThreadHandle handle, uint64_t affinityMask);

		static void setName(SpectraThreadHandle handle, std::string name);
		static void destroy(SpectraThreadHandle handle);
	};

	inline constexpr SpectraThreadHandle InvalidThreadHandle{ 0, 0 };


	namespace this_thread {
		inline thread_local SpectraThreadHandle gCurrentHandle;
		inline SpectraThreadHandle getCurrentThreadHandle() {
			return gCurrentHandle;
		}

		inline void yield() {
			::SwitchToThread();
		}

		inline void sleep(uint32_t milliseconds) {
			::Sleep(milliseconds);
		}

		template<typename Rep, typename Period>
		void sleepFor(std::chrono::duration<Rep, Period> duration) {
			sleep(
				std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
			);
		}

		template<typename Clock, typename Duration>
		void sleepUntil(std::chrono::time_point<Clock, Duration> timePoint) {
			sleepFor(timePoint - Clock::now());
		}

		inline void exit(uint32_t exitCode = 0) {
			if (gCurrentHandle.id != 0) {
				SpectraThreadManager::terminate(gCurrentHandle);
			}
			::ExitThread(exitCode);
		}

		inline DWORD getThreadID() {
			return getCurrentThreadHandle().id;
		}

		inline std::string getName() {
			return SpectraThreadManager::getInstance().getName(getCurrentThreadHandle());
		}

		inline void setPriority(ThreadPriority priority) {
			SpectraThreadManager::getInstance().setPriority(getCurrentThreadHandle(), priority);
		}

		inline void setName(const std::string& name) {
			SpectraThreadManager::getInstance().setName(getCurrentThreadHandle(), name);
		}

		inline void setAffinity(uint64_t affinityMask) {
			SpectraThreadManager::getInstance().setAffinity(getCurrentThreadHandle(), affinityMask);
		}

		inline void suspend() {
			SpectraThreadManager::getInstance().suspend(getCurrentThreadHandle());
		}

		inline void resume() {
			SpectraThreadManager::getInstance().resume(getCurrentThreadHandle());
		}

		inline void terminate() {
			SpectraThreadManager::getInstance().terminate(getCurrentThreadHandle());
		}

		inline bool isAlive() {
			return SpectraThreadManager::getInstance().isAlive(getCurrentThreadHandle());
		}

		inline bool isSuspended() {
			return SpectraThreadManager::getInstance().isSuspended(getCurrentThreadHandle());
		}

		inline DWORD getExitCode() {
			return SpectraThreadManager::getInstance().getExitCode(getCurrentThreadHandle());
		}

		inline ThreadPriority getPriority() {
			return SpectraThreadManager::getInstance().getPriority(getCurrentThreadHandle());
		}

		inline void join() {
			SpectraThreadManager::getInstance().join(getCurrentThreadHandle());
		}
	}
}


template<> struct std::hash<spectra::runtime::SpectraThreadHandle> {
	size_t operator()(const spectra::runtime::SpectraThreadHandle& h) const noexcept {
		return (size_t(h.id) << 1) ^ size_t(h.generation);
	}
};