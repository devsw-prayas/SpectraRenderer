#include "SpectraMemory.h"
#include "SpectraAllocators.h"

namespace Spectra::Memory::Allocators {
	void LinearArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
	}

	void* LinearArena::allocateImpl(size_t v_Bytes) {
		SPEC_MEM_ASSERT(m_Handle.m_CommittedSize + v_Bytes <= m_Handle.m_Memory.m_TotalSize);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + m_Handle.m_CommittedSize;
		m_Handle.m_CommittedSize += v_Bytes;
		return ptr;
	}

	void* LinearArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		size_t alignedOffset = (m_Handle.m_CommittedSize + v_Alignment - 1) & ~(v_Alignment - 1);
		size_t newOffset = alignedOffset + v_Bytes;
		SPEC_MEM_ASSERT(newOffset <= m_Handle.m_Memory.m_TotalSize);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + alignedOffset;
		m_Handle.m_CommittedSize = newOffset;
		return ptr;
	}

	void LinearArena::deallocateImpl() {
		m_Handle.m_CommittedSize = 0;
	}

	void StackArena::init(MemoryHandle* p_Handle) {
		m_Handle = *p_Handle;
	}

	void* StackArena::allocateImpl(size_t v_Bytes) {
		SPEC_MEM_ASSERT(m_Handle.m_CommittedSize + v_Bytes <= m_Handle.m_Memory.m_TotalSize);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + m_Handle.m_CommittedSize;
		m_Handle.m_CommittedSize += v_Bytes;
		return ptr;
	}

	void* StackArena::allocateImpl(size_t v_Bytes, size_t v_Alignment) {
		size_t alignedOffset = (m_Handle.m_CommittedSize + v_Alignment - 1) & ~(v_Alignment - 1);
		size_t newOffset = alignedOffset + v_Bytes;
		SPEC_MEM_ASSERT(newOffset <= m_Handle.m_Memory.m_TotalSize);
		auto* ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + alignedOffset;
		m_Handle.m_CommittedSize = newOffset;
		return ptr;
	}

	void StackArena::deallocateImpl() {
		m_Handle.m_CommittedSize = 0;
	}

	StackArena::Marker StackArena::mark() const {
		return m_Handle.m_CommittedSize;
	}

	void StackArena::unwind(Marker v_Marker) {
		SPEC_MEM_ASSERT(v_Marker <= m_Handle.m_CommittedSize);
		m_Handle.m_CommittedSize = v_Marker;
	}
}
