#include "SpecFsView.h"
#include "SpecFsDiagnostic.h"

#include <PlatformFileSystem.h>
#include <PlatformMemory.h>
#include <SpecMemRegistry.h>

namespace Spectra::FileSystem {
	namespace {
		using Platform::Runtime::Memory::PlatformPlaceholders;
		using NativeFiles = Platform::Runtime::File::Files;
		using Memory::Allocators::BlockedAllocator;
		using Memory::Allocators::BLOCK_SIZE;

		constexpr size_t PAGE = 4096;

		BlockedAllocator* viewBlocks(uint32_t v_Node) noexcept {
			return Memory::AtomicAllocators::instance().viewBlocks(v_Node);
		}

		// Free view blocks are one placeholder each; a run is merged into one placeholder before mapping.
		bool shapeRun(std::byte* p_Run, uint32_t v_Blocks, size_t v_ViewBytes) noexcept {
			const size_t runBytes = size_t(v_Blocks) * BLOCK_SIZE;
			if (v_Blocks > 1 && !PlatformPlaceholders::coalesce(p_Run, runBytes)) return false;
			return v_ViewBytes == runBytes || PlatformPlaceholders::split(p_Run, v_ViewBytes);
		}

		// Back to one placeholder per block once the view is gone.
		void restoreRun(std::byte* p_Run, uint32_t v_Blocks, size_t v_ViewBytes) noexcept {
			const size_t runBytes = size_t(v_Blocks) * BLOCK_SIZE;
			if (v_ViewBytes != runBytes) (void)PlatformPlaceholders::coalesce(p_Run, runBytes);
			for (uint32_t block = 0; block + 1 < v_Blocks; ++block)
				(void)PlatformPlaceholders::split(p_Run + size_t(block) * BLOCK_SIZE, BLOCK_SIZE);
		}

		void moveView(MappedView& r_To, MappedView& r_From) noexcept {
			std::memcpy(static_cast<void*>(&r_To), &r_From, sizeof(MappedView));
			r_From.m_Base = nullptr;
			r_From.m_Data = nullptr;
			r_From.m_Section = nullptr;
			r_From.m_Mount = {};
			r_From.m_Node = {};
		}
	}

	MappedView::~MappedView() {
		Views::unmap(*this);
	}

	MappedView::MappedView(MappedView&& u_Other) noexcept {
		moveView(*this, u_Other);
	}

	MappedView& MappedView::operator=(MappedView&& u_Other) noexcept {
		if (this == &u_Other) return *this;
		Views::unmap(*this);
		moveView(*this, u_Other);
		return *this;
	}

	bool MappedView::isMapped() const noexcept {
		return m_Base != nullptr;
	}

	bool Views::map(const File& r_File, uint64_t v_Offset, size_t v_Size, bool v_Writable, MappedView& r_Out, uint32_t v_NumaNode) noexcept {
		unmap(r_Out);
		BlockedAllocator* blocks = viewBlocks(v_NumaNode);
		if (blocks == nullptr) return false;  // exec Unsupported: no placeholder support
		if (!r_File.isOpen() || v_Size == 0 || (v_Writable && r_File.m_Access == Access::READ)) return false;  // exec InvalidHandle
		uint64_t fileSize = 0;
		if (!Files::size(r_File, fileSize) || v_Offset + v_Size > fileSize) return false;  // exec OutsideFile: size the file first

		const uint64_t aligned = v_Offset & ~uint64_t(VIEW_GRANULARITY - 1);
		const size_t delta = size_t(v_Offset - aligned);
		const size_t viewBytes = (delta + v_Size + PAGE - 1) & ~(PAGE - 1);
		std::byte* run = static_cast<std::byte*>(blocks->allocate(viewBytes));
		if (run == nullptr) return false;  // exec OutOfViewSpace
		const uint32_t runBlocks = uint32_t((viewBytes + BLOCK_SIZE - 1) / BLOCK_SIZE);

		void* section = NativeFiles::createSection(r_File.m_Native, v_Writable, 0);
		bool ok = section != nullptr && shapeRun(run, runBlocks, viewBytes);
		// Exact length: a page-rounded one would run past the end of a file whose size isn't a page multiple.
		ok = ok && PlatformPlaceholders::mapView(section, aligned, run, delta + v_Size, v_Writable, v_NumaNode);  // exec MapFailed
		if (ok && Mounts::isValid(r_File.m_Mount)) {
			PathTree* tree = Mounts::tree(r_File.m_Mount);
			ok = Mounts::acquire(r_File.m_Mount);
			if (ok && tree != nullptr && r_File.m_Node.m_Index != NODE_NONE && !Trees::pin(*tree, r_File.m_Node)) {
				Mounts::release(r_File.m_Mount);
				ok = false;
			}
			if (!ok) (void)PlatformPlaceholders::unmapView(run);
		}
		if (!ok) {
			if (section != nullptr) (void)NativeFiles::closeSection(section);
			restoreRun(run, runBlocks, viewBytes);
			blocks->deallocate(run, size_t(runBlocks) * BLOCK_SIZE);
			return false;
		}

		r_Out.m_Base = run;
		r_Out.m_Data = run + delta;
		r_Out.m_Section = section;
		r_Out.m_Size = v_Size;
		r_Out.m_ViewBytes = viewBytes;
		r_Out.m_Mount = r_File.m_Mount;
		r_Out.m_Node = r_File.m_Node;
		r_Out.m_RunBlocks = runBlocks;
		r_Out.m_NumaNode = uint8_t(v_NumaNode);
		r_Out.m_Writable = v_Writable;
		return true;
	}

