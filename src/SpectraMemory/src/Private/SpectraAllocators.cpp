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
