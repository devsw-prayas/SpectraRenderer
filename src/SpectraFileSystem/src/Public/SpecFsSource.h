#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"

#include <SpectraAllocators.h>

namespace Spectra::FileSystem {
	enum class SourceKind : uint8_t {
		HEAP,          // HeapRegistry on a NUMA node
		LINEAR_ARENA,  // caller's arena; release is a no-op, the owner resets it
		DIRECT         // sector-aligned buffers in the file staging region, committed as used
	};

	enum class DescState : uint8_t {
		UNINITIALIZED,
		INITIALIZED,
		VALIDATED
	};

	// Built once per use case (configs, logs, asset streaming). Setters only before validate.
	struct SourceDesc {
		DescState m_State = DescState::UNINITIALIZED;
		SourceKind m_Kind = SourceKind::HEAP;
		uint32_t m_NumaNode = 0;
		uint32_t m_Alignment = 64;
		Memory::Allocators::LinearArena* m_Arena = nullptr;
		size_t m_StagingBytes = 0;
	};

	SPEC_FS_RUNTIME_API void initSourceDesc(SourceDesc& r_Desc) noexcept;
	SPEC_FS_RUNTIME_API void setKind(SourceDesc& r_Desc, SourceKind v_Kind) noexcept;
	SPEC_FS_RUNTIME_API void setNumaNode(SourceDesc& r_Desc, uint32_t v_Node) noexcept;
	SPEC_FS_RUNTIME_API void setAlignment(SourceDesc& r_Desc, uint32_t v_Alignment) noexcept;
	SPEC_FS_RUNTIME_API void setArena(SourceDesc& r_Desc, Memory::Allocators::LinearArena* p_Arena) noexcept;
	SPEC_FS_RUNTIME_API void setStagingBytes(SourceDesc& r_Desc, size_t v_Bytes) noexcept;
	// Freezes the desc. Fails on an arena kind without an arena, DIRECT without staging, or a bad alignment.
	SPEC_FS_NODISCARD SPEC_FS_RUNTIME_API bool validateSourceDesc(SourceDesc& r_Desc) noexcept;

	// Live binding of a mode to memory, tagged by kind. DIRECT owns a staging run and the arena over it.
	struct SourceHandle {
		SourceKind m_Kind = SourceKind::HEAP;
		uint32_t m_NumaNode = 0;
		uint32_t m_Alignment = 64;
		Memory::Allocators::LinearArena* m_Arena = nullptr;
		void* m_Run = nullptr;
		size_t m_RunBytes = 0;
	};

	class SPEC_FS_RUNTIME_API Sources final {
	public:
		Sources() = delete;

		SPEC_FS_NODISCARD static bool create(const SourceDesc& r_Desc, SourceHandle& r_Out) noexcept;
		// Gives a DIRECT handle's staging run back. No-op for the other kinds.
		static void destroy(SourceHandle& r_Source) noexcept;
		// Rewinds a DIRECT handle so its staging can be reused; everything acquired from it is invalid after.
		static void reset(SourceHandle& r_Source) noexcept;
		SPEC_FS_NODISCARD static void* acquire(const SourceHandle& r_Source, size_t v_Bytes) noexcept;
		static void release(const SourceHandle& r_Source, void* p_Ptr, size_t v_Bytes) noexcept;
	};
}
