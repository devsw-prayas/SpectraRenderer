#include "Bootstrap.h"

#include <CoriumRuntime.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <SpectraSyscalls.h>
#include <SpecInstAddrSpace.h>
#include <SpecInstImage.h>
#include <SpecInstMacros.h>
#include <FrameRecords.h>
#include <SpecMemAddrSpace.h>
#include <SpectraHeap.h>
#include <SpectraAllocators.h>

namespace {
	// Fixed-seed xorshift so a failing run replays exactly.
	struct XorShift64 {
		uint64_t m_State;
		uint64_t next() {
			m_State ^= m_State << 13;
			m_State ^= m_State >> 7;
			m_State ^= m_State << 17;
			return m_State;
		}
	};

	struct LiveAlloc {
		uint8_t* m_Ptr;
		size_t m_Size;
		uint8_t m_Tag;
	};

	struct CountingScope {
		static inline int s_live = 0;
		static inline int s_maxLive = 0;
		explicit CountingScope(int v_Tag) { SPEC_INST_UNUSED(v_Tag); if (++s_live > s_maxLive) s_maxLive = s_live; }
		~CountingScope() { --s_live; }
	};

	bool runInstrumentationCheck() {
		using namespace Spectra::Instrumentation;
		printf("[inst] carver, image-relative offsets, scope macros\n");
		int fail = 0;
		const auto check = [&](bool v_Ok, const char* p_What) {
			if (v_Ok) return;
			++fail;
			printf("[inst] FAIL  %s\n", p_What);
		};

		Utils::InstrumentationVACarver& carver = Internal::coreCarver();
		const Utils::Region callee = carver.carve(kCalleeMainCapacity * sizeof(CalleeFrameRecord), alignof(CalleeFrameRecord));
		const Utils::Region err = carver.carve(kErrStackCapacity * sizeof(ErrContext), 64);
		const Utils::Region exec = carver.carve(kExecPoolCapacity * sizeof(InstrumentedException), 4096);
		check(callee.isValid() && err.isValid() && exec.isValid(), "carves valid");
		check(reinterpret_cast<uintptr_t>(err.m_BaseAddr) % 64 == 0 && reinterpret_cast<uintptr_t>(exec.m_BaseAddr) % 4096 == 0, "carve alignment");
		check(static_cast<uint8_t*>(err.m_BaseAddr) >= static_cast<uint8_t*>(callee.m_BaseAddr) + callee.m_MaxSize, "carves don't overlap");

		// Touch first and last byte of each carve: faults here mean commit missed a page.
		for (const Utils::Region& region : { callee, err, exec }) {
			auto* bytes = static_cast<uint8_t*>(region.m_BaseAddr);
			bytes[0] = 0xAB;
			bytes[region.m_MaxSize - 1] = 0xCD;
			check(bytes[0] == 0xAB && bytes[region.m_MaxSize - 1] == 0xCD, "carved bytes writable");
		}

		static const char* s_label = "SpectraLauncher::runInstrumentationCheck";
		const uint64_t offset = imageRelativeOffset(s_label);
		check(reinterpret_cast<const char*>(currentImageBase() + offset) == s_label, "offset round-trips to the label");
		check(offset < (uint64_t{ 1 } << 32), "label lies inside this image");
		// For checking the offline round trip with spectra-resolve.
		printf("[inst] label offset 0x%llX\n", static_cast<unsigned long long>(offset));

		SPEC_INST_SCOPE_BEGIN(CountingScope, 1)
			SPEC_INST_SCOPE_BEGIN(CountingScope, 2)
				check(CountingScope::s_live == (SPECTRA_INSTRUMENTATION_ENABLED ? 2 : 0), "nested scopes alive");
			SPEC_INST_SCOPE_END()
		SPEC_INST_SCOPE_END()
		check(CountingScope::s_live == 0, "scopes destroyed at END");

		printf("[inst] %s (carved %zu KiB)\n", fail == 0 ? "passed" : "FAILED", carver.used() / 1024);
		return fail == 0;
	}