	void Views::unmap(MappedView& r_View) noexcept {
		if (r_View.m_Base == nullptr) return;
		std::byte* run = static_cast<std::byte*>(r_View.m_Base);
		const bool unmapped = PlatformPlaceholders::unmapView(run);
		SPEC_FS_ASSERT(unmapped);  // err: a failed unmap leaves the region unusable
		(void)NativeFiles::closeSection(r_View.m_Section);
		restoreRun(run, r_View.m_RunBlocks, r_View.m_ViewBytes);
		viewBlocks(r_View.m_NumaNode)->deallocate(run, size_t(r_View.m_RunBlocks) * BLOCK_SIZE);

		if (Mounts::isValid(r_View.m_Mount)) {
			if (r_View.m_Node.m_Index != NODE_NONE)
				if (PathTree* tree = Mounts::tree(r_View.m_Mount)) Trees::unpin(*tree, r_View.m_Node);
			Mounts::release(r_View.m_Mount);
		}
		r_View.m_Base = nullptr;
		r_View.m_Data = nullptr;
		r_View.m_Section = nullptr;
		r_View.m_Mount = {};
		r_View.m_Node = {};
	}

	bool Views::flush(const MappedView& r_View) noexcept {
		return r_View.isMapped() && PlatformPlaceholders::flushView(r_View.m_Base, r_View.m_ViewBytes);
	}

	bool Views::prefetch(const MappedView& r_View, size_t v_Offset, size_t v_Bytes) noexcept {
		if (!r_View.isMapped() || v_Offset >= r_View.m_Size) return false;
		size_t bytes = v_Bytes < PREFETCH_CAP ? v_Bytes : PREFETCH_CAP;
		if (bytes > r_View.m_Size - v_Offset) bytes = r_View.m_Size - v_Offset;
		return PlatformPlaceholders::prefetch(r_View.m_Data + v_Offset, bytes);
	}

	bool Views::openWindow(const File& r_File, size_t v_WindowBytes, bool v_Writable, MappedWindow& r_Out) noexcept {
		if (!r_File.isOpen() || v_WindowBytes < VIEW_GRANULARITY) return false;  // exec InvalidSize
		closeWindow(r_Out);
		r_Out.m_File = &r_File;
		r_Out.m_WindowBytes = v_WindowBytes;
		r_Out.m_Writable = v_Writable;
		r_Out.m_Start = 0;
		return true;
	}

	std::byte* Views::slide(MappedWindow& r_Window, uint64_t v_Offset, size_t v_Bytes) noexcept {
		if (r_Window.m_File == nullptr || v_Bytes == 0 || v_Bytes > r_Window.m_WindowBytes - VIEW_GRANULARITY) return nullptr;  // exec InvalidSize
		MappedView& view = r_Window.m_View;
		if (view.isMapped() && v_Offset >= r_Window.m_Start && v_Offset + v_Bytes <= r_Window.m_Start + view.m_Size)
			return view.m_Data + (v_Offset - r_Window.m_Start);

		uint64_t fileSize = 0;
		if (!Files::size(*r_Window.m_File, fileSize) || v_Offset + v_Bytes > fileSize) return nullptr;  // exec OutsideFile
		const uint64_t start = v_Offset & ~uint64_t(VIEW_GRANULARITY - 1);
		const uint64_t span = fileSize - start < r_Window.m_WindowBytes ? fileSize - start : r_Window.m_WindowBytes;
		if (!map(*r_Window.m_File, start, size_t(span), r_Window.m_Writable, view)) return nullptr;
		r_Window.m_Start = start;
		return view.m_Data + (v_Offset - start);
	}

	void Views::closeWindow(MappedWindow& r_Window) noexcept {
		unmap(r_Window.m_View);
		r_Window.m_File = nullptr;
	}
}
