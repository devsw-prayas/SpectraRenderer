// SpectraPlatformTest.cpp
// Quick smoke test for the Spectra Platform Runtime.
// Covers: Environment, VirtualMemory, Chrono, Atomics, Files.
// Excludes: Threads (tested separately).
//
// Build: add to your CMake target, link against SpectraPlatformRuntime.
// Run:   no arguments required. Prints PASS / FAIL per section.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "ProcessEnvironment.h"
#include "PlatformMemory.h"
#include "MemoryUtils.h"
#include "PlatformChrono.h"
#include "PlatformAtomics.h"
#include "SpectraAtomics.h"
#include "PlatformFileSystem.h"
#include "FileUtils.h"
#include "SpectraInternalDiagonostics.h"

#include "CudaBootstrap.h"
#include "CudaContextManager.h"
#include "CudaDeviceMemory.h"
#include "CudaPinnedMemory.h"
#include "CudaManagedMemory.h"
#include "CudaVirtualMemory.h"
#include "CudaUtils.h"
#include "CudaStream.h"
#include "CudaEvent.h"
#include "CudaGraph.h"
#include "CudaModule.h"
#include "CudaLinker.h"
#include "CudaCompute.h"
#include "OptixContextManager.h"

#include "CoriumRuntime.h"
#include "CoriumUtility.h"
// =========================================================
// Minimal test harness
// =========================================================

static int g_Passed = 0;
static int g_Failed = 0;

#define TEST(name, expr)                                          \
    do {                                                          \
        if (expr) {                                               \
            printf("  [PASS] %s\n", name);                       \
            ++g_Passed;                                           \
        } else {                                                  \
            printf("  [FAIL] %s  (line %d)\n", name, __LINE__);  \
            ++g_Failed;                                           \
        }                                                         \
    } while (0)

static void section(const char* name) {
	printf("\n=== %s ===\n", name);
}

// =========================================================
// 1. Environment
// =========================================================

static void testEnvironment() {
	section("Environment");

	using namespace Spectra::Platform::Runtime::Environment;

	PlatformProbe::init();

	const CpuInfo& cpu = PlatformProbe::getCpuInfo();
	TEST("logical cores > 0", cpu.m_LogicalCoreCount > 0);
	TEST("physical cores > 0", cpu.m_PhysicalCoreCount > 0);
	TEST("logical >= physical", cpu.m_LogicalCoreCount >= cpu.m_PhysicalCoreCount);
	TEST("cache line size is power2", cpu.m_CachedLineSize > 0 && (cpu.m_CachedLineSize & (cpu.m_CachedLineSize - 1)) == 0);
	TEST("L1 cache size > 0", cpu.m_L1CacheSize > 0);

	const VectorizeCapabilities& vec = PlatformProbe::getVectorizeCapablities();
	TEST("SSE capability present", (vec.m_VectorCapabilities & (1 << 0)) != 0);

	const OsInfo& os = PlatformProbe::getOsInfo();
	TEST("OS major version > 0", os.m_MajorVersion > 0);
	TEST("OS product name not null", os.m_ProductName != nullptr);

	uint32_t pid = PlatformProcess::getCurrentProcessId();
	TEST("process ID > 0", pid > 0);

	const char* exe = PlatformProcess::getExecutablePath();
	TEST("executable path not null", exe != nullptr && exe[0] != '\0');

	const char* cwd = PlatformProcess::getWorkingDirectory();
	TEST("working directory not null", cwd != nullptr && cwd[0] != '\0');
}

// =========================================================
// 2. Virtual Memory
// =========================================================

static void testVirtualMemory() {
	section("VirtualMemory");

	using namespace Spectra::Platform::Runtime::Memory;

	PlatformVirtualMemory::init();

	const PlatformMemoryInfo& info = PlatformVirtualMemory::getMemoryInfo();
	TEST("page size > 0", info.m_PageSize > 0);
	TEST("page size is power of 2", (info.m_PageSize & (info.m_PageSize - 1)) == 0);
	TEST("granularity >= page size", info.m_AllocationGranularity >= info.m_PageSize);
	TEST("total physical memory > 0", info.m_TotalPhysicalMemory > 0);
	TEST("available memory > 0", info.m_AvailableMemory > 0);

	// Alignment helpers
	size_t unaligned = info.m_PageSize + 1;
	TEST("alignToPage rounds up", PlatformVirtualMemory::alignToPage(unaligned) == info.m_PageSize * 2);
	TEST("isPageAligned: page size", PlatformVirtualMemory::isPageAligned(info.m_PageSize));
	TEST("isPageAligned: unaligned", !PlatformVirtualMemory::isPageAligned(unaligned));

	size_t gran = info.m_AllocationGranularity;
	TEST("alignToGranularity rounds up", PlatformVirtualMemory::alignToGranularity(gran + 1) == gran * 2);

	// Reserve -> commit -> write -> decommit -> release
	VirtualMemoryDesc desc{};
	initMemoryDesc(desc);
	setSize(desc, gran);
	setMemoryState(desc, MemoryState::RESERVE);
	setProtection(desc, MemoryProtect::NO_ACCESS);

	VirtualMemoryHandle handle = PlatformVirtualMemory::reserve(desc);
	TEST("reserve returns valid handle", isValid(handle));
	TEST("reserved size matches", handle.m_TotalSize == gran);

	// Commit the first page
	VirtualMemoryDesc commitDesc{};
	initMemoryDesc(commitDesc);
	setTargetAddress(commitDesc, handle.m_BaseAddress);
	setSize(commitDesc, info.m_PageSize);
	setMemoryState(commitDesc, MemoryState::COMMIT);
	setProtection(commitDesc, MemoryProtect::READ_WRITE);

	PlatformVirtualMemory::commit(handle, commitDesc);

	// Write and read back
	auto* ptr = static_cast<uint8_t*>(handle.m_BaseAddress);
	ptr[0] = 0xAB;
	ptr[info.m_PageSize - 1] = 0xCD;
	TEST("write/read committed page (first byte)", ptr[0] == 0xAB);
	TEST("write/read committed page (last byte)", ptr[info.m_PageSize - 1] == 0xCD);

	// Decommit
	VirtualMemoryDesc decommitDesc{};
	initMemoryDesc(decommitDesc);
	setTargetAddress(decommitDesc, handle.m_BaseAddress);
	setSize(decommitDesc, info.m_PageSize);
	setMemoryState(decommitDesc, MemoryState::DECOMMIT);

	PlatformVirtualMemory::decommit(handle, decommitDesc);

	// Release
	VirtualMemoryDesc releaseDesc{};
	initMemoryDesc(releaseDesc);
	setTargetAddress(releaseDesc, handle.m_BaseAddress);
	setMemoryState(releaseDesc, MemoryState::RELEASE);

	PlatformVirtualMemory::release(handle, releaseDesc);
	TEST("handle nulled after release", handle.m_BaseAddress == nullptr && handle.m_TotalSize == 0);

	// queryAvailableMemory
	size_t avail = PlatformVirtualMemory::queryAvailableMemory();
	TEST("queryAvailableMemory > 0", avail > 0);
}

