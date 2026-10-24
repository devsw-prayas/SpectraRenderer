#pragma once

#include "SpectraPlatformRuntime.h"
#include "FileUtils.h"

namespace Spectra::Platform::Runtime::File {
	void SPECTRA_RUNTIME_API initializeAsyncHandler(AsyncFileHandler& ro_Handler) noexcept;
	void SPECTRA_RUNTIME_API destroyAsyncHandler(AsyncFileHandler& ro_Handler) noexcept;

	bool SPECTRA_RUNTIME_API isMappingAligned(uint64_t v_Offset) noexcept;
	void SPECTRA_RUNTIME_API validateMappingRange(uint64_t v_Offset, size_t v_Size) noexcept;

	enum class NativeBuffering : uint8_t {
		BUFFERED,
		DIRECT,        // FILE_FLAG_NO_BUFFERING: sector-aligned buffers, sizes and offsets
		WRITE_THROUGH
	};

	enum class NativeShare : uint8_t {
		NONE = 0,
		READ = 1 << 0,
		WRITE = 1 << 1,
		REMOVE = 1 << 2
	};

	constexpr NativeShare operator|(NativeShare v_A, NativeShare v_B) noexcept { return NativeShare(uint8_t(v_A) | uint8_t(v_B)); }

	enum class NativeError : uint8_t {
		NONE,
		NOT_FOUND,
		ALREADY_EXISTS,
		ACCESS_DENIED,
		SHARING_VIOLATION,
		DISK_FULL,
		OTHER
	};

	struct NativeOpenDesc {
		FileAccess m_Access;
		FileOpenMode m_OpenMode;
		NativeShare m_Share;
		NativeBuffering m_Buffering;
	};

	struct NativeStat {
		uint64_t m_Size;
		uint64_t m_CreationTime;   // FILETIME ticks
		uint64_t m_WriteTime;
		bool m_IsDirectory;
		bool m_IsReadOnly;
		bool m_IsLink;
	};

	enum class KnownFolder : uint8_t {
		LOCAL_APP_DATA,
		TEMP,
		EXECUTABLE_DIR
	};

	enum class PathAttribute : uint8_t {
		MISSING,
		FILE,
		DIRECTORY
	};

	class SPECTRA_RUNTIME_API Files final {
	public:
		static void init();
		static const PlatformFileSystemInfo& getInfo();

		static void deleteFile(const char* p_Path);
		static void renameFile(const char* p_OldPath, const char* p_NewPath);

		// -- Wide paths: already native ("\\?\" prefixed), failures returned instead of terminating --

		SPECTRA_NODISCARD static bool deleteFile(const wchar_t* p_Path);
		SPECTRA_NODISCARD static bool renameFile(const wchar_t* p_OldPath, const wchar_t* p_NewPath);
		// True if the directory exists afterwards; parents must exist.
		SPECTRA_NODISCARD static bool createDirectory(const wchar_t* p_Path);
		SPECTRA_NODISCARD static bool removeDirectory(const wchar_t* p_Path);
		// Removes the directory and everything under it. Stops at the first entry it can't remove.
		SPECTRA_NODISCARD static bool removeTree(const wchar_t* p_Path);
		SPECTRA_NODISCARD static PathAttribute getAttribute(const wchar_t* p_Path);
		// Calls p_Visit for each entry except "." and ".."; returning false stops the walk.
		using ChildVisitFn = bool(*)(const wchar_t* p_Name, bool v_IsDirectory, void* p_Context);
		SPECTRA_NODISCARD static bool forEachChild(const wchar_t* p_Directory, ChildVisitFn p_Visit, void* p_Context);
		// Plain absolute path, '\' separators, no trailing separator, NUL-terminated.
		SPECTRA_NODISCARD static bool getKnownFolder(KnownFolder v_Folder, wchar_t* p_Out, uint32_t v_Capacity, uint32_t& r_Length);

		// Handle-level I/O on native paths. Nullptr / false on failure; lastError() says why.
		SPECTRA_NODISCARD static void* openNative(const wchar_t* p_Path, const NativeOpenDesc& ro_Desc);
		SPECTRA_NODISCARD static bool closeNative(void* p_Handle);
		SPECTRA_NODISCARD static bool readAt(void* p_Handle, void* p_Buffer, size_t v_Bytes, uint64_t v_Offset, size_t& r_Read);
		SPECTRA_NODISCARD static bool writeAt(void* p_Handle, const void* p_Buffer, size_t v_Bytes, uint64_t v_Offset, size_t& r_Written);
		SPECTRA_NODISCARD static bool flushNative(void* p_Handle);
		SPECTRA_NODISCARD static bool getSize(void* p_Handle, uint64_t& r_Size);
		SPECTRA_NODISCARD static bool setSize(void* p_Handle, uint64_t v_Size);
		// Physical sector size for performance; 4096 when the volume doesn't say.
		SPECTRA_NODISCARD static uint32_t sectorSize(void* p_Handle);
		SPECTRA_NODISCARD static bool stat(const wchar_t* p_Path, NativeStat& r_Stat);
		SPECTRA_NODISCARD static bool copyFile(const wchar_t* p_From, const wchar_t* p_To, bool v_Overwrite);
		SPECTRA_NODISCARD static NativeError lastError();
		// Section over an open file for placeholder views. v_MaxSize 0 means the file's current size.
		SPECTRA_NODISCARD static void* createSection(void* p_File, bool v_Writable, uint64_t v_MaxSize);
		SPECTRA_NODISCARD static bool closeSection(void* p_Section);

		static FileHandle open(const FileStreamDesc& ro_Desc);
		static void close(FileHandle& ro_Handle);

		static size_t read(const FileHandle& ro_Handle, void* p_Buffer, size_t v_BytesToRead);
		static size_t write(const FileHandle& ro_Handle, const void* p_Buffer, size_t v_BytesToWrite);

		static void seek(const FileHandle& ro_Handle, int64_t v_Offset, FileSeekOrigin v_Origin);
		static void flush(FileHandle& ro_Handle);

		static size_t getFileSize(FileHandle& ro_Handle);

		static void readAsync(
			FileHandle& ro_Handle, AsyncFileHandler& ro_Handler,
			void* p_Buffer, size_t v_BytesToRead, uint64_t v_Offset);

		static void writeAsync(
			FileHandle& ro_Handle, AsyncFileHandler& ro_Handler,
			const void* p_Buffer, size_t v_BytesToWrite, uint64_t v_Offset);

		static size_t getOverlappedResult(
			FileHandle& ro_Handle, AsyncFileHandler& ro_Handler, bool v_Wait);

		// -- Directory Enumeration --

		static DirectoryEnumHandle beginEnumeration(const DirectoryEnumDesc& ro_Desc);
		static bool next(DirectoryEnumHandle& ro_Handle, FileInfo& ro_OutInfo);
		static void closeEnumeration(DirectoryEnumHandle& ro_Handle);

		// -- Memory Mapping --

		static FileMappingHandle createMapping(
			FileHandle& ro_Handle, FileAccess v_Access, size_t v_MaxSize);

		static void* mapView(
			FileMappingHandle& ro_Mapping, uint64_t v_Offset, size_t v_Size, FileAccess v_Access);

		static void unmapView(const void* p_BaseAddress);
		static void closeMapping(FileMappingHandle& ro_Mapping);
	};
}