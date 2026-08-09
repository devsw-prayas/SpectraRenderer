#include "SpectraLauncher.h"

#include <CoriumRuntime.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <ThreadUtils.h>
#include <CoriumUtility.h>

#include <PlatformWindowing.h>
#include <WindowUtils.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#include <cstdio>

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
	printf("[window] created and shown - close it to exit.\n");

	bool running = true;
	while (running) {
		MSG msg;
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		WindowEvent event;
		while (PlatformWindow::pollEvent(handle, event)) {
			if (event.m_Type == WindowEventType::CLOSE) {
				printf("[window] Close received - destroying.\n");
				PlatformWindow::destroy(handle);
				running = false;
			}
		}
	}

	PlatformWindow::shutdown();
}

int main() {
	printf("=== SpectraLauncher: window test via Corium DefaultThreadFactory ===\n\n");

	Corium::CoriumRuntime::initRuntime();
	printf("[main] Runtime initialised.\n");

	auto closure = Corium::Core::Utils::buildClosure<void()>(runWindowThread, 0u);

	Corium::Core::Factory::DefaultThreadFactory factory;
	const Corium::Core::ThreadHandle handle = factory.createAndStart(std::move(closure), "WindowThread");

	printf("[main] Window thread launched - waiting for it to finish...\n");
	Corium::Core::NativeThread::joinThread(handle);
	Corium::Core::NativeThread::closeHandle(handle);

	printf("\n[main] Window thread finished. Test complete.\n");
	return 0;
}