	// Factory threads go through attachThreadState, which must leave a 64-aligned native context
	// (handle + register blob) bound as both nativeContext() and currentFrame().
	bool runFrameAttachCheck() {
		using namespace Corium::Core;
		printf("[frame] native context on attach/detach\n");

		// Runs on the Corium-launched bootstrap thread, so probe a raw thread for the unattached case.
		bool mainUnattached = false;
		std::thread([&mainUnattached]() { mainUnattached = Frame::this_frame::currentFrame() == nullptr; }).join();

		static std::atomic<int> s_result{ 0 };
		Factory::AffinityFactory factory(0, 0x1);
		const ThreadHandle handle = factory.createAndStart(makeClosure<void()>([]() {
			Frame::FrameHandle* current = Frame::this_frame::currentFrame();
			if (!current) { s_result.store(2); return; }

			Frame::FrameHandle& native = Frame::this_frame::nativeContext();
			auto* bytes = reinterpret_cast<std::byte*>(current);
			const bool ok = current == std::addressof(native)
				&& reinterpret_cast<uintptr_t>(current) % 64 == 0
				&& static_cast<std::byte*>(current->m_RegBlob) == bytes + sizeof(Frame::FrameHandle);

			// The context switch writes the whole blob, so it must be ours to write.
			std::memset(current->m_RegBlob, 0xAB, Corium::Memory::Internal::FrameRegBlobSize);
			s_result.store(ok ? 1 : 3);
		}), "FrameAttachCheck");

		const bool joined = NativeThread::joinThread(handle);
		const int result = s_result.load();
		const bool ok = mainUnattached && joined && result == 1;
		printf("[frame] %s (main unattached=%d, joined=%d, thread result=%d)\n", ok ? "passed" : "FAILED", mainUnattached, joined, result);
		return ok;
	}

	// Raw threads exit without detaching; all attaches only succeed if thread exit frees the registry slot.
	bool runThreadAdoptionCheck() {
		using namespace Corium::Core;
		printf("[thread] attach/detach on foreign threads\n");

		// The bootstrap thread is Corium-launched, so the foreign-thread sequence runs on a raw thread.
		bool attached = false, nativeBound = false, handleValid = false, closeRefused = false;
		bool nested = false, stillAttached = false, released = false;
		std::thread([&]() {
			attached = this_thread::attach();
			nativeBound = Frame::this_frame::currentFrame() == std::addressof(Frame::this_frame::nativeContext());
			handleValid = NativeThread::getNumaNode(this_thread::currentHandle()) == 0u;
			closeRefused = !NativeThread::closeHandle(this_thread::currentHandle());
			nested = this_thread::attach();
			this_thread::detach();
			stillAttached = Frame::this_frame::currentFrame() != nullptr;
			this_thread::detach();
			released = Frame::this_frame::currentFrame() == nullptr;
		}).join();

		constexpr int kThreads = 300;	// more than MAX_CORIUM_THREADS, so slots must be reused
		int exitAttached = 0;
		for (int i = 0; i < kThreads; ++i) {
			bool ok = false;
			std::thread([&ok] { ok = this_thread::attach() && Frame::this_frame::currentFrame() != nullptr; }).join();
			exitAttached += ok ? 1 : 0;
		}

		const bool ok = attached && nativeBound && handleValid && closeRefused && nested && stillAttached && released && exitAttached == kThreads;
		printf("[thread] %s (attach=%d, native=%d, handle=%d, close refused=%d, nested=%d, released=%d, exit-attached %d/%d)\n",
			ok ? "passed" : "FAILED", attached, nativeBound, handleValid, closeRefused, nested && stillAttached, released, exitAttached, kThreads);
		return ok;
	}

