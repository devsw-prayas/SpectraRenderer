#include "SpectraPlatformRuntime.h"

#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"
#include "InternalUtils.h"

namespace Spectra::Platform::Runtime::Internal {
	uint32_t FileMappings::toWin32Access(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return GENERIC_READ;
		case File::FileAccess::WRITE:      return GENERIC_WRITE;
		case File::FileAccess::READ_WRITE: return GENERIC_READ | GENERIC_WRITE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t FileMappings::toWin32ShareMode(File::FileShareMode v_Share) {
		switch (v_Share) {
		case File::FileShareMode::NONE:   return 0;
		case File::FileShareMode::READ:   return FILE_SHARE_READ;
		case File::FileShareMode::WRITE:  return FILE_SHARE_WRITE;
		case File::FileShareMode::REMOVE: return FILE_SHARE_DELETE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t FileMappings::toWin32CreationDisposition(File::FileOpenMode v_OpenMode) {
		switch (v_OpenMode) {
		case File::FileOpenMode::CREATE_NEW_FILE:        return CREATE_NEW;
		case File::FileOpenMode::CREATE_ALWAYS_FILE:     return CREATE_ALWAYS;
		case File::FileOpenMode::OPEN_EXISTING_FILE:     return OPEN_EXISTING;
		case File::FileOpenMode::OPEN_ALWAYS_FILE:       return OPEN_ALWAYS;
		case File::FileOpenMode::TRUNCATE_EXISTING_FILE: return TRUNCATE_EXISTING;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t FileMappings::toWin32MappingProtect(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return PAGE_READONLY;
		case File::FileAccess::WRITE:
		case File::FileAccess::READ_WRITE: return PAGE_READWRITE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t FileMappings::toWin32MapViewAccess(File::FileAccess v_Access) {
		switch (v_Access) {
		case File::FileAccess::READ:       return FILE_MAP_READ;
		case File::FileAccess::WRITE:      return FILE_MAP_WRITE;
		case File::FileAccess::READ_WRITE: return FILE_MAP_ALL_ACCESS;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t FileMappings::toWin32SeekMethod(File::FileSeekOrigin v_Origin) {
		switch (v_Origin) {
		case File::FileSeekOrigin::BEGIN:   return FILE_BEGIN;
		case File::FileSeekOrigin::CURRENT: return FILE_CURRENT;
		case File::FileSeekOrigin::END:     return FILE_END;
		}
		SPECTRA_UNREACHABLE();
	}

	uint32_t MemMappings::toWin32Protect(Memory::MemoryProtect v_Protect) {
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

	Memory::MemoryProtect MemMappings::fromWin32Protect(uint32_t v_Protect) {
		if (v_Protect & PAGE_GUARD)              return Memory::MemoryProtect::GUARD;
		if (v_Protect & PAGE_NOACCESS)            return Memory::MemoryProtect::NO_ACCESS;
		if (v_Protect & PAGE_READONLY)            return Memory::MemoryProtect::READ_ONLY;
		if (v_Protect & PAGE_READWRITE)           return Memory::MemoryProtect::READ_WRITE;
		if (v_Protect & PAGE_EXECUTE)             return Memory::MemoryProtect::EXECUTE;
		if (v_Protect & PAGE_EXECUTE_READ)        return Memory::MemoryProtect::EXECUTE_READ;
		if (v_Protect & PAGE_EXECUTE_READWRITE)   return Memory::MemoryProtect::EXECUTE_READ_WRITE;
		return Memory::MemoryProtect::NO_ACCESS;
	}

	Memory::MemoryState MemMappings::fromWin32MemState(uint32_t v_State) {
		switch (v_State) {
		case MEM_FREE:    return Memory::MemoryState::UNINITIALIZED;
		case MEM_RESERVE: return Memory::MemoryState::RESERVE;
		case MEM_COMMIT:  return Memory::MemoryState::COMMIT;
		}
		SPECTRA_UNREACHABLE();
	}

	int32_t ThreadMappings::toWin32Priority(Thread::Priority v_Priority) {
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

	Thread::Priority ThreadMappings::fromWin32Priority(int32_t v_Priority) {
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

	GROUP_AFFINITY ThreadMappings::fromAffinityDesc(const Thread::AffinityDesc& ro_Desc) {
		GROUP_AFFINITY affinity{};
		affinity.Group = ro_Desc.m_GroupId;
		affinity.Mask = ro_Desc.m_AffMask;
		return affinity;
	}

	Thread::AffinityDesc ThreadMappings::toAffinityDesc(const GROUP_AFFINITY& ro_Affinity) {
		Thread::AffinityDesc desc{};
		desc.m_GroupId = ro_Affinity.Group;
		desc.m_AffMask = ro_Affinity.Mask;
		return desc;
	}

	// -------------------------------------------------------------------------
	// WindowMappings
	// -------------------------------------------------------------------------

	uint32_t WindowMappings::toWin32Style(Windows::WindowStyleFlags v_Flags) {
		using F = Windows::WindowStyleFlags;
		// WS_OVERLAPPED/WS_CAPTION/WS_SYSMENU are unconditional base bits, kept
		// even for UNDECORATED windows (chrome is hidden at paint level via
		// WM_NCCALCSIZE/WM_NCPAINT/WM_NCACTIVATE, not by stripping style bits).
		uint32_t style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::RESIZABLE))   style |= WS_THICKFRAME;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::MINIMIZABLE)) style |= WS_MINIMIZEBOX;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::MAXIMIZABLE)) style |= WS_MAXIMIZEBOX;
		return style;
	}

	uint32_t WindowMappings::toWin32ExStyle(Windows::WindowStyleFlags v_Flags) {
		using F = Windows::WindowStyleFlags;
		uint32_t exStyle = 0;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::TOPMOST))               exStyle |= WS_EX_TOPMOST;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::APP_WINDOW))            exStyle |= WS_EX_APPWINDOW;
		if (static_cast<uint32_t>(v_Flags) & static_cast<uint32_t>(F::NO_REDIRECTION_BITMAP)) exStyle |= WS_EX_NOREDIRECTIONBITMAP;
		return exStyle;
	}

	int32_t WindowMappings::toWin32HitTest(Windows::WindowHitTestResult v_Result) {
		switch (v_Result) {
		case Windows::WindowHitTestResult::CLIENT:  return HTCLIENT;
		case Windows::WindowHitTestResult::CAPTION: return HTCAPTION;
		case Windows::WindowHitTestResult::NOWHERE: return HTNOWHERE;
		}
		SPECTRA_UNREACHABLE();
	}

	uint16_t WindowMappings::toWin32VKey(Windows::KeyCode v_Code) {
		using KC = Windows::KeyCode;
		switch (v_Code) {
		case KC::KC_BACKSPACE:        return VK_BACK;
		case KC::KC_TAB:              return VK_TAB;
		case KC::KC_ENTER:            return VK_RETURN;
		case KC::KC_SHIFT:            return VK_SHIFT;
		case KC::KC_CONTROL:          return VK_CONTROL;
		case KC::KC_ALT:              return VK_MENU;
		case KC::KC_PAUSE:            return VK_PAUSE;
		case KC::KC_CAPS_LOCK:        return VK_CAPITAL;
		case KC::KC_ESCAPE:           return VK_ESCAPE;
		case KC::KC_SPACE:            return VK_SPACE;
		case KC::KC_PAGE_UP:          return VK_PRIOR;
		case KC::KC_PAGE_DOWN:        return VK_NEXT;
		case KC::KC_END:              return VK_END;
		case KC::KC_HOME:             return VK_HOME;
		case KC::KC_LEFT:             return VK_LEFT;
		case KC::KC_UP:               return VK_UP;
		case KC::KC_RIGHT:            return VK_RIGHT;
		case KC::KC_DOWN:             return VK_DOWN;
		case KC::KC_PRINT_SCREEN:     return VK_SNAPSHOT;
		case KC::KC_INSERT:           return VK_INSERT;
		case KC::KC_DELETE:           return VK_DELETE;
		case KC::KC_0:                return '0';
		case KC::KC_1:                return '1';
		case KC::KC_2:                return '2';
		case KC::KC_3:                return '3';
		case KC::KC_4:                return '4';
		case KC::KC_5:                return '5';
		case KC::KC_6:                return '6';
		case KC::KC_7:                return '7';
		case KC::KC_8:                return '8';
		case KC::KC_9:                return '9';
		case KC::KC_A:                return 'A';
		case KC::KC_B:                return 'B';
		case KC::KC_C:                return 'C';
		case KC::KC_D:                return 'D';
		case KC::KC_E:                return 'E';
		case KC::KC_F:                return 'F';
		case KC::KC_G:                return 'G';
		case KC::KC_H:                return 'H';
		case KC::KC_I:                return 'I';
		case KC::KC_J:                return 'J';
		case KC::KC_K:                return 'K';
		case KC::KC_L:                return 'L';
		case KC::KC_M:                return 'M';
		case KC::KC_N:                return 'N';
		case KC::KC_O:                return 'O';
		case KC::KC_P:                return 'P';
		case KC::KC_Q:                return 'Q';
		case KC::KC_R:                return 'R';
		case KC::KC_S:                return 'S';
		case KC::KC_T:                return 'T';
		case KC::KC_U:                return 'U';
		case KC::KC_V:                return 'V';
		case KC::KC_W:                return 'W';
		case KC::KC_X:                return 'X';
		case KC::KC_Y:                return 'Y';
		case KC::KC_Z:                return 'Z';
		case KC::KC_LEFT_META:        return VK_LWIN;
		case KC::KC_RIGHT_META:       return VK_RWIN;
		case KC::KC_APPLICATION:      return VK_APPS;
		case KC::KC_NUMPAD_0:         return VK_NUMPAD0;
		case KC::KC_NUMPAD_1:         return VK_NUMPAD1;
		case KC::KC_NUMPAD_2:         return VK_NUMPAD2;
		case KC::KC_NUMPAD_3:         return VK_NUMPAD3;
		case KC::KC_NUMPAD_4:         return VK_NUMPAD4;
		case KC::KC_NUMPAD_5:         return VK_NUMPAD5;
		case KC::KC_NUMPAD_6:         return VK_NUMPAD6;
		case KC::KC_NUMPAD_7:         return VK_NUMPAD7;
		case KC::KC_NUMPAD_8:         return VK_NUMPAD8;
		case KC::KC_NUMPAD_9:         return VK_NUMPAD9;
		case KC::KC_NUMPAD_MULTIPLY:  return VK_MULTIPLY;
		case KC::KC_NUMPAD_ADD:       return VK_ADD;
		case KC::KC_NUMPAD_SEPARATOR: return VK_SEPARATOR;
		case KC::KC_NUMPAD_SUBTRACT:  return VK_SUBTRACT;
		case KC::KC_NUMPAD_DECIMAL:   return VK_DECIMAL;
		case KC::KC_NUMPAD_DIVIDE:    return VK_DIVIDE;
		case KC::KC_F1:               return VK_F1;
		case KC::KC_F2:               return VK_F2;
		case KC::KC_F3:               return VK_F3;
		case KC::KC_F4:               return VK_F4;
		case KC::KC_F5:               return VK_F5;
		case KC::KC_F6:               return VK_F6;
		case KC::KC_F7:               return VK_F7;
		case KC::KC_F8:               return VK_F8;
		case KC::KC_F9:               return VK_F9;
		case KC::KC_F10:              return VK_F10;
		case KC::KC_F11:              return VK_F11;
		case KC::KC_F12:              return VK_F12;
		case KC::KC_F13:              return VK_F13;
		case KC::KC_F14:              return VK_F14;
		case KC::KC_F15:              return VK_F15;
		case KC::KC_F16:              return VK_F16;
		case KC::KC_F17:              return VK_F17;
		case KC::KC_F18:              return VK_F18;
		case KC::KC_F19:              return VK_F19;
		case KC::KC_F20:              return VK_F20;
		case KC::KC_F21:              return VK_F21;
		case KC::KC_F22:              return VK_F22;
		case KC::KC_F23:              return VK_F23;
		case KC::KC_F24:              return VK_F24;
		case KC::KC_NUM_LOCK:         return VK_NUMLOCK;
		case KC::KC_SCROLL_LOCK:      return VK_SCROLL;
		case KC::KC_LEFT_SHIFT:       return VK_LSHIFT;
		case KC::KC_RIGHT_SHIFT:      return VK_RSHIFT;
		case KC::KC_LEFT_CONTROL:     return VK_LCONTROL;
		case KC::KC_RIGHT_CONTROL:    return VK_RCONTROL;
		case KC::KC_LEFT_ALT:         return VK_LMENU;
		case KC::KC_RIGHT_ALT:        return VK_RMENU;
		case KC::KC_SEMICOLON:        return VK_OEM_1;
		case KC::KC_EQUAL:            return VK_OEM_PLUS;
		case KC::KC_COMMA:            return VK_OEM_COMMA;
		case KC::KC_MINUS:            return VK_OEM_MINUS;
		case KC::KC_PERIOD:           return VK_OEM_PERIOD;
		case KC::KC_SLASH:            return VK_OEM_2;
		case KC::KC_BACKTICK:         return VK_OEM_3;
		case KC::KC_LEFT_BRACKET:     return VK_OEM_4;
		case KC::KC_BACKSLASH:        return VK_OEM_5;
		case KC::KC_RIGHT_BRACKET:    return VK_OEM_6;
		case KC::KC_APOSTROPHE:       return VK_OEM_7;
		case KC::KC_UNKNOWN:          return 0;
		}
		SPECTRA_UNREACHABLE();
	}

	Windows::KeyCode WindowMappings::fromWin32VKey(uint16_t v_VKey) {
		using KC = Windows::KeyCode;
		switch (v_VKey) {
		case VK_BACK:       return KC::KC_BACKSPACE;
		case VK_TAB:        return KC::KC_TAB;
		case VK_RETURN:     return KC::KC_ENTER;
		case VK_SHIFT:      return KC::KC_SHIFT;
		case VK_CONTROL:    return KC::KC_CONTROL;
		case VK_MENU:       return KC::KC_ALT;
		case VK_PAUSE:      return KC::KC_PAUSE;
		case VK_CAPITAL:    return KC::KC_CAPS_LOCK;
		case VK_ESCAPE:     return KC::KC_ESCAPE;
		case VK_SPACE:      return KC::KC_SPACE;
		case VK_PRIOR:      return KC::KC_PAGE_UP;
		case VK_NEXT:       return KC::KC_PAGE_DOWN;
		case VK_END:        return KC::KC_END;
		case VK_HOME:       return KC::KC_HOME;
		case VK_LEFT:       return KC::KC_LEFT;
		case VK_UP:         return KC::KC_UP;
		case VK_RIGHT:      return KC::KC_RIGHT;
		case VK_DOWN:       return KC::KC_DOWN;
		case VK_SNAPSHOT:   return KC::KC_PRINT_SCREEN;
		case VK_INSERT:     return KC::KC_INSERT;
		case VK_DELETE:     return KC::KC_DELETE;
		case '0':           return KC::KC_0;
		case '1':           return KC::KC_1;
		case '2':           return KC::KC_2;
		case '3':           return KC::KC_3;
		case '4':           return KC::KC_4;
		case '5':           return KC::KC_5;
		case '6':           return KC::KC_6;
		case '7':           return KC::KC_7;
		case '8':           return KC::KC_8;
		case '9':           return KC::KC_9;
		case 'A':           return KC::KC_A;
		case 'B':           return KC::KC_B;
		case 'C':           return KC::KC_C;
		case 'D':           return KC::KC_D;
		case 'E':           return KC::KC_E;
		case 'F':           return KC::KC_F;
		case 'G':           return KC::KC_G;
		case 'H':           return KC::KC_H;
		case 'I':           return KC::KC_I;
		case 'J':           return KC::KC_J;
		case 'K':           return KC::KC_K;
		case 'L':           return KC::KC_L;
		case 'M':           return KC::KC_M;
		case 'N':           return KC::KC_N;
		case 'O':           return KC::KC_O;
		case 'P':           return KC::KC_P;
		case 'Q':           return KC::KC_Q;
		case 'R':           return KC::KC_R;
		case 'S':           return KC::KC_S;
		case 'T':           return KC::KC_T;
		case 'U':           return KC::KC_U;
		case 'V':           return KC::KC_V;
		case 'W':           return KC::KC_W;
		case 'X':           return KC::KC_X;
		case 'Y':           return KC::KC_Y;
		case 'Z':           return KC::KC_Z;
		case VK_LWIN:       return KC::KC_LEFT_META;
		case VK_RWIN:       return KC::KC_RIGHT_META;
		case VK_APPS:       return KC::KC_APPLICATION;
		case VK_NUMPAD0:    return KC::KC_NUMPAD_0;
		case VK_NUMPAD1:    return KC::KC_NUMPAD_1;
		case VK_NUMPAD2:    return KC::KC_NUMPAD_2;
		case VK_NUMPAD3:    return KC::KC_NUMPAD_3;
		case VK_NUMPAD4:    return KC::KC_NUMPAD_4;
		case VK_NUMPAD5:    return KC::KC_NUMPAD_5;
		case VK_NUMPAD6:    return KC::KC_NUMPAD_6;
		case VK_NUMPAD7:    return KC::KC_NUMPAD_7;
		case VK_NUMPAD8:    return KC::KC_NUMPAD_8;
		case VK_NUMPAD9:    return KC::KC_NUMPAD_9;
		case VK_MULTIPLY:   return KC::KC_NUMPAD_MULTIPLY;
		case VK_ADD:        return KC::KC_NUMPAD_ADD;
		case VK_SEPARATOR:  return KC::KC_NUMPAD_SEPARATOR;
		case VK_SUBTRACT:   return KC::KC_NUMPAD_SUBTRACT;
		case VK_DECIMAL:    return KC::KC_NUMPAD_DECIMAL;
		case VK_DIVIDE:     return KC::KC_NUMPAD_DIVIDE;
		case VK_F1:         return KC::KC_F1;
		case VK_F2:         return KC::KC_F2;
		case VK_F3:         return KC::KC_F3;
		case VK_F4:         return KC::KC_F4;
		case VK_F5:         return KC::KC_F5;
		case VK_F6:         return KC::KC_F6;
		case VK_F7:         return KC::KC_F7;
		case VK_F8:         return KC::KC_F8;
		case VK_F9:         return KC::KC_F9;
		case VK_F10:        return KC::KC_F10;
		case VK_F11:        return KC::KC_F11;
		case VK_F12:        return KC::KC_F12;
		case VK_F13:        return KC::KC_F13;
		case VK_F14:        return KC::KC_F14;
		case VK_F15:        return KC::KC_F15;
		case VK_F16:        return KC::KC_F16;
		case VK_F17:        return KC::KC_F17;
		case VK_F18:        return KC::KC_F18;
		case VK_F19:        return KC::KC_F19;
		case VK_F20:        return KC::KC_F20;
		case VK_F21:        return KC::KC_F21;
		case VK_F22:        return KC::KC_F22;
		case VK_F23:        return KC::KC_F23;
		case VK_F24:        return KC::KC_F24;
		case VK_NUMLOCK:    return KC::KC_NUM_LOCK;
		case VK_SCROLL:     return KC::KC_SCROLL_LOCK;
		case VK_LSHIFT:     return KC::KC_LEFT_SHIFT;
		case VK_RSHIFT:     return KC::KC_RIGHT_SHIFT;
		case VK_LCONTROL:   return KC::KC_LEFT_CONTROL;
		case VK_RCONTROL:   return KC::KC_RIGHT_CONTROL;
		case VK_LMENU:      return KC::KC_LEFT_ALT;
		case VK_RMENU:      return KC::KC_RIGHT_ALT;
		case VK_OEM_1:      return KC::KC_SEMICOLON;
		case VK_OEM_PLUS:   return KC::KC_EQUAL;
		case VK_OEM_COMMA:  return KC::KC_COMMA;
		case VK_OEM_MINUS:  return KC::KC_MINUS;
		case VK_OEM_PERIOD: return KC::KC_PERIOD;
		case VK_OEM_2:      return KC::KC_SLASH;
		case VK_OEM_3:      return KC::KC_BACKTICK;
		case VK_OEM_4:      return KC::KC_LEFT_BRACKET;
		case VK_OEM_5:      return KC::KC_BACKSLASH;
		case VK_OEM_6:      return KC::KC_RIGHT_BRACKET;
		case VK_OEM_7:      return KC::KC_APOSTROPHE;
		default:            return KC::KC_UNKNOWN;
		}
	}

	uint32_t WindowMappings::toWin32RimType(Windows::RawInputDeviceType v_Type) {
		switch (v_Type) {
		case Windows::RawInputDeviceType::MOUSE:    return RIM_TYPEMOUSE;
		case Windows::RawInputDeviceType::KEYBOARD: return RIM_TYPEKEYBOARD;
		case Windows::RawInputDeviceType::HID:      return RIM_TYPEHID;
		}
		SPECTRA_UNREACHABLE();
	}

	Windows::RawInputDeviceType WindowMappings::fromWin32RimType(uint32_t v_Type) {
		switch (v_Type) {
		case RIM_TYPEMOUSE:    return Windows::RawInputDeviceType::MOUSE;
		case RIM_TYPEKEYBOARD: return Windows::RawInputDeviceType::KEYBOARD;
		case RIM_TYPEHID:      return Windows::RawInputDeviceType::HID;
		}
		SPECTRA_UNREACHABLE();
	}

	Windows::WindowVisualState WindowMappings::fromWin32SizeParam(uint32_t v_SizeParam) {
		switch (v_SizeParam) {
		case SIZE_RESTORED:  return Windows::WindowVisualState::RESTORED;
		case SIZE_MINIMIZED: return Windows::WindowVisualState::MINIMIZED;
		case SIZE_MAXIMIZED: return Windows::WindowVisualState::MAXIMIZED;
		}
		// SIZE_MAXSHOW / SIZE_MAXHIDE must be filtered by the caller before dispatch here.
		SPECTRA_UNREACHABLE();
	}

	int32_t WindowMappings::toWin32ShowCmd(Windows::WindowVisualState v_State) {
		switch (v_State) {
		case Windows::WindowVisualState::RESTORED:  return SW_RESTORE;
		case Windows::WindowVisualState::MINIMIZED: return SW_MINIMIZE;
		case Windows::WindowVisualState::MAXIMIZED: return SW_MAXIMIZE;
		case Windows::WindowVisualState::HIDDEN:    return SW_HIDE;
		}
		SPECTRA_UNREACHABLE();
	}
}
