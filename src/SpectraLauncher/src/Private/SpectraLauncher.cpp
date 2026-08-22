#include "SpectraLauncher.h"

#include <CoriumRuntime.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <ThreadUtils.h>
#include <CoriumUtility.h>
#include <CoriumMemoryHandler.h>
#include <EngineAllocators.h>

#include <PlatformWindowing.h>
#include <WindowUtils.h>

#include <Region.h>
#include <KerbecsRuntime.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#include <cstdio>

// Experiment: Kerbecs stays out of Corium proper (Corium is meant to be a
// standalone, dependency-free library) - this wraps one of Corium's own
// allocator instances from the application side instead, to see whether
// Region<> can shadow-track it for real corruption hunting.
namespace {
	struct GeneralAllocatorAdapter {
		Corium::Memory::Allocators::GeneralAllocator* m_Alloc;
		void* allocate(size_t v_Bytes, size_t v_Align) noexcept { return m_Alloc->allocate(v_Bytes, v_Align); }
		void deallocate(void* p_Ptr, size_t v_Bytes) noexcept { m_Alloc->deallocate(p_Ptr, v_Bytes); }
	};

	void runKerbecsRegionExperiment() {
		Kerbecs::Runtime::initShadowzone();

		GeneralAllocatorAdapter adapter{ &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0] };
		// Region range-checks every access against [regionBase, regionBase+size), so it
		// needs GeneralAllocator[0]'s real backing VA range, not an arbitrary size - this
		// is the same VirtualSegment AllocatorRegistry::initRegistry() handed to it.
		auto& backing = Corium::Memory::Internal::AllocatorRegistry::s_RuntimeCoreObjectsMemory[0];
		printf("[kerbecs] backing.m_Memory=%p m_TotalSize=%zu (%.2f MiB)\n",
			backing.m_Memory, backing.m_TotalSize, static_cast<double>(backing.m_TotalSize) / (1024.0 * 1024.0));
		fflush(stdout);
		Kerbecs::NormalRegion<GeneralAllocatorAdapter> region(adapter, backing.m_TotalSize, 4096, backing.m_Memory);
		printf("[kerbecs] Region constructed, initialized=%d\n", region.initialized());
		fflush(stdout);

		if (!region.initialized()) {
			printf("[kerbecs] Region failed to initialize\n");
			Kerbecs::Runtime::teardownShadowzone();
			return;
		}

		auto handle = region.allocate<int>();
		if (!handle) {
			printf("[kerbecs] allocate<int>() failed\n");
			Kerbecs::Runtime::teardownShadowzone();
			return;
		}
		region.construct(handle, 42);
		printf("[kerbecs] allocated+constructed int, value=%d\n", *handle);

		region.destroy(handle);
		printf("[kerbecs] destroyed - now deliberately touching the freed handle to check UAF detection\n");

		volatile int probe = *handle; // deliberate use-after-free
		(void)probe;

		Kerbecs::Violation v{};
		bool caught = false;
		while (Kerbecs::popViolation(v)) {
			caught = true;
			printf("[kerbecs] violation kind=%d address=%p\n", static_cast<int>(v.m_Kind), v.m_Address);
		}
		printf("[kerbecs] UAF %s\n", caught ? "DETECTED - Region wrap works" : "NOT detected - something's wrong with the wrap");

		Kerbecs::Runtime::teardownShadowzone();
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

int main() {
	printf("=== SpectraLauncher: window test via Corium DefaultThreadFactory ===\n\n");

	Corium::CoriumRuntime::initRuntime();
	printf("[main] Runtime initialised.\n");

	runKerbecsRegionExperiment();

	auto closure = Corium::Core::Utils::buildClosure<void()>(runWindowThread, 0u);

	Corium::Core::Factory::DefaultThreadFactory factory;
	/*
	const Corium::Core::ThreadHandle handle = factory.createAndStart(std::move(closure), "WindowThread");

	printf("[main] Window thread launched - waiting for it to finish...\n");
	Corium::Core::NativeThread::joinThread(handle);
	Corium::Core::NativeThread::closeHandle(handle);
	*/
	printf("\n[main] Window thread finished. Test complete.\n");
	return 0;
}