	// Suspend a frame on thread A, resume it on thread B. Only frame-owned state (its stack) may survive the hop;
	// the thread id is read fresh after the resume and must differ.
	bool runFrameMigrationCheck() {
		using namespace Corium::Core;
		printf("[frame] migration: suspend on A, resume on B\n");

		// CALLEE_OWNED so the handle outlives TERMINATED and can be checked; a pool block would be released on landing.
		alignas(64) static std::byte s_stack[256 * 1024];
		static std::atomic<int> s_yielded{ 0 };
		static std::atomic<int> s_migrated{ -1 };
		static std::atomic<int> s_canaryOk{ -1 };
		static std::atomic<int> s_claimSpins{ 0 };
		static std::atomic<int> s_terminated{ 0 };

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		Frame::setProvenance(desc, Frame::Provenance::CALLEE_OWNED);
		Frame::setMemoryLocation(desc, s_stack);
		Frame::setStackSize(desc, sizeof(s_stack));
		if (!Frame::validate(desc)) {
			printf("[frame] FAILED (desc did not validate)\n");
			return false;
		}

		// Named, not a temporary: the frame keeps a FunctionView into it.
		auto entry = Utils::makeClosure<void()>([]() {
			const std::thread::id before = std::this_thread::get_id();
			volatile uint64_t canary = 0x5EC7'4A11'C0DE'F00Dull;
			Frame::NativeFrame::yieldToNative(Frame::this_frame::currentFrame(), false);
			// Resumed: fresh carrier-side reads only.
			s_migrated.store(std::this_thread::get_id() != before ? 1 : 0);
			s_canaryOk.store(canary == 0x5EC7'4A11'C0DE'F00Dull ? 1 : 0);
		}, 0);
		auto created = Frame::NativeFrame::createFrame(std::move(entry), desc);
		if (!created) {
			printf("[frame] FAILED (createFrame)\n");
			return false;
		}
		Frame::FrameHandle* frame = created.value();

		Factory::DefaultThreadFactory factory{};
		const ThreadHandle a = factory.createAndStart(makeClosure<void()>([frame]() {
			Frame::this_frame::captureFrame(frame);
			// Back on A's native context: afterSwitch has published SUSPENDED by now.
			s_yielded.store(1, std::memory_order_release);
		}), "MigrateA");

		const ThreadHandle b = factory.createAndStart(makeClosure<void()>([frame]() {
			// Without this the frame could still be READY and B would simply run it first.
			while (s_yielded.load(std::memory_order_acquire) == 0) std::this_thread::yield();
			while (!Frame::this_frame::tryCaptureFrame(frame)) {
				s_claimSpins.fetch_add(1, std::memory_order_relaxed);
				_mm_pause();
			}
			s_terminated.store(Corium::Atomics::load<Corium::Atomics::MemoryOrder::ACQUIRE>(&frame->m_State) == Frame::FrameState::TERMINATED ? 1 : 0);
		}), "MigrateB");

		const bool joined = NativeThread::joinThread(a) && NativeThread::joinThread(b);
		const bool ok = joined && s_migrated.load() == 1 && s_canaryOk.load() == 1 && s_terminated.load() == 1;
		printf("[frame] %s (joined=%d, migrated=%d, canary=%d, terminated=%d, claim spins=%d)\n", ok ? "passed" : "FAILED",
			joined, s_migrated.load(), s_canaryOk.load(), s_terminated.load(), s_claimSpins.load());
		return ok;
	}

	// Frame path: createFrame takes ownership of the closure, so a temporary must stay valid until the frame runs.
	// The destructor case catches a closure destroyed early: its capture would read as poisoned.
	struct PoisonOnDestroy {
		uint64_t m_Value;
		~PoisonOnDestroy() { m_Value = 0xDEADDEADDEADDEADull; }
	};

	// TEB stack bounds are swapped on every frame switch. Without them the dispatcher rejects a frame's stack as
	// out of bounds: any exception kills the process unhandled and stack walks return nothing (2026-10-06 experiment).
	// __try lives alone in its own function: MSVC forbids it next to objects that need unwinding.
	CORIUM_NOINLINE int sehCatchesAccessViolation() {
		__try {
			*static_cast<volatile int*>(nullptr) = 1;
		} __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			return 1;
		}
		return 0;
	}

	CORIUM_NOINLINE int cppCatchesThrow(int v_Value) {
		try {
			if (v_Value != 0) throw v_Value;
		} catch (int caught) {
			return caught == v_Value ? 1 : 0;
		}
		return 0;
	}

