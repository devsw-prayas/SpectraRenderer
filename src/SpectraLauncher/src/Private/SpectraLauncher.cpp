#include "SpectraLauncher.h"

#include <CoriumRuntime.h>

#include <PlatformWindowing.h>
#include <WindowUtils.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#include <SpecMemAddrSpace.h>
#include <SpectraHeap.h>
#include <VAUtils.h>

#include <cstdio>
#include <cstdint>
#include <cstring>

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

	Spectra::Memory::Heap::TlsfHeap g_Heap;

	bool runHeapCheck() {
		using namespace Spectra::Memory;
		printf("[heap] TLSF stress: random alloc/free, contents + checkHeap verified\n");

		MemoryHandle handle = segmentFromRegion(Internal::g_TlsfHeap[0]);
		handle.m_NumaNode = 0;
		g_Heap.init(&handle);

		int fail = 0;
		const auto check = [&](bool v_Ok, const char* p_What, int v_Step) {
			if (v_Ok) return;
			++fail;
			printf("[heap] FAIL  %s at step %d\n", p_What, v_Step);
		};

		check(g_Heap.checkHeap(), "checkHeap after init", -1);
		check(g_Heap.allocate(0) == nullptr, "allocate(0) returns null", -1);
		check(g_Heap.allocate(Heap::kMaxAlloc + 1) == nullptr, "oversized allocate returns null", -1);

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

				auto* ptr = static_cast<uint8_t*>(g_Heap.allocate(size));
				check(ptr != nullptr, "allocate returned null", step);
				if (!ptr) break;
				check(reinterpret_cast<uintptr_t>(ptr) % Heap::kAlign == 0, "payload misaligned", step);
				check(g_Heap.owns(ptr), "owns() rejects own pointer", step);

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

				g_Heap.deallocate(entry.m_Ptr);
				live[index] = live[--liveCount];
			}
			if (step % 5000 == 0) check(g_Heap.checkHeap(), "checkHeap mid-run", step);
		}

		for (size_t i = 0; i < liveCount; ++i) g_Heap.deallocate(live[i].m_Ptr);
		check(g_Heap.checkHeap(), "checkHeap after freeing everything", kSteps);

		void* big = g_Heap.allocate(64ull << 20);
		void* huge = g_Heap.allocate(3ull << 30);
		check(big && huge, "64 MiB + 3 GiB allocate", kSteps);
		check(g_Heap.checkHeap(), "checkHeap after big allocs", kSteps);
		g_Heap.deallocate(big);
		g_Heap.deallocate(huge);
		check(g_Heap.checkHeap(), "checkHeap after big frees", kSteps);

		printf("[heap] %s\n", fail == 0 ? "passed" : "FAILED");
		return fail == 0;
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

int main(int v_Argc, char** p_Argv) {
	printf("=== SpectraLauncher ===\n\n");

	if (!Spectra::Memory::Internal::init()) {
		printf("[main] SpectraMemory address space init failed\n");
		return 1;
	}
	const bool heapOk = runHeapCheck();

	Corium::CoriumRuntime::initRuntime();
	printf("[main] Runtime initialised.\n");

	for (int i = 1; i < v_Argc; ++i)
		if (std::strcmp(p_Argv[i], "--window") == 0) runWindowThread();

	return heapOk ? 0 : 1;
}
