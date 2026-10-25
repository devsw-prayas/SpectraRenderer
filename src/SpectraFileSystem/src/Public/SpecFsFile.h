#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"
#include "SpecFsPath.h"
#include "SpecFsTree.h"
#include "SpecFsMount.h"
#include "SpecFsSource.h"

namespace Spectra::FileSystem {
	enum class Access : uint8_t {
		READ,
		WRITE,
		READ_WRITE
	};

	enum class Disposition : uint8_t {
		OPEN_EXISTING,
		OPEN_ALWAYS,
		CREATE_NEW,
		CREATE_ALWAYS,
		TRUNCATE_EXISTING
	};

	enum class IoMode : uint8_t {
		BUFFERED,
		DIRECT,        // buffer, size and offset must be sector-aligned
		WRITE_THROUGH
	};

	// Default: readers let others read; anything that writes is exclusive.
	enum class SharePolicy : uint8_t {
		DEFAULT = 0,
		EXCLUSIVE = 1 << 0,
		ALLOW_WRITE = 1 << 1,
		ALLOW_DELETE = 1 << 2
	};

	constexpr SharePolicy operator|(SharePolicy v_A, SharePolicy v_B) noexcept { return SharePolicy(uint8_t(v_A) | uint8_t(v_B)); }

	struct OpenDesc {
		Access m_Access = Access::READ;
		Disposition m_Disposition = Disposition::OPEN_EXISTING;
		IoMode m_Mode = IoMode::BUFFERED;
		SharePolicy m_Share = SharePolicy::DEFAULT;
	};

	// One cache line. Move-only; closing unpins the node and releases the mount. No implicit flush.
	struct alignas(64) SPEC_FS_RUNTIME_API File {
		void* m_Native = nullptr;
		MountHandle m_Mount{};
		NodeHandle m_Node{};
		uint64_t m_SizeAtOpen = 0;
		uint32_t m_SectorSize = 0;
		Access m_Access = Access::READ;
		IoMode m_Mode = IoMode::BUFFERED;
		SharePolicy m_Share = SharePolicy::DEFAULT;
		uint8_t m_Reserved[25]{};

		File() = default;
		~File();

		File(const File&) = delete;
		File& operator=(const File&) = delete;

		File(File&& u_Other) noexcept;
		File& operator=(File&& u_Other) noexcept;

		SPEC_FS_NODISCARD bool isOpen() const noexcept;
	};
	static_assert(sizeof(File) == 64);

	struct FileStat {
		uint64_t m_Size;
		uint64_t m_CreationTime;  // FILETIME ticks
		uint64_t m_WriteTime;
		bool m_IsDirectory;
		bool m_IsReadOnly;
		bool m_IsLink;
	};

	// Bytes acquired from a SourceHandle; give them back with Sources::release.
	struct Blob {
		std::byte* m_Data = nullptr;
		size_t m_Size = 0;
	};

	// Positional, FileChannel-style. Failures return false and push an exec (module 0x07).
	class SPEC_FS_RUNTIME_API Files final {
	public:
		Files() = delete;

		// Mounted or native path. Write access on a read-only mount is refused.
		SPEC_FS_NODISCARD static bool open(const PathBuffer& r_Path, const OpenDesc& r_Desc, File& r_Out) noexcept;
		// For callers holding a mount whose name the path doesn't carry ("project:/").
		SPEC_FS_NODISCARD static bool open(MountHandle v_Mount, const PathBuffer& r_Path, const OpenDesc& r_Desc, File& r_Out) noexcept;
		static void close(File& r_File) noexcept;

		SPEC_FS_NODISCARD static bool read(const File& r_File, std::span<std::byte> v_Buffer, uint64_t v_Offset, size_t& r_Read) noexcept;
		SPEC_FS_NODISCARD static bool write(const File& r_File, std::span<const std::byte> v_Bytes, uint64_t v_Offset, size_t& r_Written) noexcept;
		SPEC_FS_NODISCARD static bool flush(const File& r_File) noexcept;
		SPEC_FS_NODISCARD static bool size(const File& r_File, uint64_t& r_Size) noexcept;
		SPEC_FS_NODISCARD static bool setSize(const File& r_File, uint64_t v_Size) noexcept;

		SPEC_FS_NODISCARD static bool stat(const PathBuffer& r_Path, FileStat& r_Out) noexcept;
		SPEC_FS_NODISCARD static bool exists(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static bool isFile(const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static bool isDirectory(const PathBuffer& r_Path) noexcept;

		SPEC_FS_NODISCARD static bool readAll(const PathBuffer& r_Path, const SourceHandle& r_Source, Blob& r_Out) noexcept;
		// Strips a UTF-8 BOM and NUL-terminates; m_Size excludes the NUL.
		SPEC_FS_NODISCARD static bool readText(const PathBuffer& r_Path, const SourceHandle& r_Source, Blob& r_Out) noexcept;
		// v_Atomic: write a sibling temp file, flush, rename over the target, so readers see old or new, never a mix.
		SPEC_FS_NODISCARD static bool writeAll(const PathBuffer& r_Path, std::span<const std::byte> v_Bytes, bool v_Atomic) noexcept;
		SPEC_FS_NODISCARD static bool writeText(const PathBuffer& r_Path, std::string_view v_Text, bool v_Atomic) noexcept;
		SPEC_FS_NODISCARD static bool appendAll(const PathBuffer& r_Path, std::span<const std::byte> v_Bytes) noexcept;

		SPEC_FS_NODISCARD static bool copy(const PathBuffer& r_From, const PathBuffer& r_To, bool v_Overwrite) noexcept;
		// Bytes only; Index/ upkeep belongs to the asset layer. Follows the node when both ends share a mount.
		SPEC_FS_NODISCARD static bool move(const PathBuffer& r_From, const PathBuffer& r_To) noexcept;
		SPEC_FS_NODISCARD static bool remove(const PathBuffer& r_Path) noexcept;
	};

	class SPEC_FS_RUNTIME_API Dirs final {
	public:
		Dirs() = delete;

		// Creates missing parents too.
		SPEC_FS_NODISCARD static bool create(const PathBuffer& r_Path) noexcept;
		// Empty directories only.
		SPEC_FS_NODISCARD static bool remove(const PathBuffer& r_Path) noexcept;
		// Everything under it; links are unlinked, never followed.
		SPEC_FS_NODISCARD static bool removeAll(const PathBuffer& r_Path) noexcept;
	};

	// Sequential cursors over a File. Not shared between threads.
	struct SPEC_FS_RUNTIME_API FileReader {
		const File* m_File = nullptr;
		uint64_t m_Position = 0;

		SPEC_FS_NODISCARD bool read(std::span<std::byte> v_Buffer, size_t& r_Read) noexcept;
	};

	struct SPEC_FS_RUNTIME_API FileWriter {
		const File* m_File = nullptr;
		uint64_t m_Position = 0;

		SPEC_FS_NODISCARD bool write(std::span<const std::byte> v_Bytes) noexcept;
	};
}
