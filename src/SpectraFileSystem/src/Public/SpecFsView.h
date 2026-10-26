#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"
#include "SpecFsFile.h"

namespace Spectra::FileSystem {
	constexpr size_t VIEW_GRANULARITY = 64 * 1024;
	constexpr size_t PREFETCH_CAP = 5 * 2 * 1024 * 1024;  // five large pages

	// One cache line. Lives in a block run of the mapped-view region; outlives the File it came from
	// (own section, own pin). Move-only; destroying it unmaps. No implicit flush.
	struct alignas(64) SPEC_FS_RUNTIME_API MappedView {
		std::byte* m_Data = nullptr;  // the requested byte, not the view base
		void* m_Base = nullptr;
		void* m_Section = nullptr;
		size_t m_Size = 0;            // requested bytes
		size_t m_ViewBytes = 0;       // mapped bytes from m_Base
		MountHandle m_Mount{};
		NodeHandle m_Node{};
		uint32_t m_RunBlocks = 0;
		uint8_t m_NumaNode = 0;
		bool m_Writable = false;

		MappedView() = default;
		~MappedView();

		MappedView(const MappedView&) = delete;
		MappedView& operator=(const MappedView&) = delete;

		MappedView(MappedView&& u_Other) noexcept;
		MappedView& operator=(MappedView&& u_Other) noexcept;

		SPEC_FS_NODISCARD bool isMapped() const noexcept;
	};
	SPEC_FS_STATIC_ASSERT(sizeof(MappedView) == 64, "MappedView must be 64 bytes");

	// A fixed-size view that moves along a file. Re-maps only when a request leaves the current window.
	struct SPEC_FS_RUNTIME_API MappedWindow {
		const File* m_File = nullptr;
		size_t m_WindowBytes = 0;
		uint64_t m_Start = 0;
		bool m_Writable = false;
		MappedView m_View;
	};

	// Failures return false and push an exec (module 0x07).
	class SPEC_FS_RUNTIME_API Views final {
	public:
		Views() = delete;

		// [v_Offset, v_Offset + v_Size) must already lie inside the file: set the final size before
		// mapping, so a full disk shows up here and not as an access fault later. Writable needs a writable File.
		SPEC_FS_NODISCARD static bool map(const File& r_File, uint64_t v_Offset, size_t v_Size, bool v_Writable, MappedView& r_Out, uint32_t v_NumaNode = 0) noexcept;
		static void unmap(MappedView& r_View) noexcept;

		// Writes dirty pages back; follow with Files::flush for durability.
		SPEC_FS_NODISCARD static bool flush(const MappedView& r_View) noexcept;
		// Clamped to the view and to PREFETCH_CAP.
		SPEC_FS_NODISCARD static bool prefetch(const MappedView& r_View, size_t v_Offset, size_t v_Bytes) noexcept;

		SPEC_FS_NODISCARD static bool openWindow(const File& r_File, size_t v_WindowBytes, bool v_Writable, MappedWindow& r_Out) noexcept;
		// Pointer to file byte v_Offset, valid for v_Bytes (at most the window size) until the next slide.
		SPEC_FS_NODISCARD static std::byte* slide(MappedWindow& r_Window, uint64_t v_Offset, size_t v_Bytes) noexcept;
		static void closeWindow(MappedWindow& r_Window) noexcept;
	};
}
