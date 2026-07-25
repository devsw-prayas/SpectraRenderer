#include "SpectraPlatformRuntime.h"

#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"
#include "PlatformFileSystem.h"
#include "ProcessEnvironment.h"
#include "SpectraDiagnostics.h"

namespace Spectra::Platform::Runtime::File {
	namespace {
		PlatformFileSystemInfo g_FileSystemInfo{};
		bool g_IsFileSystemInitialized = false;

		// UTF-8 -> UTF-16 conversion for Win32 APIs.
		// Stack buffer fast path (MAX_PATH), heap fallback for longer paths.
		// Caller is responsible for freeing heap buffer if returned != stack_buf.
		wchar_t* toWide(const char* p_Utf8, wchar_t* p_StackBuf, int v_StackLen, int& ro_OutLen) {
			ro_OutLen = MultiByteToWideChar(CP_UTF8, 0, p_Utf8, -1, nullptr, 0);
			if (ro_OutLen <= 0)
				Environment::PlatformTermination::terminate();

			if (ro_OutLen <= v_StackLen) {
				MultiByteToWideChar(CP_UTF8, 0, p_Utf8, -1, p_StackBuf, ro_OutLen);
				return p_StackBuf;
			}

			wchar_t* heap = new wchar_t[ro_OutLen];
			MultiByteToWideChar(CP_UTF8, 0, p_Utf8, -1, heap, ro_OutLen);
			return heap;
		}

		void freeWide(const wchar_t* p_Buf, const wchar_t* p_StackBuf) {
			if (p_Buf != p_StackBuf)
				delete[] p_Buf;
		}

		DWORD toWin32Access(FileAccess v_Access) {
			switch (v_Access) {
			case FileAccess::READ:       return GENERIC_READ;
			case FileAccess::WRITE:      return GENERIC_WRITE;
			case FileAccess::READ_WRITE: return GENERIC_READ | GENERIC_WRITE;
			}
			SPECTRA_UNREACHABLE();
		}

		DWORD toWin32ShareMode(FileShareMode v_Share) {
			switch (v_Share) {
			case FileShareMode::NONE:   return 0;
			case FileShareMode::READ:   return FILE_SHARE_READ;
			case FileShareMode::WRITE:  return FILE_SHARE_WRITE;
			case FileShareMode::REMOVE: return FILE_SHARE_DELETE;
			}
			SPECTRA_UNREACHABLE();
		}

		DWORD toWin32CreationDisposition(FileOpenMode v_OpenMode) {
			switch (v_OpenMode) {
			case FileOpenMode::CREATE_NEW_FILE:        return CREATE_NEW;
			case FileOpenMode::CREATE_ALWAYS_FILE:     return CREATE_ALWAYS;
			case FileOpenMode::OPEN_EXISTING_FILE:     return OPEN_EXISTING;
			case FileOpenMode::OPEN_ALWAYS_FILE:       return OPEN_ALWAYS;
			case FileOpenMode::TRUNCATE_EXISTING_FILE: return TRUNCATE_EXISTING;
			}
			SPECTRA_UNREACHABLE();
		}

		DWORD toWin32MappingProtect(FileAccess v_Access) {
			switch (v_Access) {
			case FileAccess::READ:       return PAGE_READONLY;
			case FileAccess::WRITE:
			case FileAccess::READ_WRITE: return PAGE_READWRITE;
			}
			SPECTRA_UNREACHABLE();
		}

		DWORD toWin32MapViewAccess(FileAccess v_Access) {
			switch (v_Access) {
			case FileAccess::READ:       return FILE_MAP_READ;
			case FileAccess::WRITE:      return FILE_MAP_WRITE;
			case FileAccess::READ_WRITE: return FILE_MAP_ALL_ACCESS;
			}
			SPECTRA_UNREACHABLE();
		}

