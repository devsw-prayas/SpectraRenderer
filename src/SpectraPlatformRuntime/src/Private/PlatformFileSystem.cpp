#include "SpectraPlatformRuntime.h"

#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"
#include "PlatformFileSystem.h"
#include "ProcessEnvironment.h"
#include "SpectraDiagnostics.h"
#include "InternalUtils.h"

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

		DWORD access = Internal::FileMappings::toWin32Access(ro_Desc.m_Access);
		DWORD shareMode = Internal::FileMappings::toWin32ShareMode(ro_Desc.m_ShareMode);
		DWORD disposition = Internal::FileMappings::toWin32CreationDisposition(ro_Desc.m_OpenMode);
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
								   li, nullptr, Internal::FileMappings::toWin32SeekMethod(v_Origin));

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

		DWORD protect = Internal::FileMappings::toWin32MappingProtect(v_Access);
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

		DWORD access = Internal::FileMappings::toWin32MapViewAccess(v_Access);
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
namespace Spectra::Platform::Runtime::File {
	namespace {
		constexpr uint32_t WIDE_PATH_CAPACITY = 32768;
		constexpr uint32_t MAX_TREE_DEPTH = 128;

		bool isDotEntry(const wchar_t* p_Name) {
			return p_Name[0] == L'.' && (p_Name[1] == L'\0' || (p_Name[1] == L'.' && p_Name[2] == L'\0'));
		}

