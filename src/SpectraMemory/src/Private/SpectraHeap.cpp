#include "SpectraMemory.h"
#include "SpectraHeap.h"
#include "VAUtils.h"

namespace Spectra::Memory::Heap {
	namespace {
		// Sizes are kAlign multiples, so bit 0 of the size word is spare.
		constexpr size_t kFreeBit = 1;

		std::pair<size_t, size_t> map(size_t v_Size) {
			if (v_Size < kSmallLimit) return { 0, v_Size / (kSmallLimit / kSlCount) };
			const size_t f = static_cast<size_t>(std::bit_width(v_Size)) - 1;
			const size_t sl = (v_Size >> (f - kSlLog2)) ^ kSlCount;
			const size_t fl = f - (kFlShift - 1);
			return { fl, sl };
		}

		std::pair<size_t, size_t> mapSearch(size_t v_Size) {
			if (v_Size >= kSmallLimit) v_Size += (static_cast<size_t>(1) << (static_cast<size_t>(std::bit_width(v_Size)) - 1 - kSlLog2)) - 1;
			return map(v_Size);
		}

		size_t alignUp(size_t v_Value, size_t v_Alignment) {
			return (v_Value + v_Alignment - 1) & ~(v_Alignment - 1);
		}

		size_t sizeOf(const BlockHeader* p_Block) {
			return p_Block->m_SizeAndFlags & ~kFreeBit;
		}

		bool isFree(const BlockHeader* p_Block) {
			return (p_Block->m_SizeAndFlags & kFreeBit) != 0;
		}

		void setSize(BlockHeader* p_Block, size_t v_Size) {
			p_Block->m_SizeAndFlags = v_Size | (p_Block->m_SizeAndFlags & kFreeBit);
		}

		void setFree(BlockHeader* p_Block, bool v_Free) {
			p_Block->m_SizeAndFlags = sizeOf(p_Block) | (v_Free ? kFreeBit : 0);
		}

		uint8_t* payloadOf(BlockHeader* p_Block) {
			return reinterpret_cast<uint8_t*>(p_Block) + kHeaderSize;
		}

		BlockHeader* headerOf(void* p_Payload) {
			return reinterpret_cast<BlockHeader*>(static_cast<uint8_t*>(p_Payload) - kHeaderSize);
		}

		BlockHeader* nextPhys(BlockHeader* p_Block) {
			return reinterpret_cast<BlockHeader*>(payloadOf(p_Block) + sizeOf(p_Block));
		}
	}

