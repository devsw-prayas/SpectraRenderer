#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/MemorySuiteBoot.h"
#include <CoriumDiagnostics.h>
#include <cstring>

class TestFixture1 final
	: public Hades::Runtime::IFixture<TestFixture1, Hades::Runtime::NullDeviceAdapter> {
private:
	struct LiveAlloc {
		uint8_t* m_Ptr;
		size_t m_Size;
		uint8_t m_Tag;
	};

	static constexpr size_t kMaxLive = 4096;
	static constexpr int kSteps = 200000;
	static inline LiveAlloc s_Live[kMaxLive];

public:
	explicit TestFixture1(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		CORIUM_ASSERT(MemorySuite::boot());
		Spectra::Memory::Heap::HeapRegistry::init();
	}

	void executeImpl() noexcept {
		using namespace Spectra::Memory;
		Heap::SyncHeap& heap = Heap::HeapRegistry::forNode(0);

		CORIUM_ASSERT(heap.checkHeap());
		CORIUM_ASSERT(heap.allocate(0) == nullptr && "allocate(0) returns null");
		CORIUM_ASSERT(heap.allocate(Heap::kMaxAlloc + 1) == nullptr && "Oversized allocate returns null");

		size_t liveCount = 0;
		MemorySuite::XorShift64 rng{ 0x5EC7A11E5EEDull };
		for (int step = 0; step < kSteps; ++step) {
			const bool doAlloc = liveCount == 0 || (liveCount < kMaxLive && rng.next() % 100 < 55);
			if (doAlloc) {
				const uint64_t bucket = rng.next() % 100;
				const size_t size = bucket < 70 ? 1 + rng.next() % 256
					: bucket < 95 ? 1 + rng.next() % 16384
					: bucket < 99 ? 1 + rng.next() % (1u << 20)
					: 1 + rng.next() % (8u << 20);
				const size_t alignment = rng.next() % 5 == 0 ? size_t{ 32 } << (rng.next() % 12) : Heap::kAlign;

				auto* ptr = static_cast<uint8_t*>(alignment > Heap::kAlign ? heap.allocateAligned(size, alignment) : heap.allocate(size));
				CORIUM_ASSERT(ptr != nullptr);
				CORIUM_ASSERT(reinterpret_cast<uintptr_t>(ptr) % alignment == 0 && "Payload misaligned");
				CORIUM_ASSERT(heap.owns(ptr));

				const auto tag = static_cast<uint8_t>(rng.next());
				std::memset(ptr, tag, size);
				s_Live[liveCount++] = { ptr, size, tag };
			} else {
				const size_t index = rng.next() % liveCount;
				const LiveAlloc entry = s_Live[index];
				for (size_t i = 0; i < entry.m_Size; i += 61)
					CORIUM_ASSERT(entry.m_Ptr[i] == entry.m_Tag && "Block contents clobbered");
				CORIUM_ASSERT(entry.m_Ptr[entry.m_Size - 1] == entry.m_Tag && "Block tail clobbered");
				heap.deallocate(entry.m_Ptr);
				s_Live[index] = s_Live[--liveCount];
			}
			if (step % 5000 == 0) CORIUM_ASSERT(heap.checkHeap());
		}

		for (size_t i = 0; i < liveCount; ++i) heap.deallocate(s_Live[i].m_Ptr);
		CORIUM_ASSERT(heap.checkHeap());

		void* big = heap.allocate(64ull << 20);
		void* huge = heap.allocateAligned(3ull << 30, 2ull << 20);
		CORIUM_ASSERT(big && huge && "64 MiB and 2 MiB-aligned 3 GiB allocations");
		CORIUM_ASSERT(reinterpret_cast<uintptr_t>(huge) % (2ull << 20) == 0);
		CORIUM_ASSERT(heap.checkHeap());
		heap.deallocate(big);
		heap.deallocate(huge);
		CORIUM_ASSERT(heap.checkHeap());
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
