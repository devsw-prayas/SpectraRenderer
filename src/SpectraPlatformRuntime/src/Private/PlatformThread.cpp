#include <SpectraPlatformRuntime.h>
#include <PlatformThread.h>
#include <ThreadUtils.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#if defined(_MSC_VER)
// TODO : Have to move to CMake
#pragma comment(lib, "synchronization.lib")
#endif
#include <Windows.h>
#endif

namespace Spectra::Platform::Runtime::Thread {
	struct alignas(16) InvariantHandle final {
		HANDLE m_InternalHandle;
		Atomic::Atomic32 m_AccessCount;

		InvariantHandle() : m_InternalHandle(INVALID_HANDLE_VALUE), m_AccessCount(0) {}
		~InvariantHandle() = default;

		InvariantHandle(const InvariantHandle&) = default;
		InvariantHandle& operator=(const InvariantHandle&) = default;

		InvariantHandle(InvariantHandle&&) noexcept = default;
		InvariantHandle& operator=(InvariantHandle&&) noexcept = default;
	};

	struct alignas(64) SlotIdentity final {
		Atomic::Atomic64 m_SlotMask;
		size_t m_SlotGeneration;
		ThreadState m_State;

		SlotIdentity() : m_SlotMask(1LL << 0), m_SlotGeneration(0), m_State(ThreadState::REAPED) {}
		~SlotIdentity() = default;

		SlotIdentity(const SlotIdentity&) = default;
		SlotIdentity& operator=(const SlotIdentity&) = default;

		SlotIdentity(SlotIdentity&&) noexcept = default;
		SlotIdentity& operator=(SlotIdentity&&) noexcept = default;

		[[nodiscard]] size_t allocateToken() {
#if defined(_MSC_VER)
			for (;;) {
				uint64_t mask = m_SlotMask;

				uint64_t freeBits = ~mask;
				if (freeBits == 0) return static_cast<size_t>(-1);

				unsigned long slot;
				_BitScanForward64(&slot, freeBits);

				uint64_t bit = 1ull << slot;
				uint64_t expected = mask;
				uint64_t desired = mask | bit;

				if (m_SlotMask.compareAndSwap(desired, expected)) return slot;
			}
#else
			return static_cast<size_t>(-1);
#endif
		}
	};

	struct alignas(64) ThreadRegistry final {
		//TODO Will switch it with a StormSTL container
		std::vector<std::pair<InvariantHandle, SlotIdentity>> m_HandleRegistry;

		ThreadRegistry() {
			m_HandleRegistry.resize(SPECTRA_PLATFORM_MAX_THREADS);
		}

		~ThreadRegistry() {
			m_HandleRegistry.clear();
		}

		ThreadRegistry(const ThreadRegistry&) = default;
		ThreadRegistry& operator=(const ThreadRegistry&) = default;

		ThreadRegistry(ThreadRegistry&&) noexcept = default;
		ThreadRegistry& operator=(ThreadRegistry&&) noexcept = default;
	};
}