	bool runFrameExceptionCheck() {
		using namespace Corium::Core;
		printf("[frame] exceptions and stack walks inside a frame\n");

		static std::atomic<int> s_cpp{ -1 };
		static std::atomic<int> s_seh{ -1 };
		static std::atomic<int> s_depth{ -1 };
		static std::atomic<int> s_boundsOk{ -1 };

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		Frame::validate(desc);

		auto created = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([]() {
			// The TEB must describe this frame's stack while it runs.
			NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
			volatile int local = 0;
			const auto here = reinterpret_cast<uintptr_t>(&local);
			s_boundsOk.store(here < reinterpret_cast<uintptr_t>(tib->StackBase) && here >= reinterpret_cast<uintptr_t>(tib->StackLimit) ? 1 : 0);

			s_cpp.store(cppCatchesThrow(42));
			s_seh.store(sehCatchesAccessViolation());

			void* trace[32] = {};
			s_depth.store(static_cast<int>(RtlCaptureStackBackTrace(0, 32, trace, nullptr)));
		}), desc);
		if (!created) {
			printf("[frame] FAILED (createFrame)\n");
			return false;
		}

		Frame::FrameHandle* frame = created.value();
		Factory::DefaultThreadFactory factory{};
		const ThreadHandle t = factory.createAndStart(makeClosure<void()>([frame]() {
			Frame::this_frame::captureFrame(frame);
		}), "FrameExceptionCheck");
		const bool joined = NativeThread::joinThread(t);

