#include "SpectraPlatformRuntime.h"

#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"
#include "InternalUtils.h"

namespace Spectra::Platform::Runtime::Internal {
	uint32_t toWin32Access(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return GENERIC_READ;
		case File::FileAccess::WRITE:      return GENERIC_WRITE;
		case File::FileAccess::READ_WRITE: return GENERIC_READ | GENERIC_WRITE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32ShareMode(File::FileShareMode v_Share) {
		switch (v_Share) {
		case File::FileShareMode::NONE:   return 0;
		case File::FileShareMode::READ:   return FILE_SHARE_READ;
		case File::FileShareMode::WRITE:  return FILE_SHARE_WRITE;
		case File::FileShareMode::REMOVE: return FILE_SHARE_DELETE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32CreationDisposition(File::FileOpenMode v_OpenMode) {
		switch (v_OpenMode) {
		case File::FileOpenMode::CREATE_NEW_FILE:        return CREATE_NEW;
		case File::FileOpenMode::CREATE_ALWAYS_FILE:     return CREATE_ALWAYS;
		case File::FileOpenMode::OPEN_EXISTING_FILE:     return OPEN_EXISTING;
		case File::FileOpenMode::OPEN_ALWAYS_FILE:       return OPEN_ALWAYS;
		case File::FileOpenMode::TRUNCATE_EXISTING_FILE: return TRUNCATE_EXISTING;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32MappingProtect(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return PAGE_READONLY;
		case File::FileAccess::WRITE:
		case File::FileAccess::READ_WRITE: return PAGE_READWRITE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32MapViewAccess(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return FILE_MAP_READ;
		case File::FileAccess::WRITE:      return FILE_MAP_WRITE;
		case File::FileAccess::READ_WRITE: return FILE_MAP_ALL_ACCESS;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32SeekMethod(File::FileSeekOrigin v_Origin) {
		switch (v_Origin) {
		case File::FileSeekOrigin::BEGIN:   return FILE_BEGIN;
		case File::FileSeekOrigin::CURRENT: return FILE_CURRENT;
		case File::FileSeekOrigin::END:     return FILE_END;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t toWin32Protect(Memory::MemoryProtect v_Protect) {
		switch (v_Protect) {
		case Memory::MemoryProtect::NO_ACCESS:          return PAGE_NOACCESS;
		case Memory::MemoryProtect::READ_ONLY:          return PAGE_READONLY;
		case Memory::MemoryProtect::READ_WRITE:         return PAGE_READWRITE;
		case Memory::MemoryProtect::EXECUTE:            return PAGE_EXECUTE;
		case Memory::MemoryProtect::EXECUTE_READ:       return PAGE_EXECUTE_READ;
		case Memory::MemoryProtect::EXECUTE_READ_WRITE: return PAGE_EXECUTE_READWRITE;
		case Memory::MemoryProtect::GUARD:              return PAGE_READWRITE | PAGE_GUARD;
		}
		SPECTRA_UNREACHABLE();
	}

	Memory::MemoryProtect fromWin32Protect(uint32_t v_Protect) {
		if (v_Protect & PAGE_GUARD)              return Memory::MemoryProtect::GUARD;
		if (v_Protect & PAGE_NOACCESS)            return Memory::MemoryProtect::NO_ACCESS;
		if (v_Protect & PAGE_READONLY)            return Memory::MemoryProtect::READ_ONLY;
		if (v_Protect & PAGE_READWRITE)           return Memory::MemoryProtect::READ_WRITE;
		if (v_Protect & PAGE_EXECUTE)             return Memory::MemoryProtect::EXECUTE;
		if (v_Protect & PAGE_EXECUTE_READ)        return Memory::MemoryProtect::EXECUTE_READ;
		if (v_Protect & PAGE_EXECUTE_READWRITE)   return Memory::MemoryProtect::EXECUTE_READ_WRITE;
		return Memory::MemoryProtect::NO_ACCESS;
	}

	Memory::MemoryState fromWin32MemState(uint32_t v_State) {
		switch (v_State) {
		case MEM_FREE:    return Memory::MemoryState::UNINITIALIZED;
		case MEM_RESERVE: return Memory::MemoryState::RESERVE;
		case MEM_COMMIT:  return Memory::MemoryState::COMMIT;
		}
		SPECTRA_UNREACHABLE();
	}

	int32_t toWin32Priority(Thread::Priority v_Priority) {
		switch (v_Priority) {
		case Thread::Priority::PRIORITY_IDLE:          return THREAD_PRIORITY_IDLE;
		case Thread::Priority::PRIORITY_LOWEST:        return THREAD_PRIORITY_LOWEST;
		case Thread::Priority::PRIORITY_BELOW_NORMAL:  return THREAD_PRIORITY_BELOW_NORMAL;
		case Thread::Priority::PRIORITY_NORMAL:        return THREAD_PRIORITY_NORMAL;
		case Thread::Priority::PRIORITY_ABOVE_NORMAL:  return THREAD_PRIORITY_ABOVE_NORMAL;
		case Thread::Priority::PRIORITY_HIGHEST:       return THREAD_PRIORITY_HIGHEST;
		case Thread::Priority::PRIORITY_TIME_CRITICAL: return THREAD_PRIORITY_TIME_CRITICAL;
		}
		SPECTRA_UNREACHABLE();
	}

	Thread::Priority fromWin32Priority(int32_t v_Priority) {
		switch (v_Priority) {
		case THREAD_PRIORITY_IDLE:          return Thread::Priority::PRIORITY_IDLE;
		case THREAD_PRIORITY_LOWEST:        return Thread::Priority::PRIORITY_LOWEST;
		case THREAD_PRIORITY_BELOW_NORMAL:  return Thread::Priority::PRIORITY_BELOW_NORMAL;
		case THREAD_PRIORITY_NORMAL:        return Thread::Priority::PRIORITY_NORMAL;
		case THREAD_PRIORITY_ABOVE_NORMAL:  return Thread::Priority::PRIORITY_ABOVE_NORMAL;
		case THREAD_PRIORITY_HIGHEST:       return Thread::Priority::PRIORITY_HIGHEST;
		case THREAD_PRIORITY_TIME_CRITICAL: return Thread::Priority::PRIORITY_TIME_CRITICAL;
		}
		SPECTRA_UNREACHABLE();
	}

	GROUP_AFFINITY fromAffinityDesc(const Thread::AffinityDesc& ro_Desc) {
		GROUP_AFFINITY affinity{};
		affinity.Group = ro_Desc.m_GroupId;
		affinity.Mask = ro_Desc.m_AffMask;
		return affinity;
	}

	Thread::AffinityDesc toAffinityDesc(const GROUP_AFFINITY& ro_Affinity) {
		Thread::AffinityDesc desc{};
		desc.m_GroupId = ro_Affinity.Group;
		desc.m_AffMask = ro_Affinity.Mask;
		return desc;
	}
}
