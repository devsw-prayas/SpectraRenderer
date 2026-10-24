#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"
#include "SpecMemDiagnostics.h"
#include "MemBehavior.h"
#include "SpecMemAllocator.h"
#include "MemoryRegion.h"
#include "VAUtils.h"
#include <CoriumSync.h>

namespace Spectra::Memory::Allocators {

	// TODO: inject SpectraInstrumentation/Profiler hooks (allocate/deallocate) later.
	class SPEC_MEM_RUNTIME_API LinearArena : public IArena<LinearArena> {
		MemoryHandle m_Handle;
		// Logical bump position - deliberately NOT m_Handle.m_CommittedSize, which
		// commitPageIfNeeded/Memory::commit own as the OS-committed high-water mark.
		size_t m_Cursor = 0;
	public:
		LinearArena() = default;
		explicit LinearArena(const MemoryHandle& ro_Handle) : m_Handle(ro_Handle) {}

		// Non-copyable: two copies would bump over the same pages.
		LinearArena(const LinearArena&) = delete;
		LinearArena& operator=(const LinearArena&) = delete;

		LinearArena(LinearArena&&) noexcept = default;
		LinearArena& operator=(LinearArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();
	};

	SPEC_ARENA_METADATA(LinearArena, "Linear persistent arena", 0, "linear-arena")

	// TODO: inject SpectraInstrumentation/Profiler hooks (allocate/deallocate) later.
	class SPEC_MEM_RUNTIME_API StackArena : public IArena<StackArena> {
		MemoryHandle m_Handle;
		// Same split as LinearArena - see its m_Cursor comment.
		size_t m_Cursor = 0;
	public:
		using Marker = size_t;

		StackArena() = default;
		explicit StackArena(const MemoryHandle& ro_Handle) : m_Handle(ro_Handle) {}

		// Non-copyable: two copies would bump over the same pages.
		StackArena(const StackArena&) = delete;
		StackArena& operator=(const StackArena&) = delete;

		StackArena(StackArena&&) noexcept = default;
		StackArena& operator=(StackArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();

		SPEC_MEM_NODISCARD Marker mark() const;
		void unwind(Marker v_Marker);
	};

	SPEC_ARENA_METADATA(StackArena, "LIFO bump arena with mark/unwind rewind points", 1, "stack-arena")

	// TODO: Add a StackArenaScope RAII guard that calls unwind() on destruction.

	// Reclaimable variants: same bump behaviour, but reset/unwind hand pages back to the OS.
	// Non-copyable, since two copies would decommit pages the other still uses.

	// TODO: inject SpectraInstrumentation/Profiler hooks (allocate/deallocate) later.
	class SPEC_MEM_RUNTIME_API ReclaimableLinearArena : public IArena<ReclaimableLinearArena> {
		MemoryHandle m_Handle;
		size_t m_Cursor = 0;
	public:
		ReclaimableLinearArena() = default;
		explicit ReclaimableLinearArena(const MemoryHandle& ro_Handle) : m_Handle(ro_Handle) {}

		ReclaimableLinearArena(const ReclaimableLinearArena&) = delete;
		ReclaimableLinearArena& operator=(const ReclaimableLinearArena&) = delete;

		ReclaimableLinearArena(ReclaimableLinearArena&&) noexcept = default;
		ReclaimableLinearArena& operator=(ReclaimableLinearArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		// Rewinds to zero and decommits everything.
		void deallocateImpl();

		// Decommits above max(cursor, v_KeepBytes) without moving the cursor.
		void trim(size_t v_KeepBytes = 0);
	};

	SPEC_ARENA_METADATA(ReclaimableLinearArena, "Linear arena that decommits on reset", 2, "reclaimable-linear-arena")

	// TODO: inject SpectraInstrumentation/Profiler hooks (allocate/deallocate) later.
	class SPEC_MEM_RUNTIME_API ReclaimableStackArena : public IArena<ReclaimableStackArena> {
		MemoryHandle m_Handle;
		size_t m_Cursor = 0;
	public:
		using Marker = size_t;