		const bool ok = joined && s_boundsOk.load() == 1 && s_cpp.load() == 1 && s_seh.load() == 1 && s_depth.load() > 0;
		printf("[frame] %s (TEB bounds=%d, C++ throw=%d, SEH AV=%d, stack walk depth=%d)\n", ok ? "passed" : "FAILED",
			s_boundsOk.load(), s_cpp.load(), s_seh.load(), s_depth.load());
		return ok;
	}

	bool runFrameCaptureCheck() {
		using namespace Corium::Core;
		printf("[frame] capturing closures through createFrame\n");

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		Frame::validate(desc);

		static std::atomic<uint64_t> s_seen{ 0 };
		// Runtime values, so the captures are real reads from closure storage, not folded constants.
		volatile uint64_t va = 0x1111'2222'3333'4444ull, vb = 0x5555'6666'7777'8888ull;
		const uint64_t a = va, b = vb;

		const auto runOn = [](Frame::FrameHandle* p_Frame) {
			Factory::DefaultThreadFactory factory{};
			NativeThread::joinThread(factory.createAndStart(makeClosure<void()>([p_Frame]() {
				Frame::this_frame::captureFrame(p_Frame);
			}), "CaptureCheck"));
		};
		const auto report = [](const char* p_Name, uint64_t v_Expected) {
			const uint64_t seen = s_seen.exchange(0);
			printf("[frame]   %-22s expected=%016llx seen=%016llx %s\n", p_Name, (unsigned long long)v_Expected,
				(unsigned long long)seen, seen == v_Expected ? "ok" : "MISMATCH");
			return seen == v_Expected;
		};

		auto named = Utils::makeClosure<void()>([a, b]() { s_seen.store(a ^ b); }, 0);
		runOn(Frame::NativeFrame::createFrame(std::move(named), desc).value());
		const bool namedOk = report("named, plain", a ^ b);

		// Separate statement: the temporary closure dies at the semicolon, before the frame runs.
		auto f2 = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([a, b]() { s_seen.store(a ^ b); }, 0), desc);
		runOn(f2.value());
		const bool tempPlainOk = report("temporary, plain", a ^ b);

		auto f3 = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([p = PoisonOnDestroy{ a }]() { s_seen.store(p.m_Value); }, 0), desc);
		runOn(f3.value());
		const bool tempDtorOk = report("temporary, with dtor", a);

		return namedOk && tempPlainOk && tempDtorOk;
	}

	bool runHeapCheck() {
		using namespace Spectra::Memory;
		printf("[heap] TLSF stress: random plain/aligned alloc/free, contents + checkHeap verified\n");

		Heap::HeapRegistry::init();
		Heap::SyncHeap& heap = Heap::HeapRegistry::forNode(0);

		int fail = 0;
		const auto check = [&](bool v_Ok, const char* p_What, int v_Step) {
			if (v_Ok) return;
			++fail;
			printf("[heap] FAIL  %s at step %d\n", p_What, v_Step);
		};

		check(heap.checkHeap(), "checkHeap after init", -1);
		check(heap.allocate(0) == nullptr, "allocate(0) returns null", -1);
		check(heap.allocate(Heap::kMaxAlloc + 1) == nullptr, "oversized allocate returns null", -1);

		constexpr size_t kMaxLive = 4096;
		static LiveAlloc live[kMaxLive];
		size_t liveCount = 0;
		XorShift64 rng{ 0x5EC7A11E5EEDull };

		constexpr int kSteps = 200000;
		for (int step = 0; step < kSteps && fail == 0; ++step) {
			const bool doAlloc = liveCount == 0 || (liveCount < kMaxLive && rng.next() % 100 < 55);
			if (doAlloc) {
				// Mostly small, some medium, rare multi-MiB so grow() gets exercised.
				const uint64_t bucket = rng.next() % 100;
				const size_t size = bucket < 70 ? 1 + rng.next() % 256
					: bucket < 95 ? 1 + rng.next() % 16384
					: bucket < 99 ? 1 + rng.next() % (1u << 20)
					: 1 + rng.next() % (8u << 20);
				// One in five goes through the aligned path, 32 B to 64 KiB.
				const size_t alignment = rng.next() % 5 == 0 ? size_t{ 32 } << (rng.next() % 12) : Heap::kAlign;

				auto* ptr = static_cast<uint8_t*>(alignment > Heap::kAlign ? heap.allocateAligned(size, alignment) : heap.allocate(size));
				check(ptr != nullptr, "allocate returned null", step);
				if (!ptr) break;
				check(reinterpret_cast<uintptr_t>(ptr) % alignment == 0, "payload misaligned", step);
				check(heap.owns(ptr), "owns() rejects own pointer", step);

				const auto tag = static_cast<uint8_t>(rng.next());
				std::memset(ptr, tag, size);
				live[liveCount++] = { ptr, size, tag };
			} else {
				const size_t index = rng.next() % liveCount;
				const LiveAlloc entry = live[index];
				// Strided + last byte: catches neighbours overwriting this block without an O(n) scan.
				for (size_t i = 0; i < entry.m_Size; i += 61)
					if (entry.m_Ptr[i] != entry.m_Tag) { check(false, "block contents clobbered", step); break; }
				check(entry.m_Ptr[entry.m_Size - 1] == entry.m_Tag, "block tail clobbered", step);

				heap.deallocate(entry.m_Ptr);
				live[index] = live[--liveCount];
			}
			if (step % 5000 == 0) check(heap.checkHeap(), "checkHeap mid-run", step);
		}

		for (size_t i = 0; i < liveCount; ++i) heap.deallocate(live[i].m_Ptr);
		check(heap.checkHeap(), "checkHeap after freeing everything", kSteps);

		void* big = heap.allocate(64ull << 20);
		void* huge = heap.allocateAligned(3ull << 30, 2ull << 20);
		check(big && huge, "64 MiB + 2 MiB-aligned 3 GiB allocate", kSteps);
		check(reinterpret_cast<uintptr_t>(huge) % (2ull << 20) == 0, "3 GiB block misaligned", kSteps);
		check(heap.checkHeap(), "checkHeap after big allocs", kSteps);
		heap.deallocate(big);
		heap.deallocate(huge);
		check(heap.checkHeap(), "checkHeap after big frees", kSteps);

		printf("[heap] single-thread %s\n", fail == 0 ? "passed" : "FAILED");
		return fail == 0;
	}

	// Threads trade pointers through shared slots, so most frees land on a heap another thread is
	// using: covers the lock, the tryLock-fail remote-free push, and the drain.
	constexpr size_t kSlotCount = 1024;
	constexpr int kThreadCount = 8;
	constexpr int kOpsPerThread = 100000;
	std::atomic<uint8_t*> g_Slots[kSlotCount];
	std::atomic<int> g_MtFailures{ 0 };

	bool verifyAndFree(uint8_t* p_Ptr) {
		size_t size;
		std::memcpy(&size, p_Ptr, sizeof(size));
		const auto tag = static_cast<uint8_t>(size * 131);
		bool ok = true;
		for (size_t i = sizeof(size); i < size; i += 37)
			if (p_Ptr[i] != tag) { ok = false; break; }
		Spectra::Memory::Heap::HeapRegistry::deallocate(p_Ptr);
		return ok;
	}

	void heapWorker(uint64_t v_Seed) {
		XorShift64 rng{ v_Seed };
		for (int op = 0; op < kOpsPerThread; ++op) {
			const size_t size = sizeof(size_t) + rng.next() % 2048;
			auto* ptr = static_cast<uint8_t*>(Spectra::Memory::Heap::HeapRegistry::allocate(size));
			if (!ptr) { g_MtFailures.fetch_add(1); return; }
			std::memcpy(ptr, &size, sizeof(size));
			std::memset(ptr + sizeof(size), static_cast<uint8_t>(size * 131), size - sizeof(size));

			uint8_t* previous = g_Slots[rng.next() % kSlotCount].exchange(ptr, std::memory_order_acq_rel);
			if (previous && !verifyAndFree(previous)) g_MtFailures.fetch_add(1);
		}
	}

	bool runHeapThreadedCheck() {
		printf("[heap] TLSF threaded: %d threads x %d ops, cross-thread frees\n", kThreadCount, kOpsPerThread);
		std::thread workers[kThreadCount];
		for (int i = 0; i < kThreadCount; ++i) workers[i] = std::thread(heapWorker, 0x9E3779B97F4A7C15ull * (i + 1));
		for (std::thread& worker : workers) worker.join();

		for (std::atomic<uint8_t*>& slot : g_Slots)
			if (uint8_t* ptr = slot.exchange(nullptr); ptr && !verifyAndFree(ptr)) g_MtFailures.fetch_add(1);

		const bool ok = g_MtFailures.load() == 0 && Spectra::Memory::Heap::HeapRegistry::forNode(0).checkHeap();
		printf("[heap] threaded %s\n", ok ? "passed" : "FAILED");
		return ok;
	}

	bool isCommitted(const Spectra::Memory::MemoryHandle& ro_Handle, size_t v_Offset) {
		Spectra::Memory::MemoryQueryDesc desc{};
		desc.m_TargetAddress = static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + v_Offset;
		return Spectra::Memory::Memory::query(desc).m_State == Spectra::Memory::MemoryState::COMMIT;
	}

	Spectra::Memory::MemoryHandle reserveScratch(size_t v_Size) {
		using namespace Spectra::Memory;
		VirtualMemoryDesc desc{};
		initMemoryDesc(desc);
		setSize(desc, v_Size);
		setMemoryState(desc, MemoryState::RESERVE);
		return Memory::reserve(desc);
	}

	bool runArenaCheck() {
		using namespace Spectra::Memory;
		using namespace Spectra::Memory::Allocators;
		printf("[arena] bad-input nulls, reclaimable reset/unwind/trim decommit\n");
		constexpr size_t kMiB = size_t{ 1 } << 20;

		MemoryHandle plainHandle = reserveScratch(16 * kMiB);
		LinearArena plain;
		plain.init(&plainHandle);
		const bool nulls = plain.allocate(0) == nullptr && plain.allocate(64, 3) == nullptr
			&& plain.allocate(32 * kMiB) == nullptr && plain.allocate(SIZE_MAX) == nullptr;
		void* aligned = plain.allocate(100, 4096);
		const bool alignedOk = aligned && (reinterpret_cast<uintptr_t>(aligned) & 4095) == 0;
		const bool fillsExactly = plain.allocate(16 * kMiB - 100) != nullptr && plain.allocate(1) == nullptr;

		MemoryHandle linearHandle = reserveScratch(64 * kMiB);
		ReclaimableLinearArena linear;
		linear.init(&linearHandle);
		auto* big = static_cast<uint8_t*>(linear.allocate(10 * kMiB));
		big[10 * kMiB - 1] = 0xAB;
		const bool linearCommitted = isCommitted(linearHandle, 10 * kMiB - 1);
		linear.reset();
		// reset() decommits through the arena's own handle copy; probe the OS, not our copy.
		const bool linearReleased = !isCommitted(linearHandle, 0) && !isCommitted(linearHandle, 10 * kMiB - 1);
		auto* again = static_cast<uint8_t*>(linear.allocate(kMiB));
		again[0] = 1;
		const bool linearRecommit = again == big && isCommitted(linearHandle, 0);

		MemoryHandle stackHandle = reserveScratch(64 * kMiB);
		ReclaimableStackArena stack;
		stack.init(&stackHandle);
		(void)stack.allocate(kMiB);
		const auto marker = stack.mark();
		auto* spike = static_cast<uint8_t*>(stack.allocate(20 * kMiB));
		spike[20 * kMiB - 1] = 1;
		stack.unwind(marker);
		// Chunk-rounded: the first 2 MiB chunk (holding the marker) stays, everything past it goes.
		const bool stackKept = isCommitted(stackHandle, 0);
		const bool stackReleased = !isCommitted(stackHandle, 4 * kMiB) && !isCommitted(stackHandle, 21 * kMiB - 1);
		(void)stack.allocate(8 * kMiB);
		stack.trim(0);
		const bool trimKeepsLive = isCommitted(stackHandle, 9 * kMiB - 1);

		const bool ok = nulls && alignedOk && fillsExactly && linearCommitted && linearReleased && linearRecommit
			&& stackKept && stackReleased && trimKeepsLive;
		printf("[arena] %s (nulls=%d, aligned=%d, fills=%d, linear commit/release/recommit=%d/%d/%d, stack kept/released/trim-live=%d/%d/%d)\n",
			ok ? "passed" : "FAILED", nulls, alignedOk, fillsExactly, linearCommitted, linearReleased, linearRecommit, stackKept, stackReleased, trimKeepsLive);
		return ok;
	}

	bool runHeapTrimCheck() {
		using namespace Spectra::Memory::Heap;
		printf("[heap] end-shrink trim after a big transient load\n");
		SyncHeap& heap = HeapRegistry::forNode(0);
		// The stress runs above leave the pool huge; trim first so growth is measurable.
		HeapRegistry::trim();
		const size_t before = heap.poolSize();

		void* small = heap.allocate(256);
		void* big = heap.allocate(size_t{ 96 } << 20);
		const size_t grown = heap.poolSize();
		heap.deallocate(big);
		HeapRegistry::trim();
		const size_t trimmed = heap.poolSize();
		const bool consistent = heap.checkHeap();

		void* regrow = heap.allocate(size_t{ 32 } << 20);
		const bool regrowOk = regrow && heap.checkHeap();
		heap.deallocate(regrow);
		heap.deallocate(small);
		HeapRegistry::trim();

		const bool ok = big && grown > before && trimmed < grown && trimmed <= before + kGrowChunk
			&& consistent && regrowOk && heap.checkHeap();
		printf("[heap] trim %s (pool before=%zu KiB, grown=%zu KiB, trimmed=%zu KiB, final=%zu KiB)\n",
			ok ? "passed" : "FAILED", before >> 10, grown >> 10, trimmed >> 10, heap.poolSize() >> 10);
		return ok;
	}
}