		DWORD toWin32SeekMethod(FileSeekOrigin v_Origin) {
			switch (v_Origin) {
			case FileSeekOrigin::BEGIN:   return FILE_BEGIN;
			case FileSeekOrigin::CURRENT: return FILE_CURRENT;
			case FileSeekOrigin::END:     return FILE_END;
			}
			SPECTRA_UNREACHABLE();
		}

		bool isValidHandle(const FileHandle& ro_Handle) {
			return ro_Handle.m_NativeHandle != nullptr
				&& ro_Handle.m_NativeHandle != INVALID_HANDLE_VALUE;
		}

		bool isValidMappingHandle(const FileMappingHandle& ro_Handle) {
			return ro_Handle.m_NativeHandle != nullptr
				&& ro_Handle.m_NativeHandle != INVALID_HANDLE_VALUE;
		}

		bool isValidEnumHandle(const DirectoryEnumHandle& ro_Handle) {
			return ro_Handle.m_IsValid
				&& ro_Handle.m_NativeHandle != nullptr
				&& ro_Handle.m_NativeHandle != INVALID_HANDLE_VALUE;
		}
	}

	void initializeAsyncHandler(AsyncFileHandler& ro_Handler) noexcept {
		HANDLE ev = CreateEventW(nullptr, TRUE, FALSE, nullptr); // manual-reset, initially unsignaled
		if (ev == nullptr || ev == INVALID_HANDLE_VALUE)
			Environment::PlatformTermination::terminate();

		ro_Handler.m_EventHandle = ev;
		ro_Handler.m_Offset = 0;
		ro_Handler.m_BytesTransferred = 0;
	}

	void destroyAsyncHandler(AsyncFileHandler& ro_Handler) noexcept {
		if (ro_Handler.m_EventHandle != nullptr && ro_Handler.m_EventHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(static_cast<HANDLE>(ro_Handler.m_EventHandle));
			ro_Handler.m_EventHandle = nullptr;
		}
	}

	bool isMappingAligned(uint64_t v_Offset) noexcept {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		return (v_Offset & (g_FileSystemInfo.m_AllocationGranularity - 1)) == 0;
	}

	void validateMappingRange(uint64_t v_Offset, size_t v_Size) noexcept {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		if (!isMappingAligned(v_Offset))
			Environment::PlatformTermination::terminate();
		if (v_Size == 0)
			Environment::PlatformTermination::terminate();
	}

	void Files::init() {
		if (g_IsFileSystemInitialized) return;

		SYSTEM_INFO info{};
		GetSystemInfo(&info);

		g_FileSystemInfo.m_AllocationGranularity = info.dwAllocationGranularity;
		g_FileSystemInfo.m_MaxPathLength = MAX_PATH;

		g_IsFileSystemInitialized = true;
	}

	const PlatformFileSystemInfo& Files::getInfo() {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		return g_FileSystemInfo;
	}

	void Files::deleteFile(const char* p_Path) {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		SPECTRA_ASSERT(p_Path != nullptr);

		wchar_t stackBuf[MAX_PATH];
		int len = 0;
		wchar_t* wide = toWide(p_Path, stackBuf, MAX_PATH, len);

		BOOL ok = DeleteFileW(wide);
		freeWide(wide, stackBuf);

		if (!ok)
			Environment::PlatformTermination::terminate();
	}

	void Files::renameFile(const char* p_OldPath, const char* p_NewPath) {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		SPECTRA_ASSERT(p_OldPath != nullptr && p_NewPath != nullptr);

		wchar_t oldStack[MAX_PATH], newStack[MAX_PATH];
		int oldLen = 0, newLen = 0;

		wchar_t* oldWide = toWide(p_OldPath, oldStack, MAX_PATH, oldLen);
		wchar_t* newWide = toWide(p_NewPath, newStack, MAX_PATH, newLen);

		BOOL ok = MoveFileExW(oldWide, newWide, MOVEFILE_REPLACE_EXISTING);

		freeWide(oldWide, oldStack);
		freeWide(newWide, newStack);

		if (!ok)
			Environment::PlatformTermination::terminate();
	}