		ReclaimableStackArena() = default;
		explicit ReclaimableStackArena(const MemoryHandle& ro_Handle) : m_Handle(ro_Handle) {}

		ReclaimableStackArena(const ReclaimableStackArena&) = delete;
		ReclaimableStackArena& operator=(const ReclaimableStackArena&) = delete;

		ReclaimableStackArena(ReclaimableStackArena&&) noexcept = default;
		ReclaimableStackArena& operator=(ReclaimableStackArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();

		SPEC_MEM_NODISCARD Marker mark() const;
		// Decommits whole chunks above the marker.
		void unwind(Marker v_Marker);

		void trim(size_t v_KeepBytes = 0);
	};

	SPEC_ARENA_METADATA(ReclaimableStackArena, "LIFO bump arena that decommits on unwind", 3, "reclaimable-stack-arena")

	// Tag-distinguished raw allocator over LinearArena.
	template<typename Tag>
	struct RawArenaAllocator final : IAllocator<void, LinearArena, RawArenaAllocator<Tag>> {};

	// Backing allocator for ClosureFunction/FunctionView payloads.
	using ClosureAllocator = RawArenaAllocator<struct ClosureTag>;

	// Stack arena that hands out reserved space: allocate commits only the page the allocation starts on,
	// the owner commits the rest as it touches it. unwind decommits everything above the marker.
	class SPEC_MEM_RUNTIME_API LazyStackArena : public IArena<LazyStackArena> {
		MemoryHandle m_Handle{};
		size_t m_Cursor = 0;
		size_t m_HighWater = 0;
		bool m_CommitOnTouch = true;

	public:
		using Marker = size_t;

		LazyStackArena() = default;

		LazyStackArena(const LazyStackArena&) = delete;
		LazyStackArena& operator=(const LazyStackArena&) = delete;

		LazyStackArena(LazyStackArena&&) noexcept = default;
		LazyStackArena& operator=(LazyStackArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();

		SPEC_MEM_NODISCARD Marker mark() const;
		void unwind(Marker v_Marker);

		SPEC_MEM_NODISCARD const MemoryHandle& handle() const;
		// Off for placeholder regions, where the owner maps instead of committing.
		void setCommitOnTouch(bool v_Commit);
		SPEC_MEM_NODISCARD bool commitsOnTouch() const;
	};

	SPEC_ARENA_METADATA(LazyStackArena, "LIFO arena that commits on touch and decommits on unwind", 4, "lazy-stack-arena")

	constexpr size_t BLOCK_SIZE = 2_MiB;

	// Contiguous block runs over a LazyStackArena, freed in any order. Freed holes are reused first;
	// when the top run frees, the arena unwinds past every free run below it.
	class SPEC_MEM_RUNTIME_API BlockedAllocator final : public IAllocator<void, LazyStackArena, BlockedAllocator> {
		static constexpr uint32_t MAX_BLOCKS = 16384;

		uint64_t m_Free[MAX_BLOCKS / 64]{};
		Corium::Runtime::Sync::CriticalSection m_Lock;

	public:
		BlockedAllocator() = default;

		BlockedAllocator(const BlockedAllocator&) = delete;
		BlockedAllocator& operator=(const BlockedAllocator&) = delete;

		BlockedAllocator(BlockedAllocator&&) = delete;
		BlockedAllocator& operator=(BlockedAllocator&&) = delete;

		// Sizes round up to whole blocks. Alignment beyond a block is refused.
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl(void* p_Ptr, size_t v_Bytes);

		SPEC_MEM_NODISCARD bool owns(const void* p_Ptr) const;
		// Placeholder regions hand out address space only: no commit on allocate, no decommit on free.
		void setPlaceholder(bool v_Placeholder);
		SPEC_MEM_NODISCARD bool isPlaceholder() const;

		// A MemoryHandle over a run, ready for an arena.
		SPEC_MEM_NODISCARD static MemoryHandle handleFor(void* p_Ptr, size_t v_Bytes, size_t v_NumaNode);
	};

	SPEC_ALLOCATOR_METADATA(void, LazyStackArena, "Block runs freed in any order", 0, "blocked-allocator", AllocShape::Raw)
}
