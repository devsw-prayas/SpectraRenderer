#pragma once
#include "SpectraInternalDiagonostics.h"
#include "SpectraPlatformRuntime.h"

namespace Spectra::Platform::Runtime::File {
	enum class FileIOMode final : uint8_t {
		SEQUENTIAL, ASYNCHRONOUS
	};

	enum class FileAccess final : uint8_t {
		READ, WRITE, READ_WRITE
	};

	enum class FileOpenMode final : uint8_t {
		CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
	};

	enum class FileShareMode final : uint8_t {
		NONE, READ, WRITE, DELETE
	};

	enum class FileSeekOrigin final : uint8_t {
		BEGIN, CURRENT, END
	};

	struct alignas(16) SPECTRA_RUNTIME_API PlatformFileSystemInfo final {
		size_t m_MaxPathLength;
		size_t m_AllocationGranularity;

		PlatformFileSystemInfo() = default;
		~PlatformFileSystemInfo() = default;

		PlatformFileSystemInfo(const PlatformFileSystemInfo&) = default;
		PlatformFileSystemInfo& operator=(const PlatformFileSystemInfo&) = default;

		PlatformFileSystemInfo(PlatformFileSystemInfo&&) noexcept = default;
		PlatformFileSystemInfo& operator=(PlatformFileSystemInfo&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API  FileHandle final {
		void* m_NativeHandle;
		FileIOMode m_Mode;

		FileHandle() = default;
		~FileHandle() = default;

		FileHandle(const FileHandle&) = default;
		FileHandle& operator=(const FileHandle&) = default;

		FileHandle(FileHandle&&) noexcept = default;
		FileHandle& operator=(FileHandle&&) noexcept = default;
	};

	struct alignas(32) SPECTRA_RUNTIME_API AsyncFileHandler final {
		void* m_EventHandle;
		uint64_t m_Offset;
		size_t m_BytesTransferred;

		AsyncFileHandler() = default;
		~AsyncFileHandler() = default;

		AsyncFileHandler(const AsyncFileHandler&) = default;
		AsyncFileHandler& operator=(const AsyncFileHandler&) = default;

		AsyncFileHandler(AsyncFileHandler&&) noexcept = default;
		AsyncFileHandler& operator=(AsyncFileHandler&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API DirectoryEnumHandle final {
		void* m_NativeHandle;
		bool m_IsValid;

		DirectoryEnumHandle() = default;
		~DirectoryEnumHandle() = default;

		DirectoryEnumHandle(const DirectoryEnumHandle&) = default;
		DirectoryEnumHandle& operator=(const DirectoryEnumHandle&) = default;

		DirectoryEnumHandle(DirectoryEnumHandle&&) noexcept = default;
		DirectoryEnumHandle& operator=(DirectoryEnumHandle&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API FileMappingHandle final {
		void* m_NativeHandle;
		size_t m_MappingSize;

		FileMappingHandle() = default;
		~FileMappingHandle() = default;

		FileMappingHandle(const FileMappingHandle&) = default;
		FileMappingHandle& operator=(const FileMappingHandle&) = default;

		FileMappingHandle(FileMappingHandle&&) noexcept = default;
		FileMappingHandle& operator=(FileMappingHandle&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API FileInfo final {
		char      m_FileName[260];
		bool      m_IsDirectory;
		bool      m_IsReadOnly;
	private:
		SPECTRA_MAYBE_UNUSED short pad_ = 0;
	public:
		size_t    m_FileSize;
		uint64_t  m_LastWriteTime;
	};
}
