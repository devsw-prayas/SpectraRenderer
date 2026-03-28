#include <SpectraPlatformRuntime.h>
#include <PlatformThread.h>
#include <ThreadUtils.h>
#include <SpectraCompiler.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#if defined(SPECTRA_COMPILER_MSVC)
// TODO : Have to move to CMake
#pragma comment(lib, "synchronization.lib")
#endif

namespace Spectra::Platform::Runtime::Thread {
	struct WinLaunchContext {
		void* userContext;
		void (*userEntry)(void*);
	};

	static DWORD WINAPI WinThreadEntry(void* ctx) {
		auto* context = static_cast<WinLaunchContext*>(ctx);

		context[0].userEntry(context[0].userContext); // startup
		context[1].userEntry(context[1].userContext); // main
		context[2].userEntry(context[2].userContext); // shutdown

		delete[] context; // replace later with allocator
		return 0;
	}

	struct alignas(32) InvariantHandle final {
		HANDLE m_InternalHandle;
		Atomic::AtomicValue32<uint32_t> m_AccessCount;

		InvariantHandle() : m_InternalHandle(INVALID_HANDLE_VALUE), m_AccessCount(0) {}
		~InvariantHandle() = default;

		InvariantHandle(const InvariantHandle&) = default;
		InvariantHandle& operator=(const InvariantHandle&) = default;

		InvariantHandle(InvariantHandle&&) noexcept = default;
		InvariantHandle& operator=(InvariantHandle&&) noexcept = default;
	};

	struct alignas(64) SlotIdentity final {
		Atomic::AtomicValue64<size_t> m_SlotMask;
		size_t m_SlotGeneration;
		ThreadState m_State;

		SlotIdentity() : m_SlotMask(1LL << 0), m_SlotGeneration(0), m_State(ThreadState::REAPED) {}
		~SlotIdentity() = default;

		SlotIdentity(const SlotIdentity&) = default;
		SlotIdentity& operator=(const SlotIdentity&) = default;

		SlotIdentity(SlotIdentity&&) noexcept = default;
		SlotIdentity& operator=(SlotIdentity&&) noexcept = default;

		[[nodiscard]] size_t allocateToken() {
#ifdef SPECTRA_COMPILER_MSVC
			for (;;) {
				uint64_t mask = m_SlotMask.load(Intrinsic::MemoryOrder::RELAXED);

				uint64_t freeBits = ~mask;
				if (freeBits == 0) return static_cast<size_t>(-1);

				unsigned long slot;
				_BitScanForward64(&slot, freeBits);

				uint64_t bit = 1ull << slot;
				uint64_t expected = mask;
				uint64_t desired = mask | bit;

				if (m_SlotMask.compareExchange(&expected, desired, Intrinsic::MemoryOrder::ACQUIRE, Intrinsic::MemoryOrder::ACQ_REL)) return slot;
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

		size_t findFreeSlot() {
			for (size_t i = 0; i < this->m_HandleRegistry.size(); ++i) {
				auto& [handle, identity] = this->m_HandleRegistry[i];

				if (identity.m_State == ThreadState::REAPED &&
					identity.m_SlotMask.load(Intrinsic::MemoryOrder::RELAXED) == 0) {
					return i;
				}
			}

			return static_cast<size_t>(-1); // no free slot
		}

		size_t occupiedCount() const{
			size_t count = 0;

			for (auto& [handle, identity] : this->m_HandleRegistry) {
				if (!(identity.m_State == ThreadState::REAPED &&
					  identity.m_SlotMask.load(Intrinsic::MemoryOrder::RELAXED) == 0)) {
					count++;
				}
			}

			return count;
		}
	};

	ThreadHandle PlatformThread::createThread(const ThreadLaunchDesc& ro_LaunchDesc, const ThreadLaunchExecDesc& ro_ExecDesc) noexcept {
		InvariantHandle handle;
		SlotIdentity identity;
		identity.m_State = ThreadState::REAPED;
#if SPECTRA_COMPILER_MSVC
		LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList = nullptr;
		SIZE_T size = 0;
		InitializeProcThreadAttributeList(nullptr, 2, 0, &size);
		lpAttributeList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), 0, size));
		if (!lpAttributeList) return ThreadHandle::getInvalidHandle();
		InitializeProcThreadAttributeList(lpAttributeList, 2, 0, &size);

		const auto l_Cleanup = [&]() {
			DeleteProcThreadAttributeList(lpAttributeList);
			HeapFree(GetProcessHeap(), 0, lpAttributeList);
			};

		if (ro_ExecDesc.m_SupportsThreadGroup) {
			GROUP_AFFINITY affinity;
			affinity.Mask = ro_ExecDesc.m_AffinityMask;
			if (!UpdateProcThreadAttribute(lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY,
										   &affinity, sizeof(GROUP_AFFINITY), nullptr, nullptr)) {
				l_Cleanup();
				return ThreadHandle::getInvalidHandle();
			}
		}

		if (ro_ExecDesc.m_SupportsIdealProcessor) {
			DWORD idx = ro_ExecDesc.m_IdealProcessor;
			if (!UpdateProcThreadAttribute(lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR, &idx,
										   sizeof(Dword), nullptr, nullptr)) {
				l_Cleanup();
				return ThreadHandle::getInvalidHandle();
			}
		}

		SlotIdentity threadIdentity;
		ThreadHandle userHandle{ 0, 0, 0, ThreadState::REAPED };

		WinLaunchContext* contexts = new WinLaunchContext[3];
		contexts[0].userEntry = ro_LaunchDesc.m_StartupEntry;
		contexts[0].userContext = ro_LaunchDesc.m_StartupContext;

		contexts[1].userEntry = ro_LaunchDesc.m_StartEntry;
		contexts[1].userContext = ro_LaunchDesc.m_StartContext;

		contexts[2].userEntry = ro_LaunchDesc.m_ShutdownEntry;
		contexts[2].userContext = ro_LaunchDesc.m_ShutdownContext;

		auto l_TlsInit = 0;

			auto l_LaunchPoint = [&userHandle](const WinLaunchContext* context) {
			//TODO TLS stuff
			context[0].userEntry(context[0].userContext);
			context[1].userEntry(context[1].userContext);
			context[2].userEntry(context[2].userContext);

			delete[] context;
			};

		// TODO I need to sleep

#else
#error  "No Valid Syscalls available"
#endif
		return ThreadHandle{ 0, 0, 0, ThreadState::SEALED };
	}
}