	void TlsfHeap::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Base = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress);
		m_PoolEnd = 0;
		m_Sentinel = nullptr;
		m_FlBitmap = 0;
		for (size_t fl = 0; fl < kFlCount; ++fl) {
			m_SlBitmap[fl] = 0;
			for (size_t sl = 0; sl < kSlCount; ++sl) m_FreeLists[fl][sl] = nullptr;
		}

		const bool grown = grow(0);
		SPEC_MEM_ASSERT(grown);
	}

	void* TlsfHeap::allocate(size_t v_Bytes) {
		if (v_Bytes == 0 || v_Bytes > kMaxAlloc) return nullptr;
		const size_t size = alignUp(v_Bytes < kMinBlock ? kMinBlock : v_Bytes, kAlign);

		BlockHeader* block = acquireBlock(size);
		if (!block) return nullptr;

		split(block, size);
		setFree(block, false);
		return payloadOf(block);
	}

	void* TlsfHeap::allocateAligned(size_t v_Bytes, size_t v_Alignment) {
		SPEC_MEM_ASSERT(std::has_single_bit(v_Alignment));
		if (v_Alignment <= kAlign) return allocate(v_Bytes);
		if (v_Bytes == 0 || v_Bytes > kMaxAlloc) return nullptr;
		const size_t size = alignUp(v_Bytes < kMinBlock ? kMinBlock : v_Bytes, kAlign);

		// Worst case the aligned spot sits a full alignment plus one minimal leading block in.
		const size_t request = size + v_Alignment + kHeaderSize + kMinBlock;
		if (request > kMaxAlloc) return nullptr;

		BlockHeader* block = acquireBlock(request);
		if (!block) return nullptr;

		uint8_t* payload = payloadOf(block);
		uint8_t* aligned = reinterpret_cast<uint8_t*>(alignUp(reinterpret_cast<uintptr_t>(payload), v_Alignment));
		// A non-zero gap must hold a whole free block, else push to the next aligned spot.
		if (aligned != payload && static_cast<size_t>(aligned - payload) < kHeaderSize + kMinBlock)
			aligned = reinterpret_cast<uint8_t*>(alignUp(reinterpret_cast<uintptr_t>(payload + kHeaderSize + kMinBlock), v_Alignment));

		const size_t gap = static_cast<size_t>(aligned - payload);
		if (gap != 0) {
			auto* alignedBlock = reinterpret_cast<BlockHeader*>(aligned - kHeaderSize);
			alignedBlock->m_PrevPhys = block;
			alignedBlock->m_SizeAndFlags = (sizeOf(block) - gap) | kFreeBit;
			nextPhys(alignedBlock)->m_PrevPhys = alignedBlock;

			// The leading block's physical predecessor was already non-free (block was free), so no merge.
			setSize(block, gap - kHeaderSize);
			insertFree(block);
			block = alignedBlock;
		}

		split(block, size);
		setFree(block, false);
		return payloadOf(block);
	}

	BlockHeader* TlsfHeap::acquireBlock(size_t v_Size) {
		const auto [fl, sl] = mapSearch(v_Size);
		BlockHeader* block = findSuitable(fl, sl);
		if (!block) {
			if (!grow(v_Size)) return nullptr;
			block = findSuitable(fl, sl);
			if (!block) return nullptr;
		}
		removeFree(block);
		return block;
	}

	void TlsfHeap::deallocate(void* p_Ptr) {
		if (!p_Ptr) return;
		SPEC_MEM_ASSERT(owns(p_Ptr));

		BlockHeader* block = headerOf(p_Ptr);
		SPEC_MEM_ASSERT(!isFree(block)); // double free
		setFree(block, true);

		// Next first: mergePrev may move the block start, mergeNext never does.
		mergeNext(block);
		block = mergePrev(block);
		insertFree(block);
	}

	bool TlsfHeap::owns(const void* p_Ptr) const {
		const auto* ptr = static_cast<const uint8_t*>(p_Ptr);
		return ptr >= m_Base && ptr < m_Base + m_PoolEnd;
	}

	void TlsfHeap::insertFree(BlockHeader* p_Block) {
		const auto [fl, sl] = map(sizeOf(p_Block));
		BlockHeader* head = m_FreeLists[fl][sl];

		p_Block->m_NextFree = head;
		p_Block->m_PrevFree = nullptr;
		if (head) head->m_PrevFree = p_Block;
		m_FreeLists[fl][sl] = p_Block;

		m_SlBitmap[fl] |= static_cast<uint32_t>(1) << sl;
		m_FlBitmap |= static_cast<uint32_t>(1) << fl;
	}

	void TlsfHeap::removeFree(BlockHeader* p_Block) {
		const auto [fl, sl] = map(sizeOf(p_Block));

		if (p_Block->m_PrevFree) p_Block->m_PrevFree->m_NextFree = p_Block->m_NextFree;
		else m_FreeLists[fl][sl] = p_Block->m_NextFree;
		if (p_Block->m_NextFree) p_Block->m_NextFree->m_PrevFree = p_Block->m_PrevFree;

		if (!m_FreeLists[fl][sl]) {
			m_SlBitmap[fl] &= ~(static_cast<uint32_t>(1) << sl);
			if (!m_SlBitmap[fl]) m_FlBitmap &= ~(static_cast<uint32_t>(1) << fl);
		}
	}

	BlockHeader* TlsfHeap::findSuitable(size_t v_Fl, size_t v_Sl) const {
		uint32_t slMap = m_SlBitmap[v_Fl] & (~static_cast<uint32_t>(0) << v_Sl);
		if (!slMap) {
			// kFlCount < 32, so this shift never reaches 32 (UB).
			const uint32_t flMap = m_FlBitmap & (~static_cast<uint32_t>(0) << (v_Fl + 1));
			if (!flMap) return nullptr;
			v_Fl = static_cast<size_t>(std::countr_zero(flMap));
			slMap = m_SlBitmap[v_Fl];
		}
		return m_FreeLists[v_Fl][static_cast<size_t>(std::countr_zero(slMap))];
	}

	void TlsfHeap::split(BlockHeader* p_Block, size_t v_Size) {
		const size_t current = sizeOf(p_Block);
		if (current < v_Size + kHeaderSize + kMinBlock) return;

		auto* remainder = reinterpret_cast<BlockHeader*>(payloadOf(p_Block) + v_Size);
		remainder->m_PrevPhys = p_Block;
		remainder->m_SizeAndFlags = (current - v_Size - kHeaderSize) | kFreeBit;
		nextPhys(remainder)->m_PrevPhys = remainder;

		setSize(p_Block, v_Size);
		// A free block's successor is never free, so the remainder needs no merge.
		insertFree(remainder);
	}

	BlockHeader* TlsfHeap::mergePrev(BlockHeader* p_Block) {
		BlockHeader* prev = p_Block->m_PrevPhys;
		if (!prev || !isFree(prev)) return p_Block;

		removeFree(prev);
		setSize(prev, sizeOf(prev) + kHeaderSize + sizeOf(p_Block));
		nextPhys(prev)->m_PrevPhys = prev;
		return prev;
	}

	void TlsfHeap::mergeNext(BlockHeader* p_Block) {
		BlockHeader* next = nextPhys(p_Block);
		if (!isFree(next)) return; // sentinel is never free

		removeFree(next);
		setSize(p_Block, sizeOf(p_Block) + kHeaderSize + sizeOf(next));
		nextPhys(p_Block)->m_PrevPhys = p_Block;
	}

	bool TlsfHeap::grow(size_t v_Size) {
		// Headroom for mapSearch's round-up (<= one slice) and two headers, so the retry in allocate always hits.
		const size_t needed = v_Size + (v_Size >> kSlLog2) + 2 * kHeaderSize;
		const size_t growBy = alignUp(needed < kGrowChunk ? kGrowChunk : needed, kGrowChunk);
		const size_t newEnd = m_PoolEnd + growBy;
		if (newEnd > m_Handle.m_Memory.m_TotalSize) return false;
		if (!commitPageIfNeeded(m_Handle, newEnd - 1)) return false;

		// The old sentinel becomes the new free block.
		BlockHeader* block = m_Sentinel;
		if (!block) {
			block = reinterpret_cast<BlockHeader*>(m_Base);
			block->m_PrevPhys = nullptr;
		}

		auto* sentinel = reinterpret_cast<BlockHeader*>(m_Base + newEnd - kHeaderSize);
		block->m_SizeAndFlags = static_cast<size_t>(reinterpret_cast<uint8_t*>(sentinel) - payloadOf(block)) | kFreeBit;
		sentinel->m_PrevPhys = block;
		sentinel->m_SizeAndFlags = 0;

		m_Sentinel = sentinel;
		m_PoolEnd = newEnd;

		insertFree(mergePrev(block));
		return true;
	}

	bool TlsfHeap::checkHeap() const {
		if (!m_Sentinel) return true;

		size_t physicalFree = 0;
		BlockHeader* prev = nullptr;
		auto* block = reinterpret_cast<BlockHeader*>(m_Base);
		while (block != m_Sentinel) {
			if (block->m_PrevPhys != prev) return false;
			if (sizeOf(block) < kMinBlock || sizeOf(block) % kAlign != 0) return false;
			if (isFree(block)) {
				if (prev && isFree(prev)) return false;
				const auto [fl, sl] = map(sizeOf(block));
				if (!((m_SlBitmap[fl] >> sl) & 1)) return false;
				++physicalFree;
			}
			prev = block;
			block = nextPhys(block);
			if (reinterpret_cast<uint8_t*>(block) > reinterpret_cast<uint8_t*>(m_Sentinel)) return false;
		}
		if (m_Sentinel->m_PrevPhys != prev || m_Sentinel->m_SizeAndFlags != 0) return false;

		size_t listedFree = 0;
		for (size_t fl = 0; fl < kFlCount; ++fl) {
			if (((m_FlBitmap >> fl) & 1) != (m_SlBitmap[fl] != 0 ? 1u : 0u)) return false;
			for (size_t sl = 0; sl < kSlCount; ++sl) {
				const BlockHeader* node = m_FreeLists[fl][sl];
				if (((m_SlBitmap[fl] >> sl) & 1) != (node ? 1u : 0u)) return false;
				const BlockHeader* expectedPrev = nullptr;
				for (; node; node = node->m_NextFree) {
					if (!isFree(node) || node->m_PrevFree != expectedPrev) return false;
					const auto [nodeFl, nodeSl] = map(sizeOf(node));
					if (nodeFl != fl || nodeSl != sl) return false;
					expectedPrev = node;
					++listedFree;
				}
			}
		}
		return listedFree == physicalFree;
	}

	void SyncHeap::init(MemoryHandle* p_Handle) {
		m_Lock.lock();
		m_Heap.init(p_Handle);
		m_RemoteFrees.store(0, Platform::Runtime::Intrinsic::MemoryOrder::RELEASE);
		m_Lock.unlock();
	}

	void* SyncHeap::allocate(size_t v_Bytes) {
		m_Lock.lock();
		drainRemoteFrees();
		void* ptr = m_Heap.allocate(v_Bytes);
		m_Lock.unlock();
		return ptr;
	}

	void* SyncHeap::allocateAligned(size_t v_Bytes, size_t v_Alignment) {
		m_Lock.lock();
		drainRemoteFrees();
		void* ptr = m_Heap.allocateAligned(v_Bytes, v_Alignment);
		m_Lock.unlock();
		return ptr;
	}

	void SyncHeap::deallocate(void* p_Ptr) {
		if (!p_Ptr) return;
		if (m_Lock.tryLock()) {
			drainRemoteFrees();
			m_Heap.deallocate(p_Ptr);
			m_Lock.unlock();
			return;
		}

		// Push-only + drain-by-exchange means no pop races, so ABA can't happen here.
		using Platform::Runtime::Intrinsic::MemoryOrder;
		auto* link = static_cast<uintptr_t*>(p_Ptr);
		uintptr_t expected = m_RemoteFrees.load(MemoryOrder::RELAXED);
		for (;;) {
			*link = expected;
			const uintptr_t seen = expected;
			if (m_RemoteFrees.compareExchange(&expected, reinterpret_cast<uintptr_t>(p_Ptr), MemoryOrder::RELEASE, MemoryOrder::RELAXED) == seen) return;
		}
	}

	bool SyncHeap::owns(const void* p_Ptr) const {
		return m_Heap.owns(p_Ptr);
	}

	bool SyncHeap::checkHeap() {
		m_Lock.lock();
		drainRemoteFrees();
		const bool ok = m_Heap.checkHeap();
		m_Lock.unlock();
		return ok;
	}

	void SyncHeap::drainRemoteFrees() {
		using Platform::Runtime::Intrinsic::MemoryOrder;
		uintptr_t node = m_RemoteFrees.exchange(0, MemoryOrder::ACQ_REL);
		while (node) {
			const uintptr_t next = *reinterpret_cast<uintptr_t*>(node);
			m_Heap.deallocate(reinterpret_cast<void*>(node));
			node = next;
		}
	}

	SyncHeap HeapRegistry::s_heaps[Internal::MAX_NUMA_NODES];
	size_t HeapRegistry::s_nodeCount = 0;

	void HeapRegistry::init() {
		SPEC_MEM_ASSERT(s_nodeCount == 0);
		for (size_t node = 0; node < Internal::MAX_NUMA_NODES; ++node) {
			// Internal::init() leaves slots past the physical node count invalid.
			if (!Internal::g_TlsfHeap[node].isValid()) break;
			MemoryHandle handle = segmentFromRegion(Internal::g_TlsfHeap[node]);
			handle.m_NumaNode = node;
			s_heaps[node].init(&handle);
			s_nodeCount = node + 1;
		}
		SPEC_MEM_ASSERT(s_nodeCount > 0);
	}

	bool HeapRegistry::isInit() {
		return s_nodeCount != 0;
	}

	size_t HeapRegistry::nodeCount() {
		return s_nodeCount;
	}

	void* HeapRegistry::allocate(size_t v_Bytes, size_t v_Node) {
		return forNode(v_Node).allocate(v_Bytes);
	}

	void* HeapRegistry::allocateAligned(size_t v_Bytes, size_t v_Alignment, size_t v_Node) {
		return forNode(v_Node).allocateAligned(v_Bytes, v_Alignment);
	}

	void HeapRegistry::deallocate(void* p_Ptr) {
		if (!p_Ptr) return;
		SyncHeap* owner = ownerOf(p_Ptr);
		SPEC_MEM_ASSERT(owner);
		owner->deallocate(p_Ptr);
	}

	SyncHeap& HeapRegistry::forNode(size_t v_Node) {
		SPEC_MEM_ASSERT(v_Node < s_nodeCount);
		return s_heaps[v_Node];
	}

	SyncHeap* HeapRegistry::ownerOf(const void* p_Ptr) {
		for (size_t node = 0; node < s_nodeCount; ++node)
			if (s_heaps[node].owns(p_Ptr)) return &s_heaps[node];
		return nullptr;
	}
}
