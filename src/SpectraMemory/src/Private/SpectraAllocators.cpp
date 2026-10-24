#include "SpectraMemory.h"
#include "SpectraAllocators.h"
#include "VAUtils.h"
#include <algorithm>

namespace Spectra::Memory::Allocators {
	namespace {
		// Shared bump step for both arenas. Returns nullptr instead of asserting, same contract as TlsfHeap.
		void* bump(MemoryHandle& r_Handle, size_t& r_Cursor, size_t v_Bytes, size_t v_Alignment) {
			if (v_Bytes == 0 || !std::has_single_bit(v_Alignment)) return nullptr;
			const size_t total = r_Handle.m_Memory.m_TotalSize;
			const size_t alignedOffset = (r_Cursor + v_Alignment - 1) & ~(v_Alignment - 1);
			// Subtraction form so a huge v_Bytes can't wrap past the bound.
			if (alignedOffset < r_Cursor || alignedOffset > total || v_Bytes > total - alignedOffset) return nullptr;

			const size_t newOffset = alignedOffset + v_Bytes;
			if (!commitPageIfNeeded(r_Handle, newOffset - 1)) return nullptr;
			r_Cursor = newOffset;
			return static_cast<uint8_t*>(r_Handle.m_Memory.m_BaseAddress) + alignedOffset;
		}
	}

	void LinearArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* LinearArena::allocateImpl(size_t v_Bytes) {
		return bump(m_Handle, m_Cursor, v_Bytes, 1);
	}

	void* LinearArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		return bump(m_Handle, m_Cursor, v_Bytes, v_Alignment);
	}

	void LinearArena::deallocateImpl() {
		m_Cursor = 0;
	}

	void StackArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* StackArena::allocateImpl(size_t v_Bytes) {
		return bump(m_Handle, m_Cursor, v_Bytes, 1);
	}

	void* StackArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		return bump(m_Handle, m_Cursor, v_Bytes, v_Alignment);
	}

	void StackArena::deallocateImpl() {
		m_Cursor = 0;
	}

	StackArena::Marker StackArena::mark() const {
		return m_Cursor;
	}

	void StackArena::unwind(Marker v_Marker) {
		SPEC_MEM_ASSERT(v_Marker <= m_Cursor);
		m_Cursor = v_Marker;
	}

	void ReclaimableLinearArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* ReclaimableLinearArena::allocateImpl(size_t v_Bytes) {
		return bump(m_Handle, m_Cursor, v_Bytes, 1);
	}

	void* ReclaimableLinearArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		return bump(m_Handle, m_Cursor, v_Bytes, v_Alignment);
	}

	void ReclaimableLinearArena::deallocateImpl() {
		m_Cursor = 0;
		decommitAbove(m_Handle, 0);
	}

	void ReclaimableLinearArena::trim(size_t v_KeepBytes) {
		decommitAbove(m_Handle, std::max(m_Cursor, v_KeepBytes));
	}

	void ReclaimableStackArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* ReclaimableStackArena::allocateImpl(size_t v_Bytes) {
		return bump(m_Handle, m_Cursor, v_Bytes, 1);
	}

	void* ReclaimableStackArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		return bump(m_Handle, m_Cursor, v_Bytes, v_Alignment);
	}

	void ReclaimableStackArena::deallocateImpl() {
		m_Cursor = 0;
		decommitAbove(m_Handle, 0);
	}

	ReclaimableStackArena::Marker ReclaimableStackArena::mark() const {
		return m_Cursor;
	}

	void ReclaimableStackArena::unwind(Marker v_Marker) {
		SPEC_MEM_ASSERT(v_Marker <= m_Cursor);
		m_Cursor = v_Marker;
		decommitAbove(m_Handle, m_Cursor);
	}

	void ReclaimableStackArena::trim(size_t v_KeepBytes) {
		decommitAbove(m_Handle, std::max(m_Cursor, v_KeepBytes));
	}
}

namespace Spectra::Memory::Allocators {
	namespace {
		constexpr size_t PAGE = 4_KiB;

		void changeRange(const MemoryHandle& ro_Owner, size_t v_Offset, size_t v_Bytes, MemoryState v_State) {
			// Scratch handle: these ranges are not the committed prefix Memory:: tracks on the owner.
			MemoryHandle scratch = ro_Owner;
			VirtualMemoryDesc desc{};
			initMemoryDesc(desc);
			setTargetAddress(desc, static_cast<uint8_t*>(ro_Owner.m_Memory.m_BaseAddress) + v_Offset);
			setSize(desc, v_Bytes);
			setMemoryState(desc, v_State);
			if (v_State == MemoryState::COMMIT) {
				setNumaNode(desc, static_cast<uint32_t>(ro_Owner.m_NumaNode));
				setProtection(desc, MemoryProtect::READ_WRITE);
				Memory::commit(scratch, desc);
			} else {
				Memory::decommit(scratch, desc);
			}
		}

		bool isFree(const uint64_t* p_Bits, size_t v_Block) {
			return (p_Bits[v_Block / 64] >> (v_Block % 64)) & 1;
		}

		void setFree(uint64_t* p_Bits, size_t v_First, size_t v_Count, bool v_Free) {
			for (size_t block = v_First; block < v_First + v_Count; ++block) {
				if (v_Free) p_Bits[block / 64] |= 1ull << (block % 64);
				else p_Bits[block / 64] &= ~(1ull << (block % 64));
			}
		}

		size_t blocksFor(size_t v_Bytes) {
			return (v_Bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
		}
	}

