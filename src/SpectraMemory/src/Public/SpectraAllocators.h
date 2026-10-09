#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"
#include "SpecMemDiagnostics.h"
#include "MemBehavior.h"
#include "SpecMemAllocator.h"
#include "MemoryRegion.h"

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

		LinearArena(const LinearArena&) = default;
		LinearArena& operator=(const LinearArena&) = default;

		LinearArena(LinearArena&&) noexcept = default;
		LinearArena& operator=(LinearArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();
	};

	SPEC_ARENA_METADATA(LinearArena, "Linear persistent arena", 0 /*TODO*/, "linear-arena")

	// TODO: inject SpectraInstrumentation/Profiler hooks (allocate/deallocate) later.
	class SPEC_MEM_RUNTIME_API StackArena : public IArena<StackArena> {
		MemoryHandle m_Handle;
		// Same split as LinearArena - see its m_Cursor comment.
		size_t m_Cursor = 0;
	public:
		using Marker = size_t;

		StackArena() = default;
		explicit StackArena(const MemoryHandle& ro_Handle) : m_Handle(ro_Handle) {}

		StackArena(const StackArena&) = default;
		StackArena& operator=(const StackArena&) = default;

		StackArena(StackArena&&) noexcept = default;
		StackArena& operator=(StackArena&&) noexcept = default;

		void init(MemoryHandle* p_Handle);

		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes, size_t v_Alignment);
		SPEC_MEM_NODISCARD void* allocateImpl(size_t v_Bytes);
		void deallocateImpl();

		SPEC_MEM_NODISCARD Marker mark() const;
		void unwind(Marker v_Marker);
	};

	SPEC_ARENA_METADATA(StackArena, "LIFO bump arena with mark/unwind rewind points", 1 /*TODO*/, "stack-arena")

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

	SPEC_ARENA_METADATA(ReclaimableLinearArena, "Linear arena that decommits on reset", 2 /*TODO*/, "reclaimable-linear-arena")

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

	SPEC_ARENA_METADATA(ReclaimableStackArena, "LIFO bump arena that decommits on unwind", 3 /*TODO*/, "reclaimable-stack-arena")

	// Tag-distinguished raw allocator over LinearArena.
	template<typename Tag>
	struct RawArenaAllocator final : IAllocator<void, LinearArena, RawArenaAllocator<Tag>> {};

	// Backing allocator for ClosureFunction/FunctionView payloads.
	using ClosureAllocator = RawArenaAllocator<struct ClosureTag>;

}