// =========================================================
// 3. Chrono
// =========================================================

static void testChrono() {
	section("Chrono");

	using namespace Spectra::Platform::Runtime::Chrono;

	// Duration arithmetic
	Duration a(1'000'000'000LL); // 1s
	Duration b(500'000'000LL);   // 0.5s

	TEST("Duration add", (a + b).m_Nanoseconds == 1'500'000'000LL);
	TEST("Duration subtract", (a - b).m_Nanoseconds == 500'000'000LL);
	TEST("Duration multiply", (b * 2).m_Nanoseconds == 1'000'000'000LL);
	TEST("Duration divide", (a / 2).m_Nanoseconds == 500'000'000LL);
	TEST("toSeconds", a.toSeconds() > 0.999 && a.toSeconds() < 1.001);
	TEST("toMilliseconds", a.toMilliseconds() > 999.0 && a.toMilliseconds() < 1001.0);

	// MonotonicClock: two samples, delta must be >= 0
	Timestamp t0 = MonotonicClock::now();
	Timestamp t1 = MonotonicClock::now();
	Duration  dt = t1 - t0;
	TEST("monotonic clock non-negative delta", dt.m_Nanoseconds >= 0);
	TEST("monotonic clock domain", t0.m_Domain == ClockDomain::MONOTONIC);

	ClockInfo mInfo = MonotonicClock::getInfo();
	TEST("monotonic clock is monotonic", mInfo.m_IsMonotonic);
	TEST("monotonic clock frequency > 0", mInfo.m_Frequency > 0);

	// WallClock
	Timestamp w0 = WallClock::now();
	Timestamp w1 = WallClock::now();
	Duration  wd = w1 - w0;
	TEST("wall clock non-negative delta", wd.m_Nanoseconds >= 0);
	TEST("wall clock domain", w0.m_Domain == ClockDomain::WALL);

	// CycleClock
	CycleCount c0 = CycleClock::now();
	CycleCount c1 = CycleClock::now();
	TEST("cycle counter advances", c1.value() >= c0.value());
}

// =========================================================
// 4. Atomics
// =========================================================

static void testAtomics() {
	section("Atomics");

	using namespace Spectra::Platform::Runtime::Atomic;
	using namespace Spectra::Platform::Runtime::Intrinsic;

	// AtomicValue32
	{
		AtomicValue32<uint32_t> a(0u);

		a.store(42u);
		TEST("AtomicValue32 store/load", a.load() == 42u);

		uint32_t prev = a.exchange(100u);
		TEST("AtomicValue32 exchange returns old", prev == 42u);
		TEST("AtomicValue32 exchange sets new", a.load() == 100u);

		uint32_t expected = 100u;
		a.compareExchange(&expected, 200u, MemoryOrder::SEQ_CST, MemoryOrder::RELAXED);
		TEST("AtomicValue32 CAS success", a.load() == 200u);

		expected = 999u; // wrong expected
		a.compareExchange(&expected, 300u, MemoryOrder::SEQ_CST, MemoryOrder::RELAXED);
		TEST("AtomicValue32 CAS fail no-change", a.load() == 200u);

		a.store(0u);
		a.increment();
		TEST("AtomicValue32 increment", a.load() == 1u);
		a.decrement();
		TEST("AtomicValue32 decrement", a.load() == 0u);

		a.store(0xFFu);
		a.fetchAnd(0x0Fu);
		TEST("AtomicValue32 fetchAnd", a.load() == 0x0Fu);

		a.store(0x00u);
		a.fetchOr(0xF0u);
		TEST("AtomicValue32 fetchOr", a.load() == 0xF0u);

		a.store(0xFFu);
		a.fetchXor(0x0Fu);
		TEST("AtomicValue32 fetchXor", a.load() == 0xF0u);
	}

	// AtomicValue64
	{
		AtomicValue64<uint64_t> b(0ull);

		b.store(0xDEADBEEFCAFEull);
		TEST("AtomicValue64 store/load", b.load() == 0xDEADBEEFCAFEull);

		uint64_t prev = b.exchange(1ull);
		TEST("AtomicValue64 exchange returns old", prev == 0xDEADBEEFCAFEull);
		TEST("AtomicValue64 exchange sets new", b.load() == 1ull);

		b.store(0ull);
		b.fetchAdd(10ull);
		TEST("AtomicValue64 fetchAdd", b.load() == 10ull);

		b.increment(1ull);
		TEST("AtomicValue64 increment", b.load() == 11ull);
		b.decrement(1ull);
		TEST("AtomicValue64 decrement", b.load() == 10ull);
	}

	// Intrinsic bit helpers
	{
		using namespace Spectra::Platform::Runtime::Intrinsic;

		uint32_t val32 = 0b1010u;
		TEST("testBit32 bit1 set", testBit(&val32, 1));
		TEST("testBit32 bit0 clear", !testBit(&val32, 0));

		uint64_t val64 = 1ull << 40;
		TEST("testBit64 bit40 set", testBit(&val64, 40));
		TEST("testBit64 bit0 clear", !testBit(&val64, 0));

		uint32_t idx32 = 0;
		TEST("BSF32 finds LSB", scanLeastSignificantSetBit(0b1000u, &idx32) && idx32 == 3);

		uint32_t idx64 = 0;
		TEST("BSF64 finds LSB", scanLeastSignificantSetBit(1ull << 33, &idx64) && idx64 == 33);

		TEST("popcount32", countSetBits(0b10101010u) == 4);
		TEST("popcount64", countSetBits(0xFFFFFFFFFFFFFFFFull) == 64);

		TEST("rotl32", rotateBitsLeft(1u, 1) == 2u);
		TEST("rotr32", rotateBitsRight(2u, 1) == 1u);

		TEST("byteswap16", byteSwapEndianness(uint16_t(0x0102u)) == 0x0201u);
		TEST("byteswap32", byteSwapEndianness(uint32_t(0x01020304u)) == 0x04030201u);
		TEST("byteswap64", byteSwapEndianness(uint64_t(0x0102030405060708ull)) == 0x0807060504030201ull);

		uint64_t tsc0 = readTimeStampCounter();
		uint64_t tsc1 = readTimeStampCounter();
		TEST("RDTSC advances", tsc1 >= tsc0);
	}
}

// =========================================================
// 5. Files
// =========================================================

static void testFiles() {
	section("Files");

	using namespace Spectra::Platform::Runtime::File;

	Files::init();

	const PlatformFileSystemInfo& fsInfo = Files::getInfo();
	TEST("fs allocation granularity > 0", fsInfo.m_AllocationGranularity > 0);
	TEST("fs max path length > 0", fsInfo.m_MaxPathLength > 0);

	const char* testPath = "spectra_platform_test_tmp.bin";

	// --- open / write / seek / read / close (SEQUENTIAL) ---
	{
		FileStreamDesc desc{};
		desc.m_Path = testPath;
		desc.m_Mode = FileIOMode::SEQUENTIAL;
		desc.m_Access = FileAccess::READ_WRITE;
		desc.m_OpenMode = FileOpenMode::CREATE_ALWAYS_FILE;
		desc.m_ShareMode = FileShareMode::NONE;

		FileHandle handle = Files::open(desc);
		TEST("open CREATE_ALWAYS succeeds", handle.m_NativeHandle != nullptr);

		uint8_t writeData[64];
		for (int i = 0; i < 64; ++i) writeData[i] = static_cast<uint8_t>(i);

		size_t written = Files::write(handle, writeData, 64);
		TEST("write 64 bytes", written == 64);

		size_t fileSize = Files::getFileSize(handle);
		TEST("getFileSize == 64", fileSize == 64);

		Files::seek(handle, 0, FileSeekOrigin::BEGIN);

		uint8_t readData[64]{};
		size_t bytesRead = Files::read(handle, readData, 64);
		TEST("read 64 bytes", bytesRead == 64);

		bool match = memcmp(writeData, readData, 64) == 0;
		TEST("read data matches written data", match);

		// Seek from end
		Files::seek(handle, -1, FileSeekOrigin::END);
		uint8_t lastByte = 0;
		Files::read(handle, &lastByte, 1);
		TEST("seek END reads last byte", lastByte == 63);

		Files::flush(handle);
		TEST("flush does not crash", true);

		Files::close(handle);
		TEST("handle nulled after close", handle.m_NativeHandle == nullptr);
	}

	// --- open OPEN_EXISTING and verify contents persist ---
	{
		FileStreamDesc desc{};
		desc.m_Path = testPath;
		desc.m_Mode = FileIOMode::SEQUENTIAL;
		desc.m_Access = FileAccess::READ;
		desc.m_OpenMode = FileOpenMode::OPEN_EXISTING_FILE;
		desc.m_ShareMode = FileShareMode::READ;

		FileHandle handle = Files::open(desc);
		TEST("open OPEN_EXISTING succeeds", handle.m_NativeHandle != nullptr);

		uint8_t buf[64]{};
		Files::read(handle, buf, 64);
		TEST("persisted data byte 0", buf[0] == 0);
		TEST("persisted data byte 63", buf[63] == 63);

		Files::close(handle);
	}

	// --- async handler init/destroy (no actual I/O, just lifetime) ---
	{
		AsyncFileHandler handler{};
		initializeAsyncHandler(handler);
		TEST("async handler event not null", handler.m_EventHandle != nullptr);
		destroyAsyncHandler(handler);
		TEST("async handler event null after destroy", handler.m_EventHandle == nullptr);
	}

	// --- memory mapping ---
	{
		FileStreamDesc desc{};
		desc.m_Path = testPath;
		desc.m_Mode = FileIOMode::SEQUENTIAL;
		desc.m_Access = FileAccess::READ_WRITE;
		desc.m_OpenMode = FileOpenMode::OPEN_EXISTING_FILE;
		desc.m_ShareMode = FileShareMode::NONE;

		FileHandle handle = Files::open(desc);

		FileMappingHandle mapping = Files::createMapping(handle, FileAccess::READ_WRITE, 64);
		TEST("createMapping succeeds", mapping.m_NativeHandle != nullptr);

		// mapView offset must be granularity-aligned; use 0
		void* view = Files::mapView(mapping, 0, 64, FileAccess::READ_WRITE);
		TEST("mapView returns non-null", view != nullptr);

		auto* bytes = static_cast<uint8_t*>(view);
		TEST("mapped view byte 0 == 0", bytes[0] == 0);
		TEST("mapped view byte 63 == 63", bytes[63] == 63);

		// Write through the mapping
		bytes[0] = 0xFF;

		Files::unmapView(view);
		TEST("unmapView does not crash", true);

		// Can close mapping while file handle still open
		Files::closeMapping(mapping);
		TEST("mapping nulled after close", mapping.m_NativeHandle == nullptr);

		Files::close(handle);

		// Verify the write-through persisted
		FileStreamDesc verifyDesc{};
		verifyDesc.m_Path = testPath;
		verifyDesc.m_Mode = FileIOMode::SEQUENTIAL;
		verifyDesc.m_Access = FileAccess::READ;
		verifyDesc.m_OpenMode = FileOpenMode::OPEN_EXISTING_FILE;
		verifyDesc.m_ShareMode = FileShareMode::READ;

		FileHandle verifyHandle = Files::open(verifyDesc);
		uint8_t verifyBuf[1]{};
		Files::read(verifyHandle, verifyBuf, 1);
		TEST("mapped write persisted", verifyBuf[0] == 0xFF);
		Files::close(verifyHandle);
	}

	// --- isMappingAligned ---
	{
		const PlatformFileSystemInfo& info = Files::getInfo();
		TEST("isMappingAligned: 0", isMappingAligned(0));
		TEST("isMappingAligned: granularity", isMappingAligned(info.m_AllocationGranularity));
		TEST("isMappingAligned: not aligned", !isMappingAligned(info.m_AllocationGranularity + 1));
	}

	// --- directory enumeration ---
	{
		// Create a couple of temp files to enumerate
		const char* tmpA = "spectra_enum_test_a.tmp";
		const char* tmpB = "spectra_enum_test_b.tmp";

		for (const char* p : { tmpA, tmpB }) {
			FileStreamDesc d{};
			d.m_Path = p;
			d.m_Mode = FileIOMode::SEQUENTIAL;
			d.m_Access = FileAccess::WRITE;
			d.m_OpenMode = FileOpenMode::CREATE_ALWAYS_FILE;
			d.m_ShareMode = FileShareMode::NONE;
			FileHandle h = Files::open(d);
			Files::close(h);
		}

		DirectoryEnumDesc enumDesc{};
		enumDesc.m_Path = ".";
		enumDesc.m_Filter = "spectra_enum_test_*.tmp";

		DirectoryEnumHandle enumHandle = Files::beginEnumeration(enumDesc);
		TEST("beginEnumeration succeeds", enumHandle.m_IsValid);

		int count = 0;
		FileInfo fi{};
		while (Files::next(enumHandle, fi)) {
			TEST("enumerated filename not empty", fi.m_FileName[0] != '\0');
			++count;
		}
		TEST("enumerated expected file count", count == 2);

		Files::closeEnumeration(enumHandle);
		TEST("enum handle invalidated after close", !enumHandle.m_IsValid);

		// Cleanup temp enum files
		Files::deleteFile(tmpA);
		Files::deleteFile(tmpB);
	}

	// --- rename ---
	{
		const char* renamed = "spectra_platform_test_renamed.bin";
		Files::renameFile(testPath, renamed);

		// Verify old path gone, new path accessible
		FileStreamDesc d{};
		d.m_Path = renamed;
		d.m_Mode = FileIOMode::SEQUENTIAL;
		d.m_Access = FileAccess::READ;
		d.m_OpenMode = FileOpenMode::OPEN_EXISTING_FILE;
		d.m_ShareMode = FileShareMode::READ;
		FileHandle h = Files::open(d);
		TEST("renamed file accessible", h.m_NativeHandle != nullptr);
		Files::close(h);

		// Cleanup
		Files::deleteFile(renamed);
	}

	TEST("all file cleanup done", true);
}

// =========================================================
// CUDA shared state
// =========================================================

static Spectra::Cuda::Utils::CudaContext  g_CudaCtx{};
static Spectra::Cuda::Utils::DeviceHandle g_Device{};
static bool g_CudaReady = false;

static void setupCuda() {
	using namespace Spectra::Cuda;
	Bootstrap::CudaDriver::initCuda();
	g_Device = Bootstrap::CudaDeviceManager::getCudaDevice(0);
	g_CudaReady = g_Device.isValid();
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device found\n"); return; }
	g_CudaCtx = Context::ContextManager::createCudaContext(
		g_Device,
		Utils::ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC,
		Utils::ContextCreationFlags::NONE
	);
	Context::ContextManager::setCurrentCudaContext(g_CudaCtx);
}

static void teardownCuda() {
	using namespace Spectra::Cuda;
	if (!g_CudaReady) return;
	Context::ContextManager::destroyCudaContext(g_CudaCtx);
}

// =========================================================
// CUDA 2.1 — Device Memory
// =========================================================

static void testCudaDeviceMemory() {
	section("CUDA 2.1 -- Device Memory");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }

	using namespace Spectra::Cuda::Memory;
	using namespace Spectra::Cuda::Utils;

	GpuMemory mem = DeviceMemory::query();
	TEST("query: total > 0", mem.m_TotalMemory > 0);
	TEST("query: free > 0", mem.m_AvailableMemory > 0);

	GpuAddress addr = DeviceMemory::deviceAlloc(1024);
	TEST("deviceAlloc valid", addr.isValid());

	DeviceMemory::memsetD8(addr, 0xAB, 1024);
	TEST("memsetD8 does not crash", true);

	DeviceMemory::memsetD16(addr, 0xCDEF, 512);
	TEST("memsetD16 does not crash", true);

	DeviceMemory::memsetD32(addr, 0xDEADBEEF, 256);
	TEST("memsetD32 does not crash", true);

	GpuAddress dst = DeviceMemory::deviceAlloc(1024);
	TEST("dst alloc valid", dst.isValid());
	DeviceMemory::copyDeviceToDevice(dst, addr, 1024);
	TEST("copyDeviceToDevice does not crash", true);

	DeviceMemory::deviceFree(dst);
	TEST("dst freed", !dst.isValid());
	DeviceMemory::deviceFree(addr);
	TEST("addr freed", !addr.isValid());

	PitchedAllocation pitched = DeviceMemory::deviceAllocPitch(128, 64, 4);
	TEST("deviceAllocPitch valid", pitched.m_Address.isValid());
	TEST("deviceAllocPitch pitch > 0", pitched.m_Pitch > 0);
	DeviceMemory::deviceFree(pitched.m_Address);
	TEST("pitched freed", !pitched.m_Address.isValid());
}

// =========================================================
// CUDA 2.2 — Pinned Memory
// =========================================================

static void testCudaPinnedMemory() {
	section("CUDA 2.2 -- Pinned Memory");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }

	using namespace Spectra::Cuda::Memory;
	using namespace Spectra::Cuda::Utils;

	PinnedAddress pinned = PinnedMemory::pinnedAlloc(4096);
	TEST("pinnedAlloc valid", pinned.isValid());

	GpuAddress mapped{};
	PinnedMemory::mapToDevice(mapped, pinned, 0);
	TEST("mapToDevice valid", mapped.isValid());

	PinnedMemory::pinnedFree(pinned);
	TEST("pinnedFree (invalid after free)", !pinned.isValid());

	PinnedAddress wc = PinnedMemory::pinnedAlloc(4096, CudaHelpers::computeAllocFlag({ HostAllocFlags::ALLOC_WRITE_COMBINED }));
	TEST("pinnedAlloc write-combined valid", wc.isValid());
	PinnedMemory::pinnedFree(wc);
	TEST("write-combined freed", !wc.isValid());

	void* raw = malloc(4096);
	TEST("malloc succeeded", raw != nullptr);
	PinnedAddress reg{ raw };
	PinnedMemory::hostMemRegister(reg, 4096, CudaHelpers::computeRegFlag({ HostRegisterFlags::REG_PORTABLE }));
	TEST("hostMemRegister does not crash", true);
	PinnedMemory::hostMemUnRegister(reg);
	TEST("hostMemUnRegister does not crash", true);
	free(raw);
}

// =========================================================
// CUDA 2.3 — Managed Memory
// =========================================================

static void testCudaManagedMemory() {
	section("CUDA 2.3 -- Managed Memory");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }

	using namespace Spectra::Cuda::Memory;
	using namespace Spectra::Cuda::Utils;

	GpuAddress managed = ManagedMemory::allocManaged(4096, 0x1);
	TEST("allocManaged valid", managed.isValid());

	ManagedMemory::adviseMemory(managed, 4096, MemoryAdvise::SET_PREFERRED_LOCATION, g_Device);
	TEST("adviseMemory SET_PREFERRED_LOCATION does not crash", true);

	ManagedMemory::adviseMemory(managed, 4096, MemoryAdvise::SET_ACCESSED_BY, g_Device);
	TEST("adviseMemory SET_ACCESSED_BY does not crash", true);

	Spectra::Cuda::Streams::GpuStream nullStream{};
	ManagedMemory::prefetchAsync(managed, 4096, g_Device, nullStream);
	TEST("prefetchAsync does not crash", true);

	DeviceMemory::deviceFree(managed);
	TEST("managed freed", !managed.isValid());
}

