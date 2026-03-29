#include <cstdint>

#include "CudaBootstrap.h"
#include "CudaContextManager.h"

#include <cstdio>

using namespace Spectra::Cuda;

namespace {
	void printUUID(const Utils::DeviceUUID& uuid) {
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&uuid);
		for (int i = 0; i < 16; i++) {
			printf("%02x", bytes[i]);
			if (i == 3 || i == 5 || i == 7 || i == 9) printf("-");
		}
		printf("\n");
	}

	void testDeviceAttributes(Utils::DeviceHandle handle) {
		int major = 0, minor = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&major, Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MAJOR, handle);
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&minor, Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MINOR, handle);
		printf("  Compute Capability : SM %d.%d\n", major, minor);

		int maxThreads = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&maxThreads, Utils::CudaDeviceAttribute::MAX_THREADS_PER_BLOCK, handle);
		printf("  Max Threads/Block  : %d\n", maxThreads);

		int warpSize = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&warpSize, Utils::CudaDeviceAttribute::WARP_SIZE, handle);
		printf("  Warp Size          : %d\n", warpSize);

		int l2 = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&l2, Utils::CudaDeviceAttribute::L2_CACHE_SIZE, handle);
		printf("  L2 Cache           : %d KB\n", l2 / 1024);

		int gdrSupported = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&gdrSupported, Utils::CudaDeviceAttribute::GPU_DIRECT_RDMA_SUPPORTED, handle);
		printf("  GPUDirect RDMA     : %s\n", gdrSupported ? "yes" : "no");

		int vmSupported = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&vmSupported, Utils::CudaDeviceAttribute::VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED, handle);
		printf("  Virtual Memory Mgmt: %s\n", vmSupported ? "yes" : "no");

		int concurrentKernels = 0;
		Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(
			&concurrentKernels, Utils::CudaDeviceAttribute::CONCURRENT_KERNELS, handle);
		printf("  Concurrent Kernels : %s\n", concurrentKernels ? "yes" : "no");

		size_t totalMem = Bootstrap::CudaDeviceManager::getCudaDeviceTotalMemory(handle);
		printf("  Total VRAM         : %zu MB\n", totalMem / (1024 * 1024));
	}

	void testContextLifecycle(Utils::DeviceHandle handle) {
		printf("\n[Context Lifecycle]\n");

		// Create context
		Utils::CudaContext ctx = Context::ContextManager::createCudaContext(
			handle,
			Utils::ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC,
			Utils::ContextCreationFlags::NONE);
		printf("  createCudaContext  : OK (handle = %p)\n", ctx.m_Handle);

		// Get current - should match what we just created
		Utils::CudaContext current = Context::ContextManager::getCurrentCudaContext();
		printf("  getCurrentContext  : %p\n", current.m_Handle);
		printf("  Matches created    : %s\n", current.m_Handle == ctx.m_Handle ? "YES" : "NO");

		// Push / pop test
		// Create a second context to push
		Utils::CudaContext ctx2 = Context::ContextManager::createCudaContext(
			handle,
			Utils::ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC,
			Utils::ContextCreationFlags::NONE);
		printf("  createCudaContext2 : OK (handle = %p)\n", ctx2.m_Handle);

		// Set first context current, then push second
		Context::ContextManager::setCurrentCudaContext(ctx);
		printf("  setCurrentContext  : OK\n");

		Context::ContextManager::pushCudaContext(ctx2);
		printf("  pushCudaContext    : OK\n");

		Utils::CudaContext afterPush = Context::ContextManager::getCurrentCudaContext();
		printf("  Current after push : %p\n", afterPush.m_Handle);
		printf("  Is ctx2            : %s\n", afterPush.m_Handle == ctx2.m_Handle ? "YES" : "NO");

		Utils::CudaContext popped = Context::ContextManager::popCudaContext();
		printf("  popCudaContext     : OK (popped = %p)\n", popped.m_Handle);
		printf("  Popped is ctx2     : %s\n", popped.m_Handle == ctx2.m_Handle ? "YES" : "NO");

		Utils::CudaContext afterPop = Context::ContextManager::getCurrentCudaContext();
		printf("  Current after pop  : %p\n", afterPop.m_Handle);
		printf("  Restored to ctx    : %s\n", afterPop.m_Handle == ctx.m_Handle ? "YES" : "NO");

		// Synchronize
		Context::ContextManager::cudaContextSynchronize();
		printf("  cudaContextSync    : OK\n");

		// Teardown - destroy in reverse order
		Context::ContextManager::destroyCudaContext(ctx2);
		printf("  destroyContext2    : OK (handle nulled = %s)\n", ctx2.m_Handle == nullptr ? "YES" : "NO");

		Context::ContextManager::destroyCudaContext(ctx);
		printf("  destroyContext     : OK (handle nulled = %s)\n", ctx.m_Handle == nullptr ? "YES" : "NO");
	}
}

int main() {
	printf("=== Spectra CUDA Backend - Section 1 Test ===\n\n");

	// Driver init
	printf("[Driver Init]\n");
	bool initOk = Bootstrap::CudaDriver::initCuda();
	if (!initOk) {
		printf("  No CUDA devices found. Exiting.\n");
		return 0;
	}
	printf("  cuInit             : OK\n");

	uint32_t version = Bootstrap::CudaDriver::getCudaDriverVersion();
	printf("  Driver Version     : %u.%u\n", version / 1000, (version % 1000) / 10);

	// Device enumeration
	int deviceCount = Bootstrap::CudaDeviceManager::getCudaDeviceCount();
	printf("  Device Count       : %d\n\n", deviceCount);

	for (int i = 0; i < deviceCount; i++) {
		Utils::DeviceHandle handle = Bootstrap::CudaDeviceManager::getCudaDevice(i);

		char name[256] = {};
		Bootstrap::CudaDeviceManager::getCudaDeviceName(name, sizeof(name), handle);
		printf("[Device %d] %s\n", i, name);

		Utils::DeviceUUID uuid = Bootstrap::CudaDeviceManager::getCudaDeviceUUID(handle);
		printf("  UUID               : ");
		printUUID(uuid);

		testDeviceAttributes(handle);
		testContextLifecycle(handle);

		printf("\n");
	}

	printf("=== Section 1 Complete ===\n");
	return 0;
}