#include "SpectraMemory.h"
#include "SpectraAllocators.h"
#include "VAUtils.h"

namespace Spectra::Memory::Allocators {
	void LinearArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* LinearArena::allocateImpl(size_t v_Bytes) {
		SPEC_MEM_ASSERT(m_Cursor + v_Bytes <= m_Handle.m_Memory.m_TotalSize);
		const size_t offset = m_Cursor;
		const bool committed = commitPageIfNeeded(m_Handle, offset + v_Bytes - 1);
		SPEC_MEM_ASSERT(committed);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + offset;
		m_Cursor += v_Bytes;
		return ptr;
	}

	void* LinearArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		size_t alignedOffset = (m_Cursor + v_Alignment - 1) & ~(v_Alignment - 1);
		size_t newOffset = alignedOffset + v_Bytes;
		SPEC_MEM_ASSERT(newOffset <= m_Handle.m_Memory.m_TotalSize);
		const bool committed = commitPageIfNeeded(m_Handle, newOffset - 1);
		SPEC_MEM_ASSERT(committed);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + alignedOffset;
		m_Cursor = newOffset;
		return ptr;
	}

	void LinearArena::deallocateImpl() {
		m_Cursor = 0;
	}

	void StackArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
		m_Cursor = 0;
	}

	void* StackArena::allocateImpl(size_t v_Bytes) {
		SPEC_MEM_ASSERT(m_Cursor + v_Bytes <= m_Handle.m_Memory.m_TotalSize);
		const size_t offset = m_Cursor;
		const bool committed = commitPageIfNeeded(m_Handle, offset + v_Bytes - 1);
		SPEC_MEM_ASSERT(committed);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + offset;
		m_Cursor += v_Bytes;
		return ptr;
	}

	void* StackArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		size_t alignedOffset = (m_Cursor + v_Alignment - 1) & ~(v_Alignment - 1);
		size_t newOffset = alignedOffset + v_Bytes;
		SPEC_MEM_ASSERT(newOffset <= m_Handle.m_Memory.m_TotalSize);
		const bool committed = commitPageIfNeeded(m_Handle, newOffset - 1);
		SPEC_MEM_ASSERT(committed);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + alignedOffset;
		m_Cursor = newOffset;
		return ptr;
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
}