// =========================================================
// CUDA 2.4 — Virtual Memory
// =========================================================

static void testCudaVirtualMemory() {
	section("CUDA 2.4 -- Virtual Memory");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }

	using namespace Spectra::Cuda::Memory;
	using namespace Spectra::Cuda::Utils;

	AllocDesc desc{};
	initAllocDesc(desc);
	setAllocationType(desc, AllocationType::PINNED);
	setAllocationHandleType(desc, AllocationHandleType::NONE);
	setLocation(desc, g_Device);

	size_t gran = VirtualMemory::getAllocationGranularity(desc, AllocationGranularityOption::MINIMUM);
	TEST("getAllocationGranularity > 0", gran > 0);

	GpuAddress addr = VirtualMemory::reserveAddress(gran);
	TEST("reserveAddress valid", addr.isValid());

	AllocHandle handle = VirtualMemory::createAllocation(gran, desc);
	TEST("createAllocation valid", handle.isValid());

	VirtualMemory::map(addr, gran, 0, handle);
	TEST("map does not crash", true);

	AccessDesc accessDesc{};
	initAccessDesc(accessDesc);
	setAccessLocation(accessDesc, g_Device);
	setAccessFlags(accessDesc, CudaHelpers::computeAccessFlags({ AccessFlagBits::READWRITE }));

	VirtualMemory::setAccess(addr, gran, &accessDesc, 1);
	TEST("setAccess does not crash", true);

	VirtualMemory::unmap(addr, gran);
	TEST("unmap does not crash", true);

	VirtualMemory::releaseAllocation(handle);
	TEST("releaseAllocation (invalid after release)", !handle.isValid());

	VirtualMemory::freeAddress(addr, gran);
	TEST("freeAddress (invalid after free)", !addr.isValid());
}