		// p_Path holds the directory on entry and is restored on return.
		bool removeTreeAt(wchar_t* p_Path, size_t v_Length, uint32_t v_Depth) {
			if (v_Depth > MAX_TREE_DEPTH || v_Length + 3 >= WIDE_PATH_CAPACITY) return false;
			p_Path[v_Length] = L'\\';
			p_Path[v_Length + 1] = L'*';
			p_Path[v_Length + 2] = L'\0';

			WIN32_FIND_DATAW data{};
			HANDLE find = FindFirstFileExW(p_Path, FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
			bool ok = true;
			if (find != INVALID_HANDLE_VALUE) {
				do {
					if (isDotEntry(data.cFileName)) continue;
					const size_t nameLength = wcslen(data.cFileName);
					if (v_Length + 1 + nameLength >= WIDE_PATH_CAPACITY) {
						ok = false;
						break;
					}
					std::memcpy(p_Path + v_Length + 1, data.cFileName, (nameLength + 1) * sizeof(wchar_t));
					// Reparse points are unlinked, never followed.
					const bool isDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
					const bool isLink = (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
					if (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						SetFileAttributesW(p_Path, data.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
					if (isDirectory && !isLink) ok = removeTreeAt(p_Path, v_Length + 1 + nameLength, v_Depth + 1);
					else if (isDirectory) ok = RemoveDirectoryW(p_Path) != 0;
					else ok = DeleteFileW(p_Path) != 0;
				} while (ok && FindNextFileW(find, &data));
				FindClose(find);
			}
			p_Path[v_Length] = L'\0';
			return ok && RemoveDirectoryW(p_Path) != 0;
		}
	}

	bool Files::deleteFile(const wchar_t* p_Path) {
		SPECTRA_ASSERT(p_Path != nullptr);
		return DeleteFileW(p_Path) != 0;
	}

	bool Files::renameFile(const wchar_t* p_OldPath, const wchar_t* p_NewPath) {
		SPECTRA_ASSERT(p_OldPath != nullptr && p_NewPath != nullptr);
		return MoveFileExW(p_OldPath, p_NewPath, MOVEFILE_REPLACE_EXISTING) != 0;
	}

	bool Files::createDirectory(const wchar_t* p_Path) {
		SPECTRA_ASSERT(p_Path != nullptr);
		if (CreateDirectoryW(p_Path, nullptr)) return true;
		return GetLastError() == ERROR_ALREADY_EXISTS && getAttribute(p_Path) == PathAttribute::DIRECTORY;
	}

	bool Files::removeDirectory(const wchar_t* p_Path) {
		SPECTRA_ASSERT(p_Path != nullptr);
		return RemoveDirectoryW(p_Path) != 0;
	}

	bool Files::removeTree(const wchar_t* p_Path) {
		SPECTRA_ASSERT(p_Path != nullptr);
		const size_t length = wcslen(p_Path);
		if (length == 0 || length >= WIDE_PATH_CAPACITY) return false;
		if (getAttribute(p_Path) != PathAttribute::DIRECTORY) return false;

		wchar_t* path = static_cast<wchar_t*>(std::malloc(WIDE_PATH_CAPACITY * sizeof(wchar_t)));
		if (path == nullptr) return false;
		std::memcpy(path, p_Path, (length + 1) * sizeof(wchar_t));
		const bool ok = removeTreeAt(path, length, 0);
		std::free(path);
		return ok;
	}

	PathAttribute Files::getAttribute(const wchar_t* p_Path) {
		SPECTRA_ASSERT(p_Path != nullptr);
		const DWORD attributes = GetFileAttributesW(p_Path);
		if (attributes == INVALID_FILE_ATTRIBUTES) return PathAttribute::MISSING;
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? PathAttribute::DIRECTORY : PathAttribute::FILE;
	}

	bool Files::forEachChild(const wchar_t* p_Directory, ChildVisitFn p_Visit, void* p_Context) {
		SPECTRA_ASSERT(p_Directory != nullptr && p_Visit != nullptr);
		const size_t length = wcslen(p_Directory);
		wchar_t pattern[WIDE_PATH_CAPACITY];
		if (length + 3 > WIDE_PATH_CAPACITY) return false;
		std::memcpy(pattern, p_Directory, length * sizeof(wchar_t));
		std::memcpy(pattern + length, L"\\*", 3 * sizeof(wchar_t));

		WIN32_FIND_DATAW data{};
		HANDLE find = FindFirstFileExW(pattern, FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
		if (find == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND;
		do {
			if (isDotEntry(data.cFileName)) continue;
			if (!p_Visit(data.cFileName, (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, p_Context)) break;
		} while (FindNextFileW(find, &data));
		FindClose(find);
		return true;
	}

	bool Files::getKnownFolder(KnownFolder v_Folder, wchar_t* p_Out, uint32_t v_Capacity, uint32_t& r_Length) {
		SPECTRA_ASSERT(p_Out != nullptr && v_Capacity > 0);
		r_Length = 0;
		DWORD length = 0;
		switch (v_Folder) {
			case KnownFolder::LOCAL_APP_DATA:
				length = GetEnvironmentVariableW(L"LOCALAPPDATA", p_Out, v_Capacity);
				break;
			case KnownFolder::TEMP:
				length = GetTempPathW(v_Capacity, p_Out);
				break;
			case KnownFolder::EXECUTABLE_DIR:
				length = GetModuleFileNameW(nullptr, p_Out, v_Capacity);
				if (length == v_Capacity) return false;
				while (length > 0 && p_Out[length - 1] != L'\\') --length;
				break;
		}
		if (length == 0 || length >= v_Capacity) return false;
		while (length > 0 && (p_Out[length - 1] == L'\\' || p_Out[length - 1] == L'/')) --length;
		p_Out[length] = L'\0';
		r_Length = length;
		return length > 0;
	}
}

namespace Spectra::Platform::Runtime::File {
	namespace {
		// ReadFile/WriteFile take a DWORD; larger requests go in chunks.
		constexpr size_t IO_CHUNK = 1ull << 30;

		uint64_t ticksOf(const FILETIME& r_Time) {
			return (uint64_t(r_Time.dwHighDateTime) << 32) | r_Time.dwLowDateTime;
		}

		DWORD dispositionOf(FileOpenMode v_Mode) {
			switch (v_Mode) {
				case FileOpenMode::CREATE_NEW_FILE:        return CREATE_NEW;
				case FileOpenMode::CREATE_ALWAYS_FILE:     return CREATE_ALWAYS;
				case FileOpenMode::OPEN_EXISTING_FILE:     return OPEN_EXISTING;
				case FileOpenMode::OPEN_ALWAYS_FILE:       return OPEN_ALWAYS;
				case FileOpenMode::TRUNCATE_EXISTING_FILE: return TRUNCATE_EXISTING;
			}
			return OPEN_EXISTING;
		}
	}

	void* Files::openNative(const wchar_t* p_Path, const NativeOpenDesc& ro_Desc) {
		SPECTRA_ASSERT(p_Path != nullptr);
		DWORD access = 0;
		if (ro_Desc.m_Access != FileAccess::WRITE) access |= GENERIC_READ;
		if (ro_Desc.m_Access != FileAccess::READ) access |= GENERIC_WRITE;

		const uint8_t share = uint8_t(ro_Desc.m_Share);
		DWORD shareMode = 0;
		if (share & uint8_t(NativeShare::READ)) shareMode |= FILE_SHARE_READ;
		if (share & uint8_t(NativeShare::WRITE)) shareMode |= FILE_SHARE_WRITE;
		if (share & uint8_t(NativeShare::REMOVE)) shareMode |= FILE_SHARE_DELETE;

		DWORD flags = FILE_ATTRIBUTE_NORMAL;
		if (ro_Desc.m_Buffering == NativeBuffering::DIRECT) flags |= FILE_FLAG_NO_BUFFERING;
		if (ro_Desc.m_Buffering == NativeBuffering::WRITE_THROUGH) flags |= FILE_FLAG_WRITE_THROUGH;

		HANDLE handle = CreateFileW(p_Path, access, shareMode, nullptr, dispositionOf(ro_Desc.m_OpenMode), flags, nullptr);
		return handle == INVALID_HANDLE_VALUE ? nullptr : handle;
	}

	bool Files::closeNative(void* p_Handle) {
		return p_Handle != nullptr && CloseHandle(static_cast<HANDLE>(p_Handle)) != 0;
	}

	bool Files::readAt(void* p_Handle, void* p_Buffer, size_t v_Bytes, uint64_t v_Offset, size_t& r_Read) {
		r_Read = 0;
		while (r_Read < v_Bytes) {
			const DWORD chunk = DWORD(std::min(v_Bytes - r_Read, IO_CHUNK));
			const uint64_t offset = v_Offset + r_Read;
			OVERLAPPED position{};
			position.Offset = DWORD(offset);
			position.OffsetHigh = DWORD(offset >> 32);
			DWORD done = 0;
			if (!ReadFile(static_cast<HANDLE>(p_Handle), static_cast<uint8_t*>(p_Buffer) + r_Read, chunk, &done, &position))
				return GetLastError() == ERROR_HANDLE_EOF;
			r_Read += done;
			if (done < chunk) return true;  // end of file
		}
		return true;
	}

	bool Files::writeAt(void* p_Handle, const void* p_Buffer, size_t v_Bytes, uint64_t v_Offset, size_t& r_Written) {
		r_Written = 0;
		while (r_Written < v_Bytes) {
			const DWORD chunk = DWORD(std::min(v_Bytes - r_Written, IO_CHUNK));
			const uint64_t offset = v_Offset + r_Written;
			OVERLAPPED position{};
			position.Offset = DWORD(offset);
			position.OffsetHigh = DWORD(offset >> 32);
			DWORD done = 0;
			if (!WriteFile(static_cast<HANDLE>(p_Handle), static_cast<const uint8_t*>(p_Buffer) + r_Written, chunk, &done, &position)) return false;
			r_Written += done;
			if (done < chunk) return false;
		}
		return true;
	}

	bool Files::flushNative(void* p_Handle) {
		return FlushFileBuffers(static_cast<HANDLE>(p_Handle)) != 0;
	}

	bool Files::getSize(void* p_Handle, uint64_t& r_Size) {
		LARGE_INTEGER size{};
		if (!GetFileSizeEx(static_cast<HANDLE>(p_Handle), &size)) return false;
		r_Size = uint64_t(size.QuadPart);
		return true;
	}

	bool Files::setSize(void* p_Handle, uint64_t v_Size) {
		FILE_END_OF_FILE_INFO info{};
		info.EndOfFile.QuadPart = LONGLONG(v_Size);
		return SetFileInformationByHandle(static_cast<HANDLE>(p_Handle), FileEndOfFileInfo, &info, sizeof(info)) != 0;
	}

	uint32_t Files::sectorSize(void* p_Handle) {
		FILE_STORAGE_INFO info{};
		if (GetFileInformationByHandleEx(static_cast<HANDLE>(p_Handle), FileStorageInfo, &info, sizeof(info)) && info.PhysicalBytesPerSectorForPerformance != 0)
			return info.PhysicalBytesPerSectorForPerformance;
		return 4096;
	}

	bool Files::stat(const wchar_t* p_Path, NativeStat& r_Stat) {
		WIN32_FILE_ATTRIBUTE_DATA data{};
		if (!GetFileAttributesExW(p_Path, GetFileExInfoStandard, &data)) return false;
		r_Stat.m_Size = (uint64_t(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
		r_Stat.m_CreationTime = ticksOf(data.ftCreationTime);
		r_Stat.m_WriteTime = ticksOf(data.ftLastWriteTime);
		r_Stat.m_IsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		r_Stat.m_IsReadOnly = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
		r_Stat.m_IsLink = (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
		return true;
	}

	bool Files::copyFile(const wchar_t* p_From, const wchar_t* p_To, bool v_Overwrite) {
		return CopyFileW(p_From, p_To, v_Overwrite ? FALSE : TRUE) != 0;
	}

	NativeError Files::lastError() {
		switch (GetLastError()) {
			case ERROR_SUCCESS:                                      return NativeError::NONE;
			case ERROR_FILE_NOT_FOUND: case ERROR_PATH_NOT_FOUND:    return NativeError::NOT_FOUND;
			case ERROR_FILE_EXISTS: case ERROR_ALREADY_EXISTS:       return NativeError::ALREADY_EXISTS;
			case ERROR_ACCESS_DENIED:                                return NativeError::ACCESS_DENIED;
			case ERROR_SHARING_VIOLATION: case ERROR_LOCK_VIOLATION: return NativeError::SHARING_VIOLATION;
			case ERROR_DISK_FULL: case ERROR_HANDLE_DISK_FULL:       return NativeError::DISK_FULL;
			default:                                                 return NativeError::OTHER;
		}
	}

	void* Files::createSection(void* p_File, bool v_Writable, uint64_t v_MaxSize) {
		return CreateFileMappingW(static_cast<HANDLE>(p_File), nullptr, v_Writable ? PAGE_READWRITE : PAGE_READONLY,
			DWORD(v_MaxSize >> 32), DWORD(v_MaxSize), nullptr);
	}

	bool Files::closeSection(void* p_Section) {
		return p_Section != nullptr && CloseHandle(static_cast<HANDLE>(p_Section)) != 0;
	}
}
