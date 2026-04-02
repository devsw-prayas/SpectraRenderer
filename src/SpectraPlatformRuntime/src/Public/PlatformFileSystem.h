#pragma once

#include "SpectraPlatformRuntime.h"
#include "FileUtils.h"

namespace Spectra::Platform::Runtime::File {
	void SPECTRA_RUNTIME_API initializeAsyncHandler(AsyncFileHandler& ro_Handler) noexcept;
	void SPECTRA_RUNTIME_API destroyAsyncHandler(AsyncFileHandler& ro_Handler) noexcept;

	bool SPECTRA_RUNTIME_API isMappingAligned(uint64_t v_Offset) noexcept;
	void SPECTRA_RUNTIME_API validateMappingRange(uint64_t v_Offset, size_t v_Size) noexcept;

	class SPECTRA_RUNTIME_API Files final {
	public:
		static void init();
		static const PlatformFileSystemInfo& getInfo();

		static void deleteFile(const char* p_Path);
		static void renameFile(const char* p_OldPath, const char* p_NewPath);

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