using namespace Corium::Core::Factory;
using namespace Corium::Core::Utils;
using namespace Corium::Core::Frame;

void runFrames() {
	DefaultThreadFactory factory{};
	FrameStackDesc desc{};
	init(desc);
	validate(desc);
	auto frame = NativeFrame::createFrame(makeClosure<void()>([]() { 
		printf("Hello world from a Frame! Gonna kill it now!\n");
		NativeFrame::yieldToNative(this_frame::currentFrame(), true);
	}, 0), desc);
	auto handle = factory.createAndStart(makeClosure<void()>([&frame]() { 
		printf("This is a random ahh thread \n");
		NativeFrame::switchTo(this_frame::currentFrame(), frame.value(), false);
		printf("And That frame vanished, boom!");
	}, 0), "Random Ahh thread");

	Corium::Core::NativeThread::joinThread(handle); 
}

void tebCrossTransition() {
	
}

int bootstrapRun() {
	const bool instOk = runInstrumentationCheck();
	const bool heapOk = runHeapCheck() && runHeapThreadedCheck() && runHeapTrimCheck() && runArenaCheck();
	const bool frameOk = runFrameAttachCheck() && runThreadAdoptionCheck() && runFrameMigrationCheck() && runFrameCaptureCheck() && runFrameExceptionCheck();
	runFrames();
	return instOk && heapOk && frameOk ? 0 : 1;
}
