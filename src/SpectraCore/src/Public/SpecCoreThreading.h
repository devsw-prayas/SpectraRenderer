#pragma once
#include "SpectraCore.h"
#include "SpecCoreDiagnostic.h"
#include "SpecCoreCompiler.h"
#include "PlatformThread.h"
#include "CoriumThread.h"
#include "ThreadSupport.h"

namespace Spectra::Core::Threads {
	// Bitmask: a PlatformThread that attached to Corium is SPECTRA | CORIUM. Zero-initialised storage
	// reads FOREIGN; only SpectraCore's creation path marks a thread otherwise.
	enum class ThreadKind : uint8_t {
		FOREIGN = 0,
		SPECTRA = 1 << 0,
		CORIUM = 1 << 1,
	};

	constexpr ThreadKind operator|(const ThreadKind v_Lhs, const ThreadKind v_Rhs) {
		return static_cast<ThreadKind>(static_cast<uint8_t>(v_Lhs) | static_cast<uint8_t>(v_Rhs));
	}

	constexpr ThreadKind operator&(const ThreadKind v_Lhs, const ThreadKind v_Rhs) {
		return static_cast<ThreadKind>(static_cast<uint8_t>(v_Lhs) & static_cast<uint8_t>(v_Rhs));
	}

	constexpr ThreadKind& operator|=(ThreadKind& r_Lhs, const ThreadKind v_Rhs) {
		r_Lhs = static_cast<ThreadKind>(static_cast<uint8_t>(r_Lhs) | static_cast<uint8_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr ThreadKind& operator&=(ThreadKind& r_Lhs, const ThreadKind v_Rhs) {
		r_Lhs = static_cast<ThreadKind>(static_cast<uint8_t>(r_Lhs) & static_cast<uint8_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr bool has(const ThreadKind v_Kind, const ThreadKind v_Flag) {
		return (v_Kind & v_Flag) != ThreadKind::FOREIGN;
	}

	namespace this_thread {
		// Derived, not stored: SPECTRA from PlatformThread's launch thunk, CORIUM from a live Corium handle
		// (Corium threads and attached threads alike). An adopted engine thread reads SPECTRA | CORIUM.
		SPEC_CORE_NODISCARD SPEC_CORE_RUNTIME_API ThreadKind kind() noexcept;
	}

	// Left undefined: FOREIGN threads have no backing types, so misuse fails at the instantiation.
	template<ThreadKind K>
	struct Unified;

	template<>
	struct Unified<ThreadKind::SPECTRA> final {
		using ThreadAttr = Spectra::Platform::Runtime::Thread::ThreadAttrDesc;
		using ThreadLaunch = Spectra::Platform::Runtime::Thread::ThreadLaunchDesc;
		using ThreadHandle = Spectra::Platform::Runtime::Thread::ThreadHandle;
		using ParkHandle = Spectra::Platform::Runtime::Thread::ParkHandle;
		using Backend = Spectra::Platform::Runtime::Thread::PlatformThread;
	};

	template<>
	struct Unified<ThreadKind::CORIUM> final {
		using ThreadAttr = Corium::Core::ThreadAttrDesc;
		using ThreadLaunch = Corium::Core::ThreadLaunchDesc;
		using ThreadHandle = Corium::Core::ThreadHandle;
		using ParkHandle = Corium::Core::ParkHandle;
		using Backend = Corium::Core::NativeThread;
	};

	// An adopted engine thread stays SPECTRA: lifecycle goes through PlatformThread.
	template<>
	struct Unified<ThreadKind::SPECTRA | ThreadKind::CORIUM> final {
		using ThreadAttr = Unified<ThreadKind::SPECTRA>::ThreadAttr;
		using ThreadLaunch = Unified<ThreadKind::SPECTRA>::ThreadLaunch;
		using ThreadHandle = Unified<ThreadKind::SPECTRA>::ThreadHandle;
		using ParkHandle = Unified<ThreadKind::SPECTRA>::ParkHandle;
		using Backend = Unified<ThreadKind::SPECTRA>::Backend;
	};


	using ParkingSupport = Corium::Core::ParkingSupport;

	// [Payload]: 33B [sizeof]: 64B. m_Kind says which union member is live.
	struct alignas(32) SpectraThread final {
		union {
			Unified<ThreadKind::SPECTRA>::ThreadHandle m_Spectra;
			Unified<ThreadKind::CORIUM>::ThreadHandle m_Corium;
		};
		ThreadKind m_Kind;

		SpectraThread() noexcept : m_Spectra(Unified<ThreadKind::SPECTRA>::ThreadHandle::getInvalidHandle()), m_Kind(ThreadKind::FOREIGN) {}
		explicit SpectraThread(const Unified<ThreadKind::SPECTRA>::ThreadHandle& r_Handle) noexcept : m_Spectra(r_Handle), m_Kind(ThreadKind::SPECTRA) {}
		explicit SpectraThread(const Unified<ThreadKind::CORIUM>::ThreadHandle& r_Handle) noexcept : m_Corium(r_Handle), m_Kind(ThreadKind::CORIUM) {}
	};
	static_assert(sizeof(SpectraThread) == 64, "SpectraThread must stay one cache line");

	// Dispatch layer. K picks the backend at compile time; m_Kind is checked against it in debug.
	namespace Internal {
		template<ThreadKind K>
		using Backend = typename Unified<K>::Backend;

		// Backend match, not exact match: an adopted thread (SPECTRA | CORIUM) answers SPECTRA calls.
		template<ThreadKind K>
		SPECTRA_FORCEINLINE const typename Unified<K>::ThreadHandle& unwrap(const SpectraThread& r_Thread) noexcept {
			SPEC_CORE_DEBUG_ASSERT(r_Thread.m_Kind != ThreadKind::FOREIGN && has(r_Thread.m_Kind, ThreadKind::SPECTRA) == has(K, ThreadKind::SPECTRA));
			if constexpr (has(K, ThreadKind::SPECTRA)) return r_Thread.m_Spectra;
			else return r_Thread.m_Corium;
		}

		template<ThreadKind K>
		SPECTRA_FORCEINLINE typename Unified<K>::ParkHandle& unwrap(ParkingSupport& r_Support) noexcept {
			return *r_Support.get<typename Unified<K>::ParkHandle>();
		}
	}

	// Adoption goes through attachAsCorium, never through creation.
	template<ThreadKind K>
	SpectraThread createThread(typename Unified<K>::ThreadLaunch&& u_Launch, const typename Unified<K>::ThreadAttr& r_Attr) noexcept {
		static_assert(K == ThreadKind::SPECTRA || K == ThreadKind::CORIUM, "createThread: K must be exactly SPECTRA or CORIUM");
		return SpectraThread(Internal::Backend<K>::createThread(std::move(u_Launch), r_Attr));
	}

	template<ThreadKind K>
	bool detachThread(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::detachThread(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	bool closeHandle(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::closeHandle(Internal::unwrap<K>(r_Thread)); }

	// The duplicate keeps the source's kind, adopted bit included.
	template<ThreadKind K>
	SpectraThread duplicateHandle(const SpectraThread& r_Thread) noexcept {
		SpectraThread copy(Internal::Backend<K>::duplicateHandle(Internal::unwrap<K>(r_Thread)));
		copy.m_Kind = r_Thread.m_Kind;
		return copy;
	}

	template<ThreadKind K>
	bool isAlive(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::isAlive(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	size_t getThreadID(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::getThreadID(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	bool suspendThread(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::suspendThread(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	bool resumeThread(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::resumeThread(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	bool terminateThread(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::terminateThread(Internal::unwrap<K>(r_Thread)); }

	template<ThreadKind K>
	bool joinThread(const SpectraThread& r_Thread) noexcept { return Internal::Backend<K>::joinThread(Internal::unwrap<K>(r_Thread)); }

	// Spectra-only: engine threads are hardware-locked, Corium threads are placed by the runtime.
	template<ThreadKind K>
	bool setPriority(const SpectraThread& r_Thread, Spectra::Platform::Runtime::Thread::Priority v_Priority) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "setPriority: SPECTRA threads only");
		return Internal::Backend<K>::setPriority(Internal::unwrap<K>(r_Thread), v_Priority);
	}

	template<ThreadKind K>
	Spectra::Platform::Runtime::Thread::Priority getPriority(const SpectraThread& r_Thread) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "getPriority: SPECTRA threads only");
		return Internal::Backend<K>::getPriority(Internal::unwrap<K>(r_Thread));
	}

	template<ThreadKind K>
	bool setPriorityBoost(const SpectraThread& r_Thread, bool v_Permission) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "setPriorityBoost: SPECTRA threads only");
		return Internal::Backend<K>::setPriorityBoost(Internal::unwrap<K>(r_Thread), v_Permission);
	}

	template<ThreadKind K>
	bool getPriorityBoost(const SpectraThread& r_Thread) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "getPriorityBoost: SPECTRA threads only");
		return Internal::Backend<K>::getPriorityBoost(Internal::unwrap<K>(r_Thread));
	}

	template<ThreadKind K>
	bool setAffinity(const SpectraThread& r_Thread, Spectra::Platform::Runtime::Thread::AffinityDesc& r_Desc) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "setAffinity: SPECTRA threads only");
		return Internal::Backend<K>::setAffinity(Internal::unwrap<K>(r_Thread), r_Desc);
	}

	template<ThreadKind K>
	Spectra::Platform::Runtime::Thread::AffinityDesc getAffinity(const SpectraThread& r_Thread) noexcept {
		static_assert(has(K, ThreadKind::SPECTRA), "getAffinity: SPECTRA threads only");
		return Internal::Backend<K>::getAffinity(Internal::unwrap<K>(r_Thread));
	}

	// Corium-only: needs a Corium handle, so an adopted engine thread can't answer it through here.
	template<ThreadKind K>
	uint32_t getNumaNode(const SpectraThread& r_Thread) noexcept {
		static_assert(K == ThreadKind::CORIUM, "getNumaNode: CORIUM threads only");
		return Internal::Backend<K>::getNumaNode(Internal::unwrap<K>(r_Thread));
	}

	// Parking: both backends wait on the same 32-bit permit word; Spectra takes the handle, Corium the support.
	template<ThreadKind K>
	void park(ParkingSupport& r_Permit) noexcept {
		if constexpr (has(K, ThreadKind::SPECTRA)) Internal::Backend<K>::waitOnAddress(Internal::unwrap<K>(r_Permit));
		else Internal::Backend<K>::waitOnAddress(r_Permit);
	}

	template<ThreadKind K>
	void parkFor(ParkingSupport& r_Permit, uint32_t v_TimeoutMs) noexcept {
		if constexpr (has(K, ThreadKind::SPECTRA)) Internal::Backend<K>::waitOnAddressFor(Internal::unwrap<K>(r_Permit), v_TimeoutMs);
		else Internal::Backend<K>::waitOnAddressFor(r_Permit, Corium::Core::Chrono::until(Corium::Core::Chrono::milliseconds(v_TimeoutMs)));
	}

	template<ThreadKind K>
	void unpark(ParkingSupport& r_Permit) noexcept {
		if constexpr (has(K, ThreadKind::SPECTRA)) Internal::Backend<K>::wakeOnAddress(Internal::unwrap<K>(r_Permit));
		else Internal::Backend<K>::wakeOnAddress(r_Permit);
	}

	template<ThreadKind K>
	void unparkAll(ParkingSupport& r_Permit) noexcept {
		if constexpr (has(K, ThreadKind::SPECTRA)) Internal::Backend<K>::wakeAllOnAddress(Internal::unwrap<K>(r_Permit));
		else Internal::Backend<K>::wakeAllOnAddress(r_Permit);
	}
}


namespace Spectra::Core::Frames {

}