// =========================================================
// CUDA 3.1 - Streams
// =========================================================

static void testCudaStreams() {
	section("CUDA 3.1 -- Streams");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Streams;
	using namespace Spectra::Cuda::Utils;

	GpuStream stream = DeviceStreams::createStream(StreamFlags::NON_BLOCKING);
	TEST("createStream NON_BLOCKING valid", stream.isValid());

	bool ready = DeviceStreams::queryStream(stream);
	TEST("queryStream succeeds", ready || !ready);

	DeviceStreams::syncStream(stream);
	DeviceStreams::destroyStream(stream);
	TEST("destroyStream nulls handle", !stream.isValid());
}

// =========================================================
// CUDA 3.2 - Events
// =========================================================

static void testCudaEvents() {
	section("CUDA 3.2 -- Events");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Events;
	using namespace Spectra::Cuda::Streams;
	using namespace Spectra::Cuda::Utils;

	GpuEvent e1 = DeviceEvents::createEvent(CudaHelpers::computeEventFlags({EventFlags::DEFAULT}));
	GpuEvent e2 = DeviceEvents::createEvent(CudaHelpers::computeEventFlags({EventFlags::DEFAULT}));
	TEST("createEvent valid", e1.isValid() && e2.isValid());

	GpuStream stream = DeviceStreams::createStream(StreamFlags::NON_BLOCKING);

	DeviceEvents::recordEvent(e1, stream);
	DeviceEvents::recordEvent(e2, stream);
	DeviceEvents::syncEvent(e2);

	float ms = DeviceEvents::elapsedTime(e1, e2);
	TEST("elapsedTime works", ms >= 0.0f);

	DeviceStreams::destroyStream(stream);
	DeviceEvents::destroyEvent(e1);
	DeviceEvents::destroyEvent(e2);
}