	void LazyStackArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
		m_HighWater = 0;
	}

	void* LazyStackArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		if (v_Bytes == 0 || !std::has_single_bit(v_Alignment)) return nullptr;
		const size_t total = m_Handle.m_Memory.m_TotalSize;
		const size_t start = (m_Cursor + v_Alignment - 1) & ~(v_Alignment - 1);
		if (start < m_Cursor || start > total || v_Bytes > total - start) return nullptr;

		if (m_CommitOnTouch) changeRange(m_Handle, start & ~(PAGE - 1), PAGE, MemoryState::COMMIT);
		m_Cursor = start + v_Bytes;
		m_HighWater = std::max(m_HighWater, m_Cursor);
		return static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + start;
	}

	void* LazyStackArena::allocateImpl(size_t v_Bytes) {
		return allocateImpl(v_Bytes, 1);
	}

	void LazyStackArena::deallocateImpl() {
		unwind(0);
	}

	LazyStackArena::Marker LazyStackArena::mark() const {
		return m_Cursor;
	}

	void LazyStackArena::unwind(Marker v_Marker) {
		SPEC_MEM_ASSERT(v_Marker <= m_Cursor);
		m_Cursor = v_Marker;
		// Keep the page the marker sits in; it can still hold live bytes below it.
		const size_t keep = (v_Marker + PAGE - 1) & ~(PAGE - 1);
		if (keep < m_HighWater) {
			if (m_CommitOnTouch) changeRange(m_Handle, keep, m_HighWater - keep, MemoryState::DECOMMIT);
			m_HighWater = keep;
		}
	}

	const MemoryHandle& LazyStackArena::handle() const {
		return m_Handle;
	}

	void LazyStackArena::setCommitOnTouch(bool v_Commit) {
		m_CommitOnTouch = v_Commit;
	}

	bool LazyStackArena::commitsOnTouch() const {
		return m_CommitOnTouch;
	}

	void BlockedAllocator::setPlaceholder(bool v_Placeholder) {
		m_UnderlyingArena.setCommitOnTouch(!v_Placeholder);
	}

	bool BlockedAllocator::isPlaceholder() const {
		return !m_UnderlyingArena.commitsOnTouch();
	}

	void* BlockedAllocator::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		if (!std::has_single_bit(v_Alignment) || v_Alignment > BLOCK_SIZE) return nullptr;
		return allocateImpl(v_Bytes);
	}

	void* BlockedAllocator::allocateImpl(size_t v_Bytes) {
		const size_t wanted = blocksFor(v_Bytes);
		if (wanted == 0) return nullptr;

		m_Lock.lock();
		const MemoryHandle& handle = m_UnderlyingArena.handle();
		const size_t top = m_UnderlyingArena.mark() / BLOCK_SIZE;
		size_t start = 0;
		size_t length = 0;
		for (size_t block = 0; block < top && length < wanted; ++block) {
			if (isFree(m_Free, block)) {
				++length;
			} else {
				length = 0;
				start = block + 1;
			}
		}

		void* ptr = nullptr;
		if (length == wanted) {
			setFree(m_Free, start, wanted, false);
			if (!isPlaceholder()) changeRange(handle, start * BLOCK_SIZE, PAGE, MemoryState::COMMIT);
			ptr = static_cast<uint8_t*>(handle.m_Memory.m_BaseAddress) + start * BLOCK_SIZE;
		} else if (top + wanted <= MAX_BLOCKS) {
			ptr = m_UnderlyingArena.allocate(wanted * BLOCK_SIZE);
		}
		m_Lock.unlock();
		return ptr;
	}

	void BlockedAllocator::deallocateImpl(void* p_Ptr, size_t v_Bytes) {
		if (p_Ptr == nullptr) return;
		SPEC_MEM_ASSERT(owns(p_Ptr));

		m_Lock.lock();
		const MemoryHandle& handle = m_UnderlyingArena.handle();
		const size_t first = (static_cast<uint8_t*>(p_Ptr) - static_cast<uint8_t*>(handle.m_Memory.m_BaseAddress)) / BLOCK_SIZE;
		const size_t count = blocksFor(v_Bytes);
		setFree(m_Free, first, count, true);

		size_t top = m_UnderlyingArena.mark() / BLOCK_SIZE;
		if (first + count == top) {
			while (top > 0 && isFree(m_Free, top - 1)) setFree(m_Free, --top, 1, false);
			m_UnderlyingArena.unwind(top * BLOCK_SIZE);
		} else if (!isPlaceholder()) {
			changeRange(handle, first * BLOCK_SIZE, count * BLOCK_SIZE, MemoryState::DECOMMIT);
		}
		m_Lock.unlock();
	}

	bool BlockedAllocator::owns(const void* p_Ptr) const {
		const MemoryHandle& handle = m_UnderlyingArena.handle();
		const uint8_t* base = static_cast<const uint8_t*>(handle.m_Memory.m_BaseAddress);
		const uint8_t* ptr = static_cast<const uint8_t*>(p_Ptr);
		return ptr >= base && ptr < base + handle.m_Memory.m_TotalSize;
	}

	MemoryHandle BlockedAllocator::handleFor(void* p_Ptr, size_t v_Bytes, size_t v_NumaNode) {
		MemoryHandle handle{};
		handle.m_Memory.m_BaseAddress = p_Ptr;
		handle.m_Memory.m_TotalSize = blocksFor(v_Bytes) * BLOCK_SIZE;
		handle.m_CommittedSize = 0;
		handle.m_NumaNode = v_NumaNode;
		return handle;
	}
}
