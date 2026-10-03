#pragma once
#include "SpectraMemory.h"
#include "SpecMemAddrSpace.h"
#include "SpecMemDiagnostics.h"
#include "MemoryRegion.h"
#include "PlatformAtomics.h"
#include <CoriumSync.h>

namespace Spectra::Memory::Heap {
	static constexpr size_t kSlLog2 = SPECTRA_TLSF_SL_LOG2;
	static constexpr size_t kAlignLog2 = SPECTRA_TLSF_ALIGN_LOG2;

	static constexpr size_t kAlign = static_cast<size_t>(1) << kAlignLog2;
	static constexpr size_t kSlCount = static_cast<size_t>(1) << kSlLog2;
	static constexpr size_t kFlShift = kSlLog2 + kAlignLog2;
	static constexpr size_t kSmallLimit = static_cast<size_t>(1) << kFlShift;

	// +1 turns the top bit index into a class count, +1 more for the linear FL 0 below kFlShift.
	static constexpr size_t kFlCount = static_cast<size_t>(std::bit_width(Internal::TlsfHeapSize)) - 1 - kFlShift + 2;

	// Used-block overhead: prev-physical pointer + size word. Payload starts right after.
	static constexpr size_t kHeaderSize = 2 * sizeof(void*);
	// Smallest payload: a free block must hold its next/prev free-list links.
	static constexpr size_t kMinBlock = 2 * sizeof(void*);
	static constexpr size_t kMaxAlloc = Internal::TlsfHeapSize - 2 * kHeaderSize;
	static constexpr size_t kGrowChunk = static_cast<size_t>(2) << 20;

	SPEC_MEM_STATIC_ASSERT(kSlCount <= 32, "SL bitmap is uint32_t");
	SPEC_MEM_STATIC_ASSERT(kFlCount <= 32, "FL bitmap is uint32_t");
	SPEC_MEM_STATIC_ASSERT(kAlign >= 2 * sizeof(void*), "free block must hold next/prev links");
	SPEC_MEM_STATIC_ASSERT(Internal::TlsfHeapSize > kSmallLimit, "heap smaller than the linear small-size range");
	SPEC_MEM_STATIC_ASSERT(kHeaderSize % kAlign == 0, "header must keep payloads aligned");
	SPEC_MEM_STATIC_ASSERT(kMinBlock % kAlign == 0, "min block must keep successors aligned");
	SPEC_MEM_STATIC_ASSERT(Internal::TlsfHeapSize % kGrowChunk == 0, "pool end must land exactly on the slice end");

	// Free-list links overlay the payload, so they are only valid while the block is free.
	struct BlockHeader {
		BlockHeader* m_PrevPhys;
		size_t m_SizeAndFlags;
		BlockHeader* m_NextFree;
		BlockHeader* m_PrevFree;
	};

	SPEC_MEM_STATIC_ASSERT(offsetof(BlockHeader, m_NextFree) == kHeaderSize, "payload must start at kHeaderSize");

	// Single-threaded on purpose: locking and remote frees layer on top, never in here.
	class SPEC_MEM_RUNTIME_API TlsfHeap final {
		MemoryHandle m_Handle{};
		uint8_t* m_Base = nullptr;
		size_t m_PoolEnd = 0;
		BlockHeader* m_Sentinel = nullptr;

		uint32_t m_FlBitmap = 0;
		uint32_t m_SlBitmap[kFlCount]{};
		BlockHeader* m_FreeLists[kFlCount][kSlCount]{};

	public:
		TlsfHeap() = default;

		TlsfHeap(const TlsfHeap&) = delete;
		TlsfHeap& operator=(const TlsfHeap&) = delete;

		TlsfHeap(TlsfHeap&&) = delete;
		TlsfHeap& operator=(TlsfHeap&&) = delete;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocate(size_t v_Bytes);
		// v_Alignment must be a power of two; anything <= kAlign takes the plain path.
		SPEC_MEM_NODISCARD void* allocateAligned(size_t v_Bytes, size_t v_Alignment);
		void deallocate(void* p_Ptr);

		SPEC_MEM_NODISCARD bool owns(const void* p_Ptr) const;
		// O(n), debug/tests only.
		SPEC_MEM_NODISCARD bool checkHeap() const;

	private:
		// Finds (growing once if needed) and unlinks a free block of at least v_Size.
		SPEC_MEM_NODISCARD BlockHeader* acquireBlock(size_t v_Size);
		void insertFree(BlockHeader* p_Block);
		void removeFree(BlockHeader* p_Block);
		SPEC_MEM_NODISCARD BlockHeader* findSuitable(size_t v_Fl, size_t v_Sl) const;

		void split(BlockHeader* p_Block, size_t v_Size);
		SPEC_MEM_NODISCARD BlockHeader* mergePrev(BlockHeader* p_Block);
		void mergeNext(BlockHeader* p_Block);

		SPEC_MEM_NODISCARD bool grow(size_t v_Size);
	};

	// Thread-safe front for one TlsfHeap. A free that finds the lock taken never blocks: it pushes
	// onto a lock-free stack, and whoever next holds the lock folds those blocks back in.
	class SPEC_MEM_RUNTIME_API SyncHeap final {
		TlsfHeap m_Heap;
		Corium::Runtime::Sync::CriticalSection m_Lock;
		alignas(64) Platform::Runtime::Atomic::AtomicPointer<void> m_RemoteFrees{ 0 };

	public:
		SyncHeap() = default;

		SyncHeap(const SyncHeap&) = delete;
		SyncHeap& operator=(const SyncHeap&) = delete;

		SyncHeap(SyncHeap&&) = delete;
		SyncHeap& operator=(SyncHeap&&) = delete;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocate(size_t v_Bytes);
		SPEC_MEM_NODISCARD void* allocateAligned(size_t v_Bytes, size_t v_Alignment);
		void deallocate(void* p_Ptr);

		SPEC_MEM_NODISCARD bool owns(const void* p_Ptr) const;
		SPEC_MEM_NODISCARD bool checkHeap();

	private:
		// Caller must hold m_Lock.
		void drainRemoteFrees();
	};

	// One SyncHeap per active NUMA node, each over that node's g_TlsfHeap slice.
	class SPEC_MEM_RUNTIME_API HeapRegistry final {
		static SyncHeap s_heaps[Internal::MAX_NUMA_NODES];
		static size_t s_nodeCount;

	public:
		// Requires Internal::init() to have carved the address space first.
		static void init();
		SPEC_MEM_NODISCARD static bool isInit();
		SPEC_MEM_NODISCARD static size_t nodeCount();

		SPEC_MEM_NODISCARD static void* allocate(size_t v_Bytes, size_t v_Node = 0);
		SPEC_MEM_NODISCARD static void* allocateAligned(size_t v_Bytes, size_t v_Alignment, size_t v_Node = 0);
		// Routes to the owning node's heap by address, so any thread may free any pointer.
		static void deallocate(void* p_Ptr);

		SPEC_MEM_NODISCARD static SyncHeap& forNode(size_t v_Node);
		SPEC_MEM_NODISCARD static SyncHeap* ownerOf(const void* p_Ptr);
	};
}