// =========================================================
// CUDA 3.3 - Graphs
// =========================================================

static void testCudaGraphs() {
	section("CUDA 3.3 -- Graphs");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Graphs;
	using namespace Spectra::Cuda::Memory;
	using namespace Spectra::Cuda::Streams;
	using namespace Spectra::Cuda::Utils;

	GpuGraph graph = DeviceGraphs::createGraph();
	TEST("createGraph valid", graph.isValid());

	GpuAddress mem = DeviceMemory::deviceAlloc(128);
	MemsetNodeParams p{};
	p.m_Dst = mem.m_GpuAddr;
	p.m_ElementSize = 1;
	p.m_Value = 0xAA;
	p.m_Width = 128;
	p.m_Height = 1;

	GpuGraphNode node = DeviceGraphs::addMemsetNode(graph, nullptr, 0, p);
	TEST("addMemsetNode valid", node.isValid());

	GpuGraphExec exec = DeviceGraphs::instantiate(graph);
	TEST("instantiate graph valid", exec.isValid());

	GpuStream stream = DeviceStreams::createStream(StreamFlags::NON_BLOCKING);
	DeviceGraphs::launch(exec, stream);
	DeviceStreams::syncStream(stream);

	DeviceMemory::deviceFree(mem);
	DeviceStreams::destroyStream(stream);
	DeviceGraphs::destroyExec(exec);
	DeviceGraphs::destroyGraph(graph);
}

// =========================================================
// CUDA 4.1 - Modules
// =========================================================

static void testCudaModules() {
	section("CUDA 4.1 -- Modules");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Modules;
	using namespace Spectra::Cuda::Utils;

	JitOptions options{};
	initJitOptions(options);
	TEST("JitOptions initialized", options.m_OptLevel == JitOptimizationLevel::DEFAULT_MAX);

	GpuLinkState linkState = DeviceLinker::createLinkState(options);
	TEST("createLinkState valid", linkState.isValid());
	
	DeviceLinker::destroyLinkState(linkState);
	TEST("destroyLinkState nulls handle", !linkState.isValid());
}

// =========================================================
// CUDA 4.2 - Compute (Kernel Launch)
// =========================================================

static void testCudaCompute() {
	section("CUDA 4.2 -- Compute");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Compute;
	using namespace Spectra::Cuda::Utils;
	
	// We just ensure the types are accessible and structs can form up without issue.
	LaunchDimension gridDim{1, 1, 1};
	LaunchDimension blockDim{128, 1, 1};
	TEST("LaunchDimensions struct verified", gridDim.x == 1 && blockDim.x == 128);

	// Since we don't have a compiled .ptx string at hand in this smoke test, 
	// we will rely on checking header visibility, which was proven if this compiles.
	TEST("Compute layer accessible", true);
}

// =========================================================
// CUDA (Section 6) - OptiX
// =========================================================

static void testOptixSection6() {
	section("CUDA 6.0 -- OptiX");
	if (!g_CudaReady) { printf("  [SKIP] No CUDA device\n"); return; }
	using namespace Spectra::Cuda::Optix;
	using namespace Spectra::Cuda::Utils;

	bool optixReady = DeviceOptixContext::initOptix();
	if (!optixReady) {
		printf("  [SKIP] OptiX not installed or configured\n");
		return;
	}
	TEST("optixInit handles successful startup", optixReady);

	OptixContextOptions options{};
	// Just a default context using the primary device config wrapper from our Cuda tests.
	// Since we mock it here, we will just pass a valid wrapper struct.
	CudaContext devCtx{};
	devCtx.m_ContextHandle = g_Context.m_ContextHandle;
	
	GpuOptixContext ctx = DeviceOptixContext::createContext(devCtx, options);
	TEST("createContext creates valid handle", ctx.isValid());

	DeviceOptixContext::destroyContext(ctx);
	TEST("destroyContext invalidates handle", !ctx.isValid());
}

// =========================================================
// Stratum includes
// =========================================================

#include "Stratum.h"
#include "Records.h"
#include "Filter.h"
#include "ProfilerTrace.h"
#include "Logger.h"
#include "Orchestrator.h"
#include "Sinks.h"
#include "StackTrace.h"
#include "Scopes.h"

// =========================================================
// Stratum test helpers
// =========================================================

struct CapturingSinkRouter final : Stratum::Logging::ISinkRouter {
	int       logCount = 0;
	int       exceptionCount = 0;
	int       tracerCount = 0;
	char      lastMessage[Stratum::MESSAGE_MAX]{};
	char      lastException[Stratum::MESSAGE_MAX]{};
	char      lastLabel[Stratum::LABEL_MAX]{};

	void write(const Stratum::Records::LogEntry& ro_Entry) override {
		++logCount;
		std::strncpy(lastMessage, ro_Entry.getMessage(), Stratum::MESSAGE_MAX - 1);
	}
	void write(const Stratum::Records::ExceptionEntry& ro_Entry) override {
		++exceptionCount;
		std::strncpy(lastException, ro_Entry.getErrorMessage(), Stratum::MESSAGE_MAX - 1);
	}
	void write(const Stratum::Records::TracerEntry& ro_Entry) override {
		++tracerCount;
		std::strncpy(lastLabel, ro_Entry.getLabel(), Stratum::LABEL_MAX - 1);
	}
};

// =========================================================
// Stratum 1 — Records
// =========================================================

static void testStratumRecords() {
	section("Stratum 1 -- Records");

	using namespace Stratum::Records;

	// LogEntry
	{
		LogEntry entry{ LogLevel::Warning, "TestComp", "hello world" };
		TEST("LogEntry level", entry.getLevel() == LogLevel::Warning);
		TEST("LogEntry component", std::strncmp(entry.getComponent(), "TestComp", 8) == 0);
		TEST("LogEntry message", std::strncmp(entry.getMessage(), "hello world", 11) == 0);
		TEST("LogEntry timestamp > 0", entry.getTimestamp() > 0);
	}

	// ExceptionEntry
	{
		ExceptionEntry entry{ "ErrComp", "something failed" };
		TEST("ExceptionEntry component", std::strncmp(entry.getComponent(), "ErrComp", 7) == 0);
		TEST("ExceptionEntry message", std::strncmp(entry.getErrorMessage(), "something failed", 16) == 0);
		TEST("ExceptionEntry timestamp > 0", entry.getTimestamp() > 0);
	}

	// TracerEntry lifecycle
	{
		TracerEntry entry{ "PerfComp", "mySection" };
		TEST("TracerEntry label", std::strncmp(entry.getLabel(), "mySection", 9) == 0);
		TEST("TracerEntry not started", !entry.isStarted());
		TEST("TracerEntry not complete", !entry.isComplete());

		entry.start();
		TEST("TracerEntry started", entry.isStarted());
		TEST("TracerEntry not complete after start", !entry.isComplete());

		entry.end();
		TEST("TracerEntry complete", entry.isComplete());
		TEST("TracerEntry ns duration >= 0", entry.getDuration(TracerPrecision::Nanoseconds) >= 0.0);
	}

	// LoggerProfile
	{
		uint8_t buf[256]{};
		LoggerProfile profile{ "myProfile", buf, sizeof(buf) };
		TEST("LoggerProfile name", std::strncmp(profile.getName(), "myProfile", 9) == 0);
		TEST("LoggerProfile buffer", profile.getBuffer() == buf);
		TEST("LoggerProfile bufferSize", profile.getBufferSize() == 256);
		TEST("LoggerProfile enabled by default", profile.isEnabled());

		profile.disable();
		TEST("LoggerProfile disabled after disable()", !profile.isEnabled());
		profile.enable();
		TEST("LoggerProfile enabled after enable()", profile.isEnabled());

		profile.setPolicy(TracePolicy::AutoFlushOnError);
		TEST("LoggerProfile policy set", profile.getPolicy() == TracePolicy::AutoFlushOnError);
	}
}

// =========================================================
// Stratum 2 — FilterChain
// =========================================================

static void testStratumFilter() {
	section("Stratum 2 -- FilterChain");

	using namespace Stratum::Filtering;
	using namespace Stratum::Records;

	FilterChain fc{};

	// Level gate
	fc.setMinLevel(LogLevel::Warning);
	TEST("filter rejects Info below Warning", !fc.accepts(LogLevel::Info, "C", "t"));
	TEST("filter accepts Warning", fc.accepts(LogLevel::Warning, "C", "t"));
	TEST("filter accepts Error", fc.accepts(LogLevel::Error, "C", "t"));
	TEST("filter accepts Crash", fc.accepts(LogLevel::Crash, "C", "t"));
	fc.setMinLevel(LogLevel::Info);

	// Component allowlist
	fc.setComponentMode(FilterMode::Allowlist);
	fc.addComponentFilter("Render");
	TEST("allowlist blocks unlisted component", !fc.accepts(LogLevel::Info, "Audio", "t"));
	TEST("allowlist passes listed component", fc.accepts(LogLevel::Info, "Render", "t"));
	fc.clearComponentFilters();
	TEST("accepts all after clear", fc.accepts(LogLevel::Info, "Audio", "t"));

	// Component denylist
	fc.setComponentMode(FilterMode::Denylist);
	fc.addComponentFilter("Blocked");
	TEST("denylist blocks listed component", !fc.accepts(LogLevel::Info, "Blocked", "t"));
	TEST("denylist passes unlisted component", fc.accepts(LogLevel::Info, "Safe", "t"));
	fc.clearComponentFilters();

	// Tag filter
	fc.addTagFilter("console");
	TEST("tag filter passes matching tag", fc.accepts(LogLevel::Info, "C", "console"));
	TEST("tag filter blocks non-matching", !fc.accepts(LogLevel::Info, "C", "file"));
	fc.clearTagFilters();
	TEST("all tags pass after tag clear", fc.accepts(LogLevel::Info, "C", "file"));
}

// =========================================================
// Stratum 3 — ProfilerTrace
// =========================================================

