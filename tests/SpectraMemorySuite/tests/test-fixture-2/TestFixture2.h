#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/MemorySuiteBoot.h"
#include <CoriumDiagnostics.h>
#include <atomic>
#include <cstring>
#include <thread>

class TestFixture2 final
	: public Hades::Runtime::IFixture<TestFixture2, Hades::Runtime::NullDeviceAdapter> {
private:
	static constexpr size_t kSlotCount = 1024;
	static constexpr int kThreadCount = 8;
	static constexpr int kOpsPerThread = 100000;
	static inline std::atomic<uint8_t*> s_Slots[kSlotCount];
	static inline std::atomic<int> s_Failures{ 0 };

	static bool verifyAndFree(uint8_t* p_Ptr) noexcept {
		size_t size;
		std::memcpy(&size, p_Ptr, sizeof(size));
		const auto tag = static_cast<uint8_t>(size * 131);
		bool ok = true;
		for (size_t i = sizeof(size); i < size; i += 37)
			if (p_Ptr[i] != tag) { ok = false; break; }
		Spectra::Memory::Heap::HeapRegistry::deallocate(p_Ptr);
		return ok;
	}

	static void worker(uint64_t v_Seed) noexcept {
		MemorySuite::XorShift64 rng{ v_Seed };
		for (int op = 0; op < kOpsPerThread; ++op) {
			const size_t size = sizeof(size_t) + rng.next() % 2048;
			auto* ptr = static_cast<uint8_t*>(Spectra::Memory::Heap::HeapRegistry::allocate(size));
			if (!ptr) { s_Failures.fetch_add(1); return; }
			std::memcpy(ptr, &size, sizeof(size));
			std::memset(ptr + sizeof(size), static_cast<uint8_t>(size * 131), size - sizeof(size));

			uint8_t* previous = s_Slots[rng.next() % kSlotCount].exchange(ptr, std::memory_order_acq_rel);
			if (previous && !verifyAndFree(previous)) s_Failures.fetch_add(1);
		}
	}

public:
	explicit TestFixture2(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		CORIUM_ASSERT(MemorySuite::boot());
	}

	void executeImpl() noexcept {
		s_Failures.store(0);
		std::thread workers[kThreadCount];
		for (int i = 0; i < kThreadCount; ++i) workers[i] = std::thread(worker, 0x9E3779B97F4A7C15ull * (i + 1));
		for (std::thread& t : workers) t.join();

		for (std::atomic<uint8_t*>& slot : s_Slots)
			if (uint8_t* ptr = slot.exchange(nullptr); ptr && !verifyAndFree(ptr)) s_Failures.fetch_add(1);

		CORIUM_ASSERT(s_Failures.load() == 0 && "Every block must come back intact after cross-thread frees");
		CORIUM_ASSERT(Spectra::Memory::Heap::HeapRegistry::forNode(0).checkHeap());
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
