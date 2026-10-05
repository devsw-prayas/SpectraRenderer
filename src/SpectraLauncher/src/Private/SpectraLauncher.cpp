#include "SpectraLauncher.h"

#include <CoriumRuntime.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>

#include <PlatformWindowing.h>
#include <WindowUtils.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#include <SpecInstAddrSpace.h>
#include <SpecInstImage.h>
#include <SpecInstMacros.h>
#include <FrameRecords.h>

#include <SpecMemAddrSpace.h>
#include <SpectraHeap.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <memory>

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

		const bool mainUnattached = Frame::this_thread::currentFrame() == nullptr;

		static std::atomic<int> s_result{ 0 };
		Factory::AffinityFactory factory(0, 0x1);
		const ThreadHandle handle = factory.createAndStart(createClosure<void()>([]() {
			Frame::FrameHandle* current = Frame::this_thread::currentFrame();
			if (!current) { s_result.store(2); return; }

			Frame::FrameHandle& native = Frame::this_thread::nativeContext();
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
}

using namespace Spectra::Platform::Runtime::Windows;

static void runWindowThread() {
	PlatformWindow::init();

	WindowDesc desc{};
	desc.m_Title = "SpectraRenderer - Window Test";
	desc.m_Width = 1280;
	desc.m_Height = 720;
	desc.m_StyleFlags = WindowStyleFlags::RESIZABLE | WindowStyleFlags::MINIMIZABLE | WindowStyleFlags::MAXIMIZABLE;

	const WindowHandle handle = PlatformWindow::create(desc);
	if (!PlatformWindow::isValidHandle(handle)) {
		printf("[window] PlatformWindow::create() failed\n");
		return;
	}

	PlatformWindow::show(handle);
	PlatformWindow::enableDragDrop(handle);
	printf("[window] created and shown - close it to exit. Drag files onto it to test drag-drop.\n");

	bool running = true;
	while (running) {
		MSG msg;
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		WindowEvent event;
		while (PlatformWindow::pollEvent(handle, event)) {
			switch (event.m_Type) {
			case WindowEventType::CLOSE:
				printf("[window] Close received - destroying.\n");
				PlatformWindow::destroy(handle);
				running = false;
				break;
			case WindowEventType::RESIZE:
				printf("[window] Resize: %ux%u state=%u\n", event.m_Resize.m_Size.m_Width, event.m_Resize.m_Size.m_Height, static_cast<uint32_t>(event.m_Resize.m_State));
				break;
			case WindowEventType::MOVE:
				printf("[window] Move: (%d, %d)\n", event.m_Move.m_X, event.m_Move.m_Y);
				break;
			case WindowEventType::FOCUS_GAINED:
				printf("[window] Focus gained\n");
				break;
			case WindowEventType::FOCUS_LOST:
				printf("[window] Focus lost\n");
				break;
			case WindowEventType::KEY_DOWN:
				printf("[window] Key down: code=%u repeat=%d\n", static_cast<uint32_t>(event.m_Key.m_Code), event.m_Key.m_IsRepeat);
				break;
			case WindowEventType::KEY_UP:
				printf("[window] Key up: code=%u\n", static_cast<uint32_t>(event.m_Key.m_Code));
				break;
			case WindowEventType::TEXT_INPUT:
				printf("[window] Text input: U+%04X\n", static_cast<uint32_t>(event.m_TextInput.m_Codepoint));
				break;
			case WindowEventType::MOUSE_MOVE:
				printf("[window] Mouse move: (%d, %d)\n", event.m_MouseMove.m_X, event.m_MouseMove.m_Y);
				break;
			case WindowEventType::MOUSE_BUTTON_DOWN:
				printf("[window] Mouse button down: %u at (%d, %d)\n", static_cast<uint32_t>(event.m_MouseButton.m_Button), event.m_MouseButton.m_X, event.m_MouseButton.m_Y);
				break;
			case WindowEventType::MOUSE_BUTTON_UP:
				printf("[window] Mouse button up: %u at (%d, %d)\n", static_cast<uint32_t>(event.m_MouseButton.m_Button), event.m_MouseButton.m_X, event.m_MouseButton.m_Y);
				break;
			case WindowEventType::MOUSE_WHEEL:
				printf("[window] Mouse wheel: %d at (%d, %d)\n", event.m_MouseWheel.m_Delta, event.m_MouseWheel.m_X, event.m_MouseWheel.m_Y);
				break;
			case WindowEventType::RAW_INPUT:
				break; // too high-frequency to log usefully here
			case WindowEventType::DISPLAY_CHANGED:
				printf("[window] Display changed\n");
				break;
			case WindowEventType::DPI_CHANGED:
				printf("[window] DPI changed: %u\n", event.m_DpiChanged.m_Dpi);
				break;
			case WindowEventType::RAW_INPUT_DEVICE_ARRIVAL:
				printf("[window] Raw input device arrived\n");
				break;
			case WindowEventType::RAW_INPUT_DEVICE_REMOVAL:
				printf("[window] Raw input device removed\n");
				break;
			case WindowEventType::DROP:
				printf("[window] Drop: %u file(s) at (%d, %d)\n", event.m_Drop.m_FileCount, event.m_Drop.m_X, event.m_Drop.m_Y);
				for (uint32_t i = 0; i < event.m_Drop.m_FileCount; ++i) {
					printf("[window]   - %s\n", event.m_Drop.m_FilePaths[i]);
				}
				break;
			}
		}
	}

	PlatformWindow::shutdown();
}

void runFrames() {
	using namespace Corium::Core::Factory;
	using namespace Corium::Core::Utils;
	using namespace Corium::Core::Frame;
	DefaultThreadFactory factory{};
	FrameStackDesc desc{};
	init(desc);
	validate(desc);
	auto frame = NativeFrame::createFrame(buildClosure<void()>([]() { 
		printf("Hello world from a Frame! Gonna kill it now!\n");
		NativeFrame::yieldToNative(this_thread::currentFrame(), true);
	}, 0), desc);
	auto handle = factory.createAndStart(buildClosure<void()>([&frame]() { 
		printf("This is a random ahh thread \n");
		NativeFrame::switchTo(this_thread::currentFrame(), frame.value(), false);
		printf("And That frame vanished, boom!");
	}, 0), "Random Ahh thread");

	Corium::Core::NativeThread::joinThread(handle);
}

int main(int v_Argc, char** p_Argv) {
	printf("=== SpectraLauncher ===\n\n");

	// Load-bearing order: Instrumentation's own VA must exist before SpectraMemory boots.
	if (!Spectra::Instrumentation::Internal::init()) {
		printf("[main] SpectraInstrumentation address space init failed\n");
		return 1;
	}
	const bool instOk = runInstrumentationCheck();

	if (!Spectra::Memory::Internal::init()) {
		printf("[main] SpectraMemory address space init failed\n");
		return 1;
	}
	const bool heapOk = runHeapCheck() && runHeapThreadedCheck();

	Corium::CoriumRuntime::initRuntime();
	printf("[main] Runtime initialised.\n");
	const bool frameOk = runFrameAttachCheck();

	for (int i = 1; i < v_Argc; ++i)
		if (std::strcmp(p_Argv[i], "--window") == 0) runWindowThread();

	runFrames();
	return instOk && heapOk && frameOk ? 0 : 1;
}
