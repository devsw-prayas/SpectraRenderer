#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"

namespace Spectra::FileSystem {
	enum class PathKind : uint8_t {
		EMPTY,
		MOUNTED,         // "engine:/Settings/x.ini"
		RELATIVE,        // "Assets/wood.png"
		NATIVE_ABSOLUTE  // "C:/Imports/scene.xml", import sources only
	};

	constexpr uint32_t PATH_CAPACITY = 507;
	constexpr uint32_t NATIVE_PATH_CAPACITY = 520;

	// Transient: parse, build, hand to the OS. Nodes are what get stored.
	// Always normalised: UTF-8, '/' separators, no '.', no '//', no trailing '/', '..' resolved.
	struct alignas(64) PathBuffer {
		char     m_Chars[PATH_CAPACITY + 1];
		uint16_t m_Length;
		PathKind m_Kind;
		uint8_t  m_MountEnd;  // one past "name:/", 0 if not mounted
	};
	static_assert(sizeof(PathBuffer) == 512);
	static_assert(std::is_trivially_copyable_v<PathBuffer>);

	// Failures return false and push an exec (module 0x07); make leaves the buffer EMPTY, the rest leave it unchanged.
	// Mount names are only text here; layer 3 resolves them, "project" included.
	class SPEC_FS_RUNTIME_API Paths final {
	public:
		Paths() = delete;

		SPEC_FS_NODISCARD static bool make(PathBuffer& r_Out, std::string_view v_Text) noexcept;
		SPEC_FS_NODISCARD static bool makeFromNative(PathBuffer& r_Out, std::wstring_view v_Native) noexcept;

		SPEC_FS_NODISCARD static bool join(PathBuffer& r_Path, std::string_view v_Relative) noexcept;
		SPEC_FS_NODISCARD static bool replaceExtension(PathBuffer& r_Path, std::string_view v_Extension) noexcept;
		static void toParent(PathBuffer& r_Path) noexcept;

		// Views stay valid while the buffer is alive and unmodified.
		SPEC_FS_NODISCARD static std::string_view view(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static std::string_view mountName(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static std::string_view relativePart(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static std::string_view fileName(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static std::string_view stem(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static std::string_view extension(const PathBuffer& r_Path) noexcept;  // no dot

		// Start with r_Offset = 0, call until false.
		SPEC_FS_NODISCARD static bool nextSegment(const PathBuffer& r_Path, uint16_t& r_Offset, std::string_view& r_Segment) noexcept;

		SPEC_FS_NODISCARD static bool equals(const PathBuffer& r_A, const PathBuffer& r_B) noexcept;
		SPEC_FS_NODISCARD static bool equalsIgnoreCase(const PathBuffer& r_A, const PathBuffer& r_B) noexcept;  // ASCII fold

		// UTF-16, '\' separators, "\?\" on absolute paths. Mounted paths must be resolved first.
		SPEC_FS_NODISCARD static bool toNative(const PathBuffer& r_Path, wchar_t (&r_Out)[NATIVE_PATH_CAPACITY], uint32_t& r_Length) noexcept;
	};
}