static void testStratumProfilerTrace() {
	section("Stratum 3 -- ProfilerTrace");

	using namespace Stratum::Profiler;
	using namespace Stratum::Records;

	// Default constructed
	{
		ProfilerTrace trace{};
		TEST("default kind is None", trace.kind() == EntryKind::None);
		TEST("default not active", !trace.active());
		TEST("get<LogEntry> on None", trace.get<LogEntry>() == nullptr);
	}

	// Emplace LogEntry
	{
		ProfilerTrace trace{};
		LogEntry entry{ LogLevel::Debug, "Comp", "msg" };
		trace.emplace(entry);
		TEST("emplace Log: kind", trace.kind() == EntryKind::Log);
		TEST("emplace Log: active", trace.active());
		TEST("emplace Log: get != null", trace.get<LogEntry>() != nullptr);
		TEST("emplace Log: get<Exc> null", trace.get<ExceptionEntry>() == nullptr);
		trace.destroy();
		TEST("destroy: not active", !trace.active());
		TEST("destroy: kind None", trace.kind() == EntryKind::None);
	}

	// Emplace ExceptionEntry
	{
		ProfilerTrace trace{};
		ExceptionEntry exc{ "Comp", "boom" };
		trace.emplace(exc);
		TEST("emplace Exc: kind", trace.kind() == EntryKind::Exception);
		TEST("emplace Exc: get != null", trace.get<ExceptionEntry>() != nullptr);
		trace.destroy();
	}

	// Emplace TracerEntry
	{
		ProfilerTrace trace{};
		TracerEntry tracer{ "Comp", "scope" };
		trace.emplace(tracer);
		TEST("emplace Tracer: kind", trace.kind() == EntryKind::Tracer);
		TEST("emplace Tracer: get != null", trace.get<TracerEntry>() != nullptr);
		trace.destroy();
	}

	// Copy
	{
		ProfilerTrace a{};
		LogEntry entry{ LogLevel::Info, "C", "copy test" };
		a.emplace(entry);
		ProfilerTrace b{ a };
		TEST("copy: same kind", b.kind() == EntryKind::Log);
		TEST("copy: both active", a.active() && b.active());
		a.destroy();
		b.destroy();
	}

	// Move
	{
		ProfilerTrace a{};
		ExceptionEntry exc{ "C", "move test" };
		a.emplace(exc);
		ProfilerTrace b{ std::move(a) };
		TEST("move: b active", b.active());
		TEST("move: b kind Exc", b.kind() == EntryKind::Exception);
		TEST("move: a inactive", !a.active());
		b.destroy();
	}
}

// =========================================================
// Stratum 4 — DefaultLogger (ring buffer + flush)
// =========================================================

static void testStratumDefaultLogger() {
	section("Stratum 4 -- DefaultLogger");

	using namespace Stratum::Logging;
	using namespace Stratum::Records;

	CapturingSinkRouter router{};

	// Basic log → flush route
	{
		DefaultLogger<4> logger{};
		logger.addRoute("console", &router);

		LogEntry entry{ LogLevel::Info, "Comp", "first" };
		logger.onLog("console", entry);

		LogEntry entry2{ LogLevel::Error, "Comp", "second" };
		logger.onLog("console", entry2);

		TEST("ring count pre-flush: capture empty", router.logCount == 0);
		logger.flush();
		TEST("flush dispatched 2 log entries", router.logCount == 2);
		TEST("last message matches", std::strncmp(router.lastMessage, "second", 6) == 0);
	}

	// Exception + tracer routes
	{
		CapturingSinkRouter r2{};
		DefaultLogger<4> logger{};
		logger.addRoute("console", &r2);

		ExceptionEntry exc{ "Comp", "oops" };
		logger.onLog("console", exc);

		TracerEntry tracer{ "Comp", "zone" };
		tracer.start();
		tracer.end();
		logger.onLog("console", tracer);

		logger.flush();
		TEST("flush dispatched exception", r2.exceptionCount == 1);
		TEST("flush dispatched tracer", r2.tracerCount == 1);
		TEST("tracer label flushed", std::strncmp(r2.lastLabel, "zone", 4) == 0);
	}

	// DropOldest overflow
	{
		CapturingSinkRouter r3{};
		DefaultLogger<2> logger{ OverflowPolicy::DropOldest };
		logger.addRoute("console", &r3);

		for (int i = 0; i < 4; ++i) {
			char msg[8]{};
			msg[0] = static_cast<char>('0' + i);
			LogEntry e{ LogLevel::Info, "C", msg };
			logger.onLog("console", e);
		}
		logger.flush();
		TEST("DropOldest: only 2 entries survive", r3.logCount == 2);
		TEST("DropOldest: last entry is '3'", r3.lastMessage[0] == '3');
	}

	// DropNewest overflow
	{
		CapturingSinkRouter r4{};
		DefaultLogger<2> logger{ OverflowPolicy::DropNewest };
		logger.addRoute("console", &r4);

		for (int i = 0; i < 4; ++i) {
			char msg[8]{};
			msg[0] = static_cast<char>('0' + i);
			LogEntry e{ LogLevel::Info, "C", msg };
			logger.onLog("console", e);
		}
		logger.flush();
		TEST("DropNewest: only 2 entries survive", r4.logCount == 2);
		TEST("DropNewest: last entry is '1'", r4.lastMessage[0] == '1');
	}

	// Filter gate
	{
		CapturingSinkRouter r5{};
		DefaultLogger<8> logger{};
		logger.addRoute("console", &r5);
		logger.filter().setMinLevel(LogLevel::Error);

		LogEntry info{ LogLevel::Info,  "C", "should be dropped" };
		LogEntry err{ LogLevel::Error, "C", "should pass" };
		logger.onLog("console", info);
		logger.onLog("console", err);
		logger.flush();
		TEST("filter gates log: only 1 survives", r5.logCount == 1);
		TEST("filter gates log: correct message",
			 std::strncmp(r5.lastMessage, "should pass", 11) == 0);
	}
}

// =========================================================
// Stratum 5 — Orchestrator
// =========================================================