	FileHandle Files::open(const FileStreamDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		SPECTRA_ASSERT(ro_Desc.m_Path != nullptr);

		wchar_t stackBuf[MAX_PATH];
		int len = 0;
		wchar_t* wide = toWide(ro_Desc.m_Path, stackBuf, MAX_PATH, len);

		DWORD access = toWin32Access(ro_Desc.m_Access);
		DWORD shareMode = toWin32ShareMode(ro_Desc.m_ShareMode);
		DWORD disposition = toWin32CreationDisposition(ro_Desc.m_OpenMode);
		DWORD flags = FILE_ATTRIBUTE_NORMAL;

		if (ro_Desc.m_Mode == FileIOMode::ASYNCHRONOUS)
			flags |= FILE_FLAG_OVERLAPPED;

		HANDLE h = CreateFileW(wide, access, shareMode, nullptr, disposition, flags, nullptr);
		freeWide(wide, stackBuf);

		if (h == INVALID_HANDLE_VALUE)
			Environment::PlatformTermination::terminate();

		FileHandle handle;
		handle.m_NativeHandle = h;
		handle.m_Mode = ro_Desc.m_Mode;
		return handle;
	}

	void Files::close(FileHandle& ro_Handle) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();

		CloseHandle(static_cast<HANDLE>(ro_Handle.m_NativeHandle));
		ro_Handle.m_NativeHandle = nullptr;
	}

	size_t Files::read(const FileHandle& ro_Handle, void* p_Buffer, size_t v_BytesToRead) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::SEQUENTIAL)
			Environment::PlatformTermination::terminate();
		SPECTRA_ASSERT(p_Buffer != nullptr);

		DWORD bytesRead = 0;
		BOOL ok = ReadFile(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
						   p_Buffer, static_cast<DWORD>(v_BytesToRead), &bytesRead, nullptr);

		if (!ok)
			Environment::PlatformTermination::terminate();

		return static_cast<size_t>(bytesRead);
	}

	size_t Files::write(const FileHandle& ro_Handle, const void* p_Buffer, size_t v_BytesToWrite) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::SEQUENTIAL)
			Environment::PlatformTermination::terminate();
		SPECTRA_ASSERT(p_Buffer != nullptr);

		DWORD bytesWritten = 0;
		BOOL ok = WriteFile(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
							p_Buffer, static_cast<DWORD>(v_BytesToWrite), &bytesWritten, nullptr);

		if (!ok)
			Environment::PlatformTermination::terminate();

		return static_cast<size_t>(bytesWritten);
	}

	void Files::seek(const FileHandle& ro_Handle, int64_t v_Offset, FileSeekOrigin v_Origin) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::SEQUENTIAL)
			Environment::PlatformTermination::terminate();

		LARGE_INTEGER li;
		li.QuadPart = v_Offset;

		BOOL ok = SetFilePointerEx(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
								   li, nullptr, toWin32SeekMethod(v_Origin));

		if (!ok)
			Environment::PlatformTermination::terminate();
	}

	void Files::flush(FileHandle& ro_Handle) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::SEQUENTIAL)
			Environment::PlatformTermination::terminate();

		BOOL ok = FlushFileBuffers(static_cast<HANDLE>(ro_Handle.m_NativeHandle));
		if (!ok)
			Environment::PlatformTermination::terminate();
	}

	size_t Files::getFileSize(FileHandle& ro_Handle) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();

		LARGE_INTEGER size{};
		BOOL ok = GetFileSizeEx(static_cast<HANDLE>(ro_Handle.m_NativeHandle), &size);
		if (!ok)
			Environment::PlatformTermination::terminate();

		return static_cast<size_t>(size.QuadPart);
	}

	void Files::readAsync(FileHandle& ro_Handle, AsyncFileHandler& ro_Handler,
						  void* p_Buffer, size_t v_BytesToRead, uint64_t v_Offset) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::ASYNCHRONOUS)
			Environment::PlatformTermination::terminate();
		if (ro_Handler.m_EventHandle == nullptr)
			Environment::PlatformTermination::terminate();
		SPECTRA_ASSERT(p_Buffer != nullptr);

		ro_Handler.m_Offset = v_Offset;
		ro_Handler.m_BytesTransferred = 0;

		OVERLAPPED ov{};
		ov.hEvent = static_cast<HANDLE>(ro_Handler.m_EventHandle);
		ov.Offset = static_cast<DWORD>(v_Offset & 0xFFFFFFFF);
		ov.OffsetHigh = static_cast<DWORD>(v_Offset >> 32);

		ResetEvent(static_cast<HANDLE>(ro_Handler.m_EventHandle));

		BOOL ok = ReadFile(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
						   p_Buffer, static_cast<DWORD>(v_BytesToRead), nullptr, &ov);

		if (!ok && GetLastError() != ERROR_IO_PENDING)
			Environment::PlatformTermination::terminate();
	}

	void Files::writeAsync(FileHandle& ro_Handle, AsyncFileHandler& ro_Handler,
						   const void* p_Buffer, size_t v_BytesToWrite, uint64_t v_Offset) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handle.m_Mode != FileIOMode::ASYNCHRONOUS)
			Environment::PlatformTermination::terminate();
		if (ro_Handler.m_EventHandle == nullptr)
			Environment::PlatformTermination::terminate();
		SPECTRA_ASSERT(p_Buffer != nullptr);

		ro_Handler.m_Offset = v_Offset;
		ro_Handler.m_BytesTransferred = 0;

		OVERLAPPED ov{};
		ov.hEvent = static_cast<HANDLE>(ro_Handler.m_EventHandle);
		ov.Offset = static_cast<DWORD>(v_Offset & 0xFFFFFFFF);
		ov.OffsetHigh = static_cast<DWORD>(v_Offset >> 32);

		ResetEvent(static_cast<HANDLE>(ro_Handler.m_EventHandle));

		BOOL ok = WriteFile(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
							p_Buffer, static_cast<DWORD>(v_BytesToWrite), nullptr, &ov);

		if (!ok && GetLastError() != ERROR_IO_PENDING)
			Environment::PlatformTermination::terminate();
	}

	size_t Files::getOverlappedResult(FileHandle& ro_Handle,
									  AsyncFileHandler& ro_Handler, bool v_Wait) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();
		if (ro_Handler.m_EventHandle == nullptr)
			Environment::PlatformTermination::terminate();

		OVERLAPPED ov{};
		ov.hEvent = static_cast<HANDLE>(ro_Handler.m_EventHandle);
		ov.Offset = static_cast<DWORD>(ro_Handler.m_Offset & 0xFFFFFFFF);
		ov.OffsetHigh = static_cast<DWORD>(ro_Handler.m_Offset >> 32);

		DWORD transferred = 0;
		BOOL ok = GetOverlappedResult(static_cast<HANDLE>(ro_Handle.m_NativeHandle),
									  &ov, &transferred, v_Wait ? TRUE : FALSE);

		if (!ok) {
			DWORD err = GetLastError();
			if (!v_Wait && err == ERROR_IO_INCOMPLETE)
				return 0; // still in-flight, polling caller
			Environment::PlatformTermination::terminate();
		}

		ro_Handler.m_BytesTransferred = static_cast<size_t>(transferred);
		return ro_Handler.m_BytesTransferred;
	}

	// Opaque enumeration state: two void* (find handle + heap-allocated WIN32_FIND_DATAW)
	// and a bool. No Win32 types in the struct itself.
	struct DirEnumState {
		void* m_FindHandle  = nullptr;
		void* m_PendingData = nullptr;   // heap-allocated WIN32_FIND_DATAW, null once consumed
		bool  m_HasPending  = false;
	};

	static void populateFileInfo(const void* p_Data, FileInfo& ro_OutInfo) {
		const WIN32_FIND_DATAW* data = static_cast<const WIN32_FIND_DATAW*>(p_Data);
		WideCharToMultiByte(CP_UTF8, 0, data->cFileName, -1,
							ro_OutInfo.m_FileName, sizeof(ro_OutInfo.m_FileName), nullptr, nullptr);
		ro_OutInfo.m_IsDirectory = (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		ro_OutInfo.m_IsReadOnly  = (data->dwFileAttributes & FILE_ATTRIBUTE_READONLY)  != 0;

		ULARGE_INTEGER size;
		size.LowPart  = data->nFileSizeLow;
		size.HighPart = data->nFileSizeHigh;
		ro_OutInfo.m_FileSize = static_cast<size_t>(size.QuadPart);

		ULARGE_INTEGER writeTime;
		writeTime.LowPart  = data->ftLastWriteTime.dwLowDateTime;
		writeTime.HighPart = data->ftLastWriteTime.dwHighDateTime;
		ro_OutInfo.m_LastWriteTime = writeTime.QuadPart;
	}

	DirectoryEnumHandle Files::beginEnumeration(const DirectoryEnumDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsFileSystemInitialized);
		SPECTRA_ASSERT(ro_Desc.m_Path != nullptr);

		const char* filter = (ro_Desc.m_Filter != nullptr) ? ro_Desc.m_Filter : "*";

		char composed[MAX_PATH * 2];
		int pathLen   = static_cast<int>(strlen(ro_Desc.m_Path));
		int filterLen = static_cast<int>(strlen(filter));

		if (pathLen + 1 + filterLen + 1 > MAX_PATH * 2)
			Environment::PlatformTermination::terminate();

		memcpy(composed, ro_Desc.m_Path, pathLen);
		composed[pathLen] = '\\';
		memcpy(composed + pathLen + 1, filter, filterLen + 1);

		wchar_t stackBuf[MAX_PATH * 2];
		int len = 0;
		wchar_t* wide = toWide(composed, stackBuf, MAX_PATH * 2, len);

		WIN32_FIND_DATAW* pending = static_cast<WIN32_FIND_DATAW*>(std::malloc(sizeof(WIN32_FIND_DATAW)));
		if (!pending)
			Environment::PlatformTermination::terminate();
		*pending = {};

		HANDLE h = FindFirstFileExW(wide, FindExInfoBasic, pending,
									FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
		freeWide(wide, stackBuf);

		if (h == INVALID_HANDLE_VALUE) {
			std::free(pending);
			Environment::PlatformTermination::terminate();
		}

		DirEnumState* state = static_cast<DirEnumState*>(std::malloc(sizeof(DirEnumState)));
		if (!state) {
			FindClose(h);
			std::free(pending);
			Environment::PlatformTermination::terminate();
		}

		state->m_FindHandle  = h;
		state->m_PendingData = pending;
		state->m_HasPending  = true;

		DirectoryEnumHandle handle;
		handle.m_NativeHandle = state;
		handle.m_IsValid      = true;
		return handle;
	}

	bool Files::next(DirectoryEnumHandle& ro_Handle, FileInfo& ro_OutInfo) {
		if (!isValidEnumHandle(ro_Handle))
			Environment::PlatformTermination::terminate();

		DirEnumState* state = static_cast<DirEnumState*>(ro_Handle.m_NativeHandle);

		for (;;) {
			if (state->m_HasPending) {
				populateFileInfo(state->m_PendingData, ro_OutInfo);
				std::free(state->m_PendingData);
				state->m_PendingData = nullptr;
				state->m_HasPending  = false;

				// Still need to skip . and .. if somehow present as first result
				if (ro_OutInfo.m_FileName[0] == '.' &&
					(ro_OutInfo.m_FileName[1] == '\0' ||
					(ro_OutInfo.m_FileName[1] == '.' && ro_OutInfo.m_FileName[2] == '\0')))
					continue;

				return true;
			}

			WIN32_FIND_DATAW findData{};
			BOOL ok = FindNextFileW(static_cast<HANDLE>(state->m_FindHandle), &findData);
			if (!ok) {
				if (GetLastError() == ERROR_NO_MORE_FILES)
					return false;
				Environment::PlatformTermination::terminate();
			}

			if (findData.cFileName[0] == L'.' &&
				(findData.cFileName[1] == L'\0' ||
				(findData.cFileName[1] == L'.' && findData.cFileName[2] == L'\0')))
				continue;

			populateFileInfo(&findData, ro_OutInfo);
			return true;
		}
	}

	void Files::closeEnumeration(DirectoryEnumHandle& ro_Handle) {
		if (!isValidEnumHandle(ro_Handle))
			Environment::PlatformTermination::terminate();

		DirEnumState* state = static_cast<DirEnumState*>(ro_Handle.m_NativeHandle);
		FindClose(static_cast<HANDLE>(state->m_FindHandle));
		if (state->m_PendingData) std::free(state->m_PendingData);
		std::free(state);

		ro_Handle.m_NativeHandle = nullptr;
		ro_Handle.m_IsValid      = false;
	}

	FileMappingHandle Files::createMapping(FileHandle& ro_Handle, FileAccess v_Access, size_t v_MaxSize) {
		if (!isValidHandle(ro_Handle))
			Environment::PlatformTermination::terminate();

		DWORD protect = toWin32MappingProtect(v_Access);
		DWORD sizeHigh = static_cast<DWORD>(v_MaxSize >> 32);
		DWORD sizeLow = static_cast<DWORD>(v_MaxSize & 0xFFFFFFFF);

		HANDLE h = CreateFileMappingW(static_cast<HANDLE>(ro_Handle.m_NativeHandle), nullptr, protect, sizeHigh, sizeLow, nullptr);

		if (h == nullptr || h == INVALID_HANDLE_VALUE)
			Environment::PlatformTermination::terminate();

		FileMappingHandle mapping;
		mapping.m_NativeHandle = h;
		mapping.m_MappingSize = v_MaxSize;
		return mapping;
	}

	void* Files::mapView(FileMappingHandle& ro_Mapping, uint64_t v_Offset, size_t v_Size, FileAccess v_Access) {
		if (!isValidMappingHandle(ro_Mapping))
			Environment::PlatformTermination::terminate();

		validateMappingRange(v_Offset, v_Size);

		DWORD access = toWin32MapViewAccess(v_Access);
		DWORD offsetHigh = static_cast<DWORD>(v_Offset >> 32);
		DWORD offsetLow = static_cast<DWORD>(v_Offset & 0xFFFFFFFF);

		void* view = MapViewOfFile(static_cast<HANDLE>(ro_Mapping.m_NativeHandle), access, offsetHigh, offsetLow, v_Size);

		if (view == nullptr)
			Environment::PlatformTermination::terminate();

		return view;
	}

	void Files::unmapView(const void* p_BaseAddress) {
		SPECTRA_ASSERT(p_BaseAddress != nullptr);

		BOOL ok = UnmapViewOfFile(p_BaseAddress);
		if (!ok)
			Environment::PlatformTermination::terminate();
	}

	void Files::closeMapping(FileMappingHandle& ro_Mapping) {
		if (!isValidMappingHandle(ro_Mapping))
			Environment::PlatformTermination::terminate();

		CloseHandle(static_cast<HANDLE>(ro_Mapping.m_NativeHandle));
		ro_Mapping.m_NativeHandle = nullptr;
		ro_Mapping.m_MappingSize = 0;
	}
}