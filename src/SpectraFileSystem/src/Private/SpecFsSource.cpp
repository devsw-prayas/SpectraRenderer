#include "SpecFsSource.h"
#include "SpecFsDiagnostic.h"

#include <SpectraHeap.h>
#include <SpecMemRegistry.h>

namespace Spectra::FileSystem {
	void initSourceDesc(SourceDesc& r_Desc) noexcept {
		r_Desc = {};
		r_Desc.m_State = DescState::INITIALIZED;
	}

	void setKind(SourceDesc& r_Desc, SourceKind v_Kind) noexcept {
		SPEC_FS_ASSERT(r_Desc.m_State == DescState::INITIALIZED);
		r_Desc.m_Kind = v_Kind;
	}

	void setNumaNode(SourceDesc& r_Desc, uint32_t v_Node) noexcept {
		SPEC_FS_ASSERT(r_Desc.m_State == DescState::INITIALIZED);
		r_Desc.m_NumaNode = v_Node;
	}

	void setAlignment(SourceDesc& r_Desc, uint32_t v_Alignment) noexcept {
		SPEC_FS_ASSERT(r_Desc.m_State == DescState::INITIALIZED);
		r_Desc.m_Alignment = v_Alignment;
	}

	void setArena(SourceDesc& r_Desc, Memory::Allocators::LinearArena* p_Arena) noexcept {
		SPEC_FS_ASSERT(r_Desc.m_State == DescState::INITIALIZED);
		r_Desc.m_Arena = p_Arena;
	}

	void setStagingBytes(SourceDesc& r_Desc, size_t v_Bytes) noexcept {
		SPEC_FS_ASSERT(r_Desc.m_State == DescState::INITIALIZED);
		r_Desc.m_StagingBytes = v_Bytes;
	}

	bool validateSourceDesc(SourceDesc& r_Desc) noexcept {
		if (r_Desc.m_State != DescState::INITIALIZED) return false;
		if (!std::has_single_bit(r_Desc.m_Alignment)) return false;  // exec InvalidAlignment
		if (r_Desc.m_Kind == SourceKind::LINEAR_ARENA && r_Desc.m_Arena == nullptr) return false;  // exec InvalidSource
		if (r_Desc.m_Kind == SourceKind::DIRECT && r_Desc.m_StagingBytes == 0) return false;  // exec InvalidSource
		// Direct buffers must meet any sector size; 4 KiB covers every volume we target.
		if (r_Desc.m_Kind == SourceKind::DIRECT && r_Desc.m_Alignment < 4096) r_Desc.m_Alignment = 4096;
		r_Desc.m_State = DescState::VALIDATED;
		return true;
	}

	bool Sources::create(const SourceDesc& r_Desc, SourceHandle& r_Out) noexcept {
		if (r_Desc.m_State != DescState::VALIDATED) return false;  // exec InvalidSource
		r_Out.m_Kind = r_Desc.m_Kind;
		r_Out.m_NumaNode = r_Desc.m_NumaNode;
		r_Out.m_Alignment = r_Desc.m_Alignment;
		r_Out.m_Arena = r_Desc.m_Arena;
		r_Out.m_Run = nullptr;
		r_Out.m_RunBytes = 0;
		if (r_Desc.m_Kind != SourceKind::DIRECT) return true;

		using Memory::Allocators::BlockedAllocator;
		using Memory::Allocators::LinearArena;
		BlockedAllocator* staging = Memory::AtomicAllocators::instance().stagingBlocks(r_Desc.m_NumaNode);
		void* run = staging->allocate(r_Desc.m_StagingBytes);
		if (run == nullptr) return false;  // exec OutOfStaging
		void* slot = Memory::Heap::HeapRegistry::allocateAligned(sizeof(LinearArena), alignof(LinearArena), r_Desc.m_NumaNode);
		if (slot == nullptr) {
			staging->deallocate(run, r_Desc.m_StagingBytes);
			return false;  // exec OutOfMemory
		}
		Memory::MemoryHandle handle = BlockedAllocator::handleFor(run, r_Desc.m_StagingBytes, r_Desc.m_NumaNode);
		r_Out.m_Arena = ::new (slot) LinearArena(handle);
		r_Out.m_Run = run;
		r_Out.m_RunBytes = r_Desc.m_StagingBytes;
		return true;
	}

	void Sources::destroy(SourceHandle& r_Source) noexcept {
		if (r_Source.m_Kind != SourceKind::DIRECT || r_Source.m_Run == nullptr) return;
		r_Source.m_Arena->~LinearArena();
		Memory::Heap::HeapRegistry::deallocate(r_Source.m_Arena);
		Memory::AtomicAllocators::instance().stagingBlocks(r_Source.m_NumaNode)->deallocate(r_Source.m_Run, r_Source.m_RunBytes);
		r_Source.m_Arena = nullptr;
		r_Source.m_Run = nullptr;
		r_Source.m_RunBytes = 0;
	}

	void Sources::reset(SourceHandle& r_Source) noexcept {
		if (r_Source.m_Kind == SourceKind::DIRECT && r_Source.m_Arena != nullptr) r_Source.m_Arena->reset();
	}

	void* Sources::acquire(const SourceHandle& r_Source, size_t v_Bytes) noexcept {
		if (v_Bytes == 0) v_Bytes = 1;
		switch (r_Source.m_Kind) {
			case SourceKind::HEAP:
				return Memory::Heap::HeapRegistry::allocateAligned(v_Bytes, r_Source.m_Alignment, r_Source.m_NumaNode);
			case SourceKind::LINEAR_ARENA:
			case SourceKind::DIRECT:
				return r_Source.m_Arena->allocate(v_Bytes, r_Source.m_Alignment);
		}
		return nullptr;
	}

	void Sources::release(const SourceHandle& r_Source, void* p_Ptr, size_t) noexcept {
		if (p_Ptr == nullptr) return;
		if (r_Source.m_Kind == SourceKind::HEAP) Memory::Heap::HeapRegistry::deallocate(p_Ptr);
	}
}