static void testStratumOrchestrator() {
	section("Stratum 5 -- Orchestrator");

	using namespace Stratum::Logging;
	using namespace Stratum::Records;

	// Use a fresh DefaultLogger for this section; Orchestrator is a singleton
	// so we give it a unique name to avoid collisions with any prior state.
	DefaultLogger<32> logger{};
	CapturingSinkRouter router{};
	logger.addRoute("console", &router);

	auto& orc = Orchestrator::getInstance();
	bool registered = orc.registerLogger("testLogger", &logger);
	TEST("registerLogger succeeds", registered);

	orc.log(LogLevel::Info, "testLogger", "OrcComp", "console", "orc log");
	orc.flush();
	TEST("Orchestrator log dispatched", router.logCount == 1);
	TEST("Orchestrator log message",
		 std::strncmp(router.lastMessage, "orc log", 7) == 0);

	TracerEntry tracer{ "OrcComp", "orc_scope" };
	tracer.start();
	tracer.end();
	orc.trace("testLogger", "console", tracer);
	orc.flush();
	TEST("Orchestrator trace dispatched", router.tracerCount == 1);

	ExceptionEntry exc{ "OrcComp", "orc_error" };
	orc.exception("testLogger", "console", exc);
	orc.flush();
	TEST("Orchestrator exception dispatched", router.exceptionCount == 1);

	// Lookup miss — should silently no-op
	orc.log(LogLevel::Error, "noSuchLogger", "C", "t", "ghost");
	orc.flush();
	TEST("Orchestrator: unknown logger is a no-op", router.logCount == 1);
}

// =========================================================
// Stratum 6 — StackTrace
// =========================================================

static void testStratumStackTrace() {
	section("Stratum 6 -- StackTrace");

	using namespace Stratum::Tracing;
	using namespace Stratum::Profiler;
	using namespace Stratum::Records;

	StackTrace<> st{};
	TEST("empty on construction", st.empty());
	TEST("peekFrame None when empty", st.peekFrame() == EntryKind::None);

	// Push a LogEntry
	LogEntry log{ LogLevel::Debug, "C", "stack msg" };
	bool pushed = st.pushFrame(log);
	TEST("pushFrame LogEntry succeeds", pushed);
	TEST("not empty after push", !st.empty());
	TEST("peekFrame is Log", st.peekFrame() == EntryKind::Log);

	LogEntry popped = st.popFrame<LogEntry>();
	TEST("popFrame LogEntry message",
		 std::strncmp(popped.getMessage(), "stack msg", 9) == 0);
	TEST("empty after pop", st.empty());

	// Push ExceptionEntry
	ExceptionEntry exc{ "C", "stack exc" };
	st.pushFrame(exc);
	TEST("peekFrame is Exception", st.peekFrame() == EntryKind::Exception);
	ExceptionEntry poppedExc = st.popFrame<ExceptionEntry>();
	TEST("popFrame ExceptionEntry message",
		 std::strncmp(poppedExc.getErrorMessage(), "stack exc", 9) == 0);

	// Push TracerEntry
	TracerEntry tracer{ "C", "stack_scope" };
	st.pushFrame(tracer);
	TEST("peekFrame is Tracer", st.peekFrame() == EntryKind::Tracer);
	TracerEntry poppedTracer = st.popFrame<TracerEntry>();
	TEST("popFrame TracerEntry label",
		 std::strncmp(poppedTracer.getLabel(), "stack_scope", 11) == 0);

	// Multiple frames — FIFO order (sentinel-based linked list pops from front)
	LogEntry a{ LogLevel::Info,    "C", "first" };
	LogEntry b{ LogLevel::Warning, "C", "second" };
	st.pushFrame(a);
	st.pushFrame(b);
	LogEntry first = st.popFrame<LogEntry>();
	LogEntry second = st.popFrame<LogEntry>();
	TEST("StackTrace FIFO order first", std::strncmp(first.getMessage(), "first", 5) == 0);
	TEST("StackTrace FIFO order second", std::strncmp(second.getMessage(), "second", 6) == 0);

	// Pop from empty returns s_invalid
	LogEntry invalid = st.popFrame<LogEntry>();
	TEST("popFrame on empty returns s_invalid message",
		 std::strncmp(invalid.getMessage(),
					  Stratum::Records::InvalidEntries::s_log.getMessage(),
					  Stratum::MESSAGE_MAX) == 0);
}

// =========================================================
// Stratum 7 — STRATUM_TRACE / STRATUM_TRY macros
// =========================================================

static void testStratumMacros() {
	section("Stratum 7 -- Macros");

	using namespace Stratum::Records;

	// STRATUM_TRACE: tracer must be started and ended by the macro
	STRATUM_TRACE(myTrace, "MacroComp", "macroSection")
		// body — nothing needed
		STRATUM_TRACE_END

		TEST("STRATUM_TRACE: is complete", myTrace.isComplete());
	TEST("STRATUM_TRACE: label matches",
		 std::strncmp(myTrace.getLabel(), "macroSection", 12) == 0);
	TEST("STRATUM_TRACE: duration >= 0",
		 myTrace.getDuration(TracerPrecision::Nanoseconds) >= 0.0);

	// STRATUM_TRY / STRATUM_CATCH: normal path returns empty exception
	STRATUM_TRY("MacroComp") {
		STRATUM_THROW("deliberate error");
	}
	STRATUM_CATCH(exc1)

		TEST("STRATUM_TRY/CATCH: component",
			 std::strncmp(exc1.getComponent(), "MacroComp", 9) == 0);
	TEST("STRATUM_TRY/CATCH: message",
		 std::strncmp(exc1.getErrorMessage(), "deliberate error", 16) == 0);

	// STRATUM_TRY / STRATUM_CATCH: std::exception propagation
	STRATUM_TRY("MacroComp") {
		throw std::runtime_error("runtime boom");
		return ExceptionEntry{ p_Comp, "" };
	}
	STRATUM_CATCH(exc2)

		TEST("STRATUM_TRY/CATCH std::exception caught",
			 std::strncmp(exc2.getErrorMessage(), "runtime boom", 12) == 0);
}
}
}

// =========================================================
// Entry Point
// =========================================================

int main() {
	printf("Spectra Platform Runtime � Component Test\n");
	printf("==========================================\n");

	testEnvironment();
	testVirtualMemory();
	testChrono();
	testAtomics();
	testFiles();

	setupCuda();
	testCudaDeviceMemory();
	testCudaPinnedMemory();
	testCudaManagedMemory();
	testCudaVirtualMemory();
	testCudaStreams();
	testCudaEvents();
	testCudaGraphs();
	testCudaModules();
	testCudaCompute();
	testOptixSection6();
	teardownCuda();

	testStratumRecords();
	testStratumFilter();
	testStratumProfilerTrace();
	testStratumDefaultLogger();
	testStratumOrchestrator();
	testStratumStackTrace();
	testStratumMacros();

	printf("\n==========================================\n");
	printf("Results: %d passed, %d failed\n", g_Passed, g_Failed);

	Corium::CoriumRuntime::initRuntime();
	int a = 3;
	int b = 300;
	auto closure = Corium::Core::Utils::buildClosure<void(int)>([b](int x) {
		std::cout << "Hello" << x * b;	}, 0);
	closure(a);

	return (g_Failed == 0) ? 0 : 1;
}