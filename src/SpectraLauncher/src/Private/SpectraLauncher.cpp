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
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <malloc.h>

#include <StlMemOps.h>

// Experiment: Kerbecs stays out of Corium proper (Corium is meant to be a
// standalone, dependency-free library) - this wraps one of Corium's own
// allocator instances from the application side instead, to see whether
// Region<> can shadow-track it for real corruption hunting.
namespace {
	void runMemsetBenchmark() {
		// Pin to one core (logical proc 1 = a P-core on this box) so Thread Director
		// can't migrate mid-run. Restored before main() continues.
		constexpr DWORD_PTR benchCore = 1;
		const HANDLE benchThread = GetCurrentThread();
		const DWORD_PTR prevAffinity = SetThreadAffinityMask(benchThread, DWORD_PTR(1) << benchCore);

		using MemsetFn = void* (*)(void*, int, size_t);
		const MemsetFn libcMemset = &std::memset;
		const MemsetFn stormMemset = [](void* p_Dst, int v_Val, size_t v_Size) -> void* {
			Stl::Memory::memSet(p_Dst, static_cast<uint8_t>(v_Val), v_Size);
			return p_Dst;
		};

		struct Case { const char* m_Name; MemsetFn m_Fn; };
		const Case cases[] = { { "std::memset", libcMemset }, { "StormSTL", stormMemset } };

		// One size per cache tier: 32 KiB stays L1-resident (measures the store loop
		// itself), 1 MiB spills to L2/L3, 64 MiB is pure DRAM write bandwidth.
		constexpr size_t sizes[] = { 32ull * 1024, 256ull * 1024, 64ull * 1024 * 1024 };
		constexpr size_t bytesPerSample = 512ull * 1024 * 1024;   // fixed work per timed sample
		constexpr int repeats = 100;

		printf("[bench] backend=%s  NT-jump=%zu KiB  repeats=%d (min shown)\n",
			Stl::V::VIntrospect<Stl::V::preferredBackend()>::kName,
			static_cast<size_t>(STL_NT_JUMP_SIZE) / 1024, repeats);

		volatile uint8_t sink = 0;

		for (const size_t size : sizes) {
			const size_t inner = bytesPerSample / size;
			const char* tier = size <= 64ull * 1024 ? "L1" : size <= 4ull * 1024 * 1024 ? "L2/L3" : "DRAM";

			for (const Case& caseEntry : cases) {
				void* buffer = _aligned_malloc(size, 64);
				if (!buffer) { printf("[bench] alloc failed\n"); return; }

				// Untimed warm-up: every page faulted in, caches primed exactly as
				// they'll be during the timed runs. Both cases pay this equally.
				caseEntry.m_Fn(buffer, 0x5A, size);

				long long bestUs = -1;
				for (int rep = 0; rep < repeats; ++rep) {
					const auto t0 = std::chrono::steady_clock::now();
					for (size_t i = 0; i < inner; ++i)
						caseEntry.m_Fn(buffer, 0xA5, size);
					const auto t1 = std::chrono::steady_clock::now();
					sink ^= *static_cast<const uint8_t*>(buffer);
					const long long us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
					if (bestUs < 0 || us < bestUs) bestUs = us;
				}

				const double gib = (static_cast<double>(size) * inner * 1e6) /
					(static_cast<double>(bestUs) * 1024.0 * 1024.0 * 1024.0);
				printf("[bench] %-5s %8zu KiB  %-11s  %8lld us  %6.2f GiB/s\n",
					tier, size / 1024, caseEntry.m_Name, bestUs, gib);

				_aligned_free(buffer);
			}
		}
		(void)sink;

		if (prevAffinity != 0)
			SetThreadAffinityMask(benchThread, prevAffinity);
	}

	void runMemcpyBenchmark() {
		constexpr DWORD_PTR benchCore = 1;
		const HANDLE benchThread = GetCurrentThread();
		const DWORD_PTR prevAffinity = SetThreadAffinityMask(benchThread, DWORD_PTR(1) << benchCore);

		using MemcpyFn = void* (*)(void*, const void*, size_t);
		const MemcpyFn libcMemcpy = &std::memcpy;
		const MemcpyFn stormMemcpy = [](void* p_Dst, const void* p_Src, size_t v_Size) -> void* {
			Stl::Memory::memCopy(p_Dst, p_Src, v_Size);
			return p_Dst;
		};

		struct Case { const char* m_Name; MemcpyFn m_Fn; };
		const Case cases[] = { { "std::memcpy", libcMemcpy }, { "StormSTL", stormMemcpy } };

		// Sizes are per-buffer; the working set is 2x (src + dst), chosen to mirror
		// the memset footprints: 16 KiB -> 32 KiB in L1, 512 KiB -> 1 MiB in L2/L3,
		// 32 MiB -> 64 MiB straight to DRAM.
		constexpr size_t sizes[] = { 16ull * 1024, 128ull * 1024, 32ull * 1024 * 1024 };
		constexpr size_t bytesPerSample = 512ull * 1024 * 1024;
		constexpr int repeats = 100;

		printf("[bench] memcpy  backend=%s  NT-jump=%zu KiB  repeats=%d (min shown)\n",
			Stl::V::VIntrospect<Stl::V::preferredBackend()>::kName,
			static_cast<size_t>(STL_NT_JUMP_SIZE) / 1024, repeats);

		volatile uint8_t sink = 0;

		for (const size_t size : sizes) {
			const size_t inner = bytesPerSample / size;
			const char* tier = size <= 32ull * 1024 ? "L1" : size <= 2ull * 1024 * 1024 ? "L2/L3" : "DRAM";

			for (const Case& caseEntry : cases) {
				void* dst = _aligned_malloc(size, 64);
				void* src = _aligned_malloc(size, 64);
				if (!dst || !src) { printf("[bench] alloc failed\n"); _aligned_free(dst); _aligned_free(src); return; }

				std::memset(src, 0x5A, size);
				caseEntry.m_Fn(dst, src, size);   // warm-up: fault + prime both buffers

				long long bestUs = -1;
				for (int rep = 0; rep < repeats; ++rep) {
					const auto t0 = std::chrono::steady_clock::now();
					for (size_t i = 0; i < inner; ++i)
						caseEntry.m_Fn(dst, src, size);
					const auto t1 = std::chrono::steady_clock::now();
					sink ^= *static_cast<const uint8_t*>(dst);
					const long long us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
					if (bestUs < 0 || us < bestUs) bestUs = us;
				}

				const double gib = (static_cast<double>(size) * inner * 1e6) /
					(static_cast<double>(bestUs) * 1024.0 * 1024.0 * 1024.0);
				printf("[bench] %-5s %8zu KiB  %-11s  %8lld us  %6.2f GiB/s\n",
					tier, size / 1024, caseEntry.m_Name, bestUs, gib);

				_aligned_free(dst);
				_aligned_free(src);
			}
		}
		(void)sink;

		if (prevAffinity != 0)
			SetThreadAffinityMask(benchThread, prevAffinity);
	}

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

static void runScanChecks() {
	printf("[scan] correctness: memEqual / memCompare / memFindByte vs CRT\n");

	int pass = 0;
	int fail = 0;
	const auto sgn = [](int v_X) { return (v_X > 0) - (v_X < 0); };
	const auto check = [&](bool v_Ok, const char* p_What, size_t v_N, long v_Pos) {
		if (v_Ok) { ++pass; return; }
		++fail;
		printf("[scan] FAIL  %-12s n=%zu pos=%ld\n", p_What, v_N, v_Pos);
	};

	constexpr size_t kCap = 200003;
	auto* a = static_cast<uint8_t*>(malloc(kCap));
	auto* b = static_cast<uint8_t*>(malloc(kCap));
	if (!a || !b) { printf("[scan] alloc failed\n"); free(a); free(b); return; }

	const size_t sizes[] = { 1, 7, 31, 32, 33, 64, 100, 255, 256, 257, 512, 1000, 4096, 100003 };
	const long spots[] = { 0, 1, 30, 31, 32, 100, 254, 255, 256, -1 /* n-1, patched below */ };

	for (const size_t n : sizes) {
		for (size_t i = 0; i < n; ++i) a[i] = b[i] = static_cast<uint8_t>((i * 131 + 7) & 0xFF);

		check(Stl::Memory::memEqual(a, b, n), "eq-equal", n, -1);
		check(sgn(Stl::Memory::memCompare(a, b, n)) == 0, "cmp-equal", n, -1);

		for (long spot : spots) {
			const long pos = spot < 0 ? static_cast<long>(n) - 1 : spot;
			if (pos < 0 || static_cast<size_t>(pos) >= n) continue;

			const uint8_t save = b[pos];
			b[pos] = static_cast<uint8_t>(save ^ 0xFF);
			check(!Stl::Memory::memEqual(a, b, n), "eq-diff", n, pos);
			check(sgn(Stl::Memory::memCompare(a, b, n)) == sgn(std::memcmp(a, b, n)), "cmp-diff", n, pos);
			b[pos] = save;
		}

		for (size_t i = 0; i < n; ++i) a[i] = static_cast<uint8_t>(1 + ((i * 7) % 200));
		constexpr uint8_t needle = 250;
		check(Stl::Memory::memFindByte(static_cast<const void*>(a), n, needle) == nullptr, "find-absent", n, -1);

		for (long spot : spots) {
			const long pos = spot < 0 ? static_cast<long>(n) - 1 : spot;
			if (pos < 0 || static_cast<size_t>(pos) >= n) continue;

			const uint8_t save = a[pos];
			a[pos] = needle;
			const void* got = Stl::Memory::memFindByte(static_cast<const void*>(a), n, needle);
			const void* want = std::memchr(a, needle, n);
			check(got == want, "find", n, pos);
			a[pos] = save;
		}
	}

	free(a);
	free(b);
	printf("[scan] %d passed, %d failed\n", pass, fail);
}

static void runScanBenchmark() {
	constexpr DWORD_PTR benchCore = 1;
	const HANDLE benchThread = GetCurrentThread();
	const DWORD_PTR prevAffinity = SetThreadAffinityMask(benchThread, DWORD_PTR(1) << benchCore);

	constexpr size_t bytesPerSample = 512ull * 1024 * 1024;
	constexpr int repeats = 100;
	printf("[bench] scanners  repeats=%d (min shown, full-scan worst case)\n", repeats);

	volatile uint64_t sink = 0;
	const auto gib = [](size_t v_Size, size_t v_Inner, long long v_Us) {
		return (static_cast<double>(v_Size) * v_Inner * 1e6) / (static_cast<double>(v_Us) * 1073741824.0);
	};
	const auto timeIt = [&](size_t v_Inner, auto&& v_Fn) -> long long {
		long long best = -1;
		for (int rep = 0; rep < repeats; ++rep) {
			const auto t0 = std::chrono::steady_clock::now();
			for (size_t i = 0; i < v_Inner; ++i) sink ^= static_cast<uint64_t>(v_Fn());
			const auto t1 = std::chrono::steady_clock::now();
			const long long us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
			if (best < 0 || us < best) best = us;
		}
		return best;
	};

	// memEqual / memCompare: buffers identical -> whole range is scanned.
	{
		const size_t sizes[] = { 16ull * 1024, 128ull * 1024, 32ull * 1024 * 1024 };
		for (const size_t size : sizes) {
			const char* tier = size <= 32ull * 1024 ? "L1" : size <= 2ull * 1024 * 1024 ? "L2/L3" : "DRAM";
			const size_t inner = bytesPerSample / size;
			void* a = _aligned_malloc(size, 64);
			void* b = _aligned_malloc(size, 64);
			if (!a || !b) { printf("[bench] alloc failed\n"); _aligned_free(a); _aligned_free(b); return; }
			std::memset(a, 0x5A, size);
			std::memset(b, 0x5A, size);

			const long long eqCrt = timeIt(inner, [&] { return std::memcmp(a, b, size) == 0; });
			const long long eqStl = timeIt(inner, [&] { return Stl::Memory::memEqual(a, b, size); });
			const long long cmCrt = timeIt(inner, [&] { return std::memcmp(a, b, size); });
			const long long cmStl = timeIt(inner, [&] { return Stl::Memory::memCompare(a, b, size); });

			printf("[bench] %-5s %8zu KiB  std::memcmp   %8lld us  %6.2f GiB/s\n", tier, size / 1024, eqCrt, gib(size, inner, eqCrt));
			printf("[bench] %-5s %8zu KiB  StormEqual    %8lld us  %6.2f GiB/s\n", tier, size / 1024, eqStl, gib(size, inner, eqStl));
			printf("[bench] %-5s %8zu KiB  std::memcmp   %8lld us  %6.2f GiB/s  (cmp)\n", tier, size / 1024, cmCrt, gib(size, inner, cmCrt));
			printf("[bench] %-5s %8zu KiB  StormCompare  %8lld us  %6.2f GiB/s\n", tier, size / 1024, cmStl, gib(size, inner, cmStl));

			_aligned_free(a);
			_aligned_free(b);
		}
	}

	// memFindByte: sentinel never present -> whole range is scanned.
	{
		const size_t sizes[] = { 32ull * 1024, 256ull * 1024, 64ull * 1024 * 1024 };
		for (const size_t size : sizes) {
			const char* tier = size <= 64ull * 1024 ? "L1" : size <= 4ull * 1024 * 1024 ? "L2/L3" : "DRAM";
			const size_t inner = bytesPerSample / size;
			auto* p = static_cast<uint8_t*>(_aligned_malloc(size, 64));
			if (!p) { printf("[bench] alloc failed\n"); return; }
			std::memset(p, 0x01, size);
			constexpr uint8_t needle = 0xFF;

			const long long crt = timeIt(inner, [&] { return reinterpret_cast<uintptr_t>(std::memchr(p, needle, size)); });
			const long long stl = timeIt(inner, [&] { return reinterpret_cast<uintptr_t>(Stl::Memory::memFindByte(static_cast<const void*>(p), size, needle)); });

			printf("[bench] %-5s %8zu KiB  std::memchr   %8lld us  %6.2f GiB/s\n", tier, size / 1024, crt, gib(size, inner, crt));
			printf("[bench] %-5s %8zu KiB  StormFind     %8lld us  %6.2f GiB/s\n", tier, size / 1024, stl, gib(size, inner, stl));

			_aligned_free(p);
		}
	}

	(void)sink;
	if (prevAffinity != 0)
		SetThreadAffinityMask(benchThread, prevAffinity);
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
	runMemsetBenchmark();
	printf("\n");
	runMemcpyBenchmark();
	printf("\n");
	runScanChecks();
	printf("\n");
	runScanBenchmark();

	Corium::CoriumRuntime::initRuntime();
	printf("[main] Runtime initialised.\n");
}
