#include "SpectraPlatformRuntime.h"
#include "PlatformWindowing.h"
#include "WindowUtils.h"

#define ALLOW_SYSCALL
#define WINDOW_BUILDER

#include "SpectraSyscalls.h"
#include "InternalUtils.h"

#include <cstring>

namespace Spectra::Platform::Runtime::Windows {
	constexpr size_t INVALID_WINDOW_SLOT = static_cast<size_t>(-1);
	constexpr size_t WINDOW_EVENT_QUEUE_CAPACITY = 512;
	constexpr size_t MAX_DROP_FILES = 32;

	struct WindowRecord;

	// WindowDropTargetImpl - the IDropTarget COM object for OLE drag-drop.
	// Embedded by value in WindowRecord (see below) rather than heap-allocated:
	// RegisterDragDrop/RevokeDragDrop just need AddRef/Release bookkeeping, not
	// real ownership, since the object's lifetime is tied to its owning window
	// slot in the fixed g_WindowRegistry array - Release() never frees anything.
	namespace Internal {
		struct WindowDropTargetImpl final : public IDropTarget {
			WindowRecord* m_Owner = nullptr;

			HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
				if (riid == __uuidof(IUnknown) || riid == __uuidof(IDropTarget)) {
					*ppv = this;
					AddRef();
					return S_OK;
				}
				*ppv = nullptr;
				return E_NOINTERFACE;
			}

			ULONG __stdcall AddRef() override {
				return static_cast<ULONG>(InterlockedIncrement(&m_RefCount));
			}

			ULONG __stdcall Release() override {
				return static_cast<ULONG>(InterlockedDecrement(&m_RefCount));
			}

			HRESULT __stdcall DragEnter(IDataObject* p_DataObj, DWORD v_KeyState, POINTL v_Pt, DWORD* p_Effect) override;
			HRESULT __stdcall DragOver(DWORD v_KeyState, POINTL v_Pt, DWORD* p_Effect) override;
			HRESULT __stdcall DragLeave() override;
			HRESULT __stdcall Drop(IDataObject* p_DataObj, DWORD v_KeyState, POINTL v_Pt, DWORD* p_Effect) override;

		private:
			LONG m_RefCount = 1;
			bool m_HasFiles = false;
		};
	}

	struct alignas(32) WindowRecord final {
		HWND m_Hwnd = nullptr;
		uint64_t m_Generation = 0;
		WindowDesc m_Desc;
		WindowEvent m_EventBuffer[WINDOW_EVENT_QUEUE_CAPACITY];
		size_t m_Head = 0;
		size_t m_Tail = 0;
		WindowLifecycle m_Lifecycle = WindowLifecycle::DESTROYED;

		EventForwardFn m_Forward = nullptr;
		void* m_ForwardContext = nullptr;

		HitTestFn m_HitTestCallback = nullptr;
		DropRegionFn m_DropRegionCallback = nullptr;

		// Only non-null once enableDragDrop() has been called for this window.
		Microsoft::WRL::ComPtr<IDropTarget> m_DropTarget;
		Internal::WindowDropTargetImpl m_DropTargetImpl;

		char m_DropPathStorage[MAX_DROP_FILES][MAX_PATH] = {};
		const char* m_DropPathPtrs[MAX_DROP_FILES] = {};
	};

	namespace {
		WindowRecord g_WindowRegistry[SPECTRA_PLATFORM_MAX_WINDOWS];

		size_t findFreeSlot() noexcept {
			for (size_t i = 0; i < SPECTRA_PLATFORM_MAX_WINDOWS; ++i) {
				if (g_WindowRegistry[i].m_Lifecycle == WindowLifecycle::DESTROYED) return i;
			}
			return INVALID_WINDOW_SLOT;
		}

		// Single-threaded ring buffer - both the writer (wndProcThunk, during the
		// UI thread's message pump) and the reader (PollEvent) run on the same
		// thread, so no atomics/ordering needed. Full buffer drops the oldest
		// event rather than the newest, since a lost mouse-delta is harmless but
		// a lost Close/Resize sitting behind a burst of input events isn't.
		void pushEvent(WindowRecord& record, const WindowEvent& event) noexcept {
			const size_t nextTail = (record.m_Tail + 1) % WINDOW_EVENT_QUEUE_CAPACITY;
			if (nextTail == record.m_Head) {
				record.m_Head = (record.m_Head + 1) % WINDOW_EVENT_QUEUE_CAPACITY;
			}
			record.m_EventBuffer[record.m_Tail] = event;
			record.m_Tail = nextTail;
		}

		bool hasFileDropFormat(IDataObject* p_DataObj) noexcept {
			FORMATETC format{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			return p_DataObj->QueryGetData(&format) == S_OK;
		}

		KeyModifierFlags currentKeyModifiers() noexcept {
			KeyModifierFlags mods = KeyModifierFlags::NONE;
			if (GetKeyState(VK_SHIFT)    & 0x8000) mods |= KeyModifierFlags::SHIFT;
			if (GetKeyState(VK_CONTROL)  & 0x8000) mods |= KeyModifierFlags::CONTROL;
			if (GetKeyState(VK_MENU)     & 0x8000) mods |= KeyModifierFlags::ALT;
			if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= KeyModifierFlags::META;
			if (GetKeyState(VK_CAPITAL)  & 0x0001) mods |= KeyModifierFlags::CAPS_LOCK;
			if (GetKeyState(VK_NUMLOCK)  & 0x0001) mods |= KeyModifierFlags::NUM_LOCK;
			if (GetKeyState(VK_LSHIFT)   & 0x8000) mods |= KeyModifierFlags::LEFT_SHIFT;
			if (GetKeyState(VK_RSHIFT)   & 0x8000) mods |= KeyModifierFlags::RIGHT_SHIFT;
			if (GetKeyState(VK_LCONTROL) & 0x8000) mods |= KeyModifierFlags::LEFT_CONTROL;
			if (GetKeyState(VK_RCONTROL) & 0x8000) mods |= KeyModifierFlags::RIGHT_CONTROL;
			if (GetKeyState(VK_LMENU)    & 0x8000) mods |= KeyModifierFlags::LEFT_ALT;
			if (GetKeyState(VK_RMENU)    & 0x8000) mods |= KeyModifierFlags::RIGHT_ALT;
			if (GetKeyState(VK_LWIN)     & 0x8000) mods |= KeyModifierFlags::LEFT_META;
			if (GetKeyState(VK_RWIN)     & 0x8000) mods |= KeyModifierFlags::RIGHT_META;
			return mods;
		}

		// WM_CHAR delivers UTF-16 code units one at a time; astral codepoints (surrogate
		// pairs) need the high half held here until the matching low half arrives. UI-thread-only,
		// same as the event ring buffer, so a plain static is safe.
		wchar_t g_PendingHighSurrogate = 0;
	}

	bool PlatformWindow::isValidHandle(const WindowHandle& ro_Handle) noexcept {
		const size_t slot = ro_Handle.m_Index;
		if (slot >= SPECTRA_PLATFORM_MAX_WINDOWS) return false;

		const WindowRecord& record = g_WindowRegistry[slot];
		if (record.m_Generation != ro_Handle.m_Generation) return false;
		if (record.m_Lifecycle == WindowLifecycle::DESTROYED) return false;
		return true;
	}

	HRESULT __stdcall Internal::WindowDropTargetImpl::DragEnter(IDataObject* p_DataObj, DWORD, POINTL v_Pt, DWORD* p_Effect) {
		m_HasFiles = hasFileDropFormat(p_DataObj);
		return DragOver(0, v_Pt, p_Effect);
	}

	HRESULT __stdcall Internal::WindowDropTargetImpl::DragOver(DWORD, POINTL v_Pt, DWORD* p_Effect) {
		if (!m_HasFiles || !m_Owner) {
			*p_Effect = DROPEFFECT_NONE;
			return S_OK;
		}

		POINT pt{ v_Pt.x, v_Pt.y };
		ScreenToClient(m_Owner->m_Hwnd, &pt);

		const bool accept = !m_Owner->m_DropRegionCallback || m_Owner->m_DropRegionCallback(pt.x, pt.y);
		*p_Effect = accept ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		return S_OK;
	}

	HRESULT __stdcall Internal::WindowDropTargetImpl::DragLeave() {
		m_HasFiles = false;
		return S_OK;
	}

	HRESULT __stdcall Internal::WindowDropTargetImpl::Drop(IDataObject* p_DataObj, DWORD, POINTL v_Pt, DWORD* p_Effect) {
		if (!m_Owner) {
			*p_Effect = DROPEFFECT_NONE;
			return S_OK;
		}

		POINT pt{ v_Pt.x, v_Pt.y };
		ScreenToClient(m_Owner->m_Hwnd, &pt);

		uint32_t fileCount = 0;
		FORMATETC format{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM medium{};

		if (SUCCEEDED(p_DataObj->GetData(&format, &medium))) {
			auto hDrop = static_cast<HDROP>(medium.hGlobal);
			const UINT total = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
			fileCount = (total < MAX_DROP_FILES) ? total : MAX_DROP_FILES;

			for (uint32_t i = 0; i < fileCount; ++i) {
				wchar_t wide[MAX_PATH];
				DragQueryFileW(hDrop, i, wide, MAX_PATH);
				WideCharToMultiByte(CP_UTF8, 0, wide, -1, m_Owner->m_DropPathStorage[i], MAX_PATH, nullptr, nullptr);
				m_Owner->m_DropPathPtrs[i] = m_Owner->m_DropPathStorage[i];
			}
			ReleaseStgMedium(&medium);
		}

		const size_t slot = static_cast<size_t>(m_Owner - g_WindowRegistry);
		const WindowHandle handle(slot, m_Owner->m_Generation);
		pushEvent(*m_Owner, makeDropEvent(handle, pt.x, pt.y, fileCount, m_Owner->m_DropPathPtrs));

		m_HasFiles = false;
		*p_Effect = DROPEFFECT_COPY;
		return S_OK;
	}

	// PlatformWindowThunkHelper - owns the Win32 WndProc so HWND/LRESULT/etc.
	// never appear in the public header. Resolves HWND -> WindowRecord* via
	// GWLP_USERDATA and forwards to WindowRecord::HandleMessage.

	namespace Internal {
		struct PlatformWindowThunkHelper final {
			static LRESULT CALLBACK wndProcThunk(HWND v_Hwnd, UINT v_Msg, WPARAM v_WParam, LPARAM v_LParam) noexcept;
		};
	}

	LRESULT CALLBACK Internal::PlatformWindowThunkHelper::wndProcThunk(HWND v_Hwnd, UINT v_Msg, WPARAM v_WParam, LPARAM v_LParam) noexcept {
		WindowRecord* record;

		if (v_Msg == WM_NCCREATE) {
			auto* createStruct = std::bit_cast<CREATESTRUCTW*>(v_LParam);
			record = static_cast<WindowRecord*>(createStruct->lpCreateParams);
			SetWindowLongPtrW(v_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(record));
			record->m_Hwnd = v_Hwnd;
		} else {
			record = std::bit_cast<WindowRecord*>(GetWindowLongPtrW(v_Hwnd, GWLP_USERDATA));
		}

		if (!record) return DefWindowProcW(v_Hwnd, v_Msg, v_WParam, v_LParam);

		const size_t slot = static_cast<size_t>(record - g_WindowRegistry);
		const WindowHandle handle(slot, record->m_Generation);

		// WM_CLOSE is swallowed by design - DefWindowProcW's default handling
		// would call DestroyWindow itself, but Window Core requires an explicit
		// PlatformWindow::destroy() call instead. Queue Close and stop here.
		if (v_Msg == WM_CLOSE) {
			pushEvent(*record, makeCloseEvent(handle));
			return 0;
		}

		// WM_NCHITTEST is the other synchronous exception (besides WM_CLOSE) to the
		// poll-only event queue - answered inline via the callback, never queued.
		if (v_Msg == WM_NCHITTEST && record->m_HitTestCallback) {
			POINT pt{ GET_X_LPARAM(v_LParam), GET_Y_LPARAM(v_LParam) };
			ScreenToClient(v_Hwnd, &pt);
			return Runtime::Internal::WindowMappings::toWin32HitTest(record->m_HitTestCallback(pt.x, pt.y));
		}

		if (v_Msg == WM_SIZE) {
			if (v_WParam == SIZE_RESTORED || v_WParam == SIZE_MINIMIZED || v_WParam == SIZE_MAXIMIZED) {
				const WindowGeometry size{ static_cast<uint32_t>(LOWORD(v_LParam)), static_cast<uint32_t>(HIWORD(v_LParam)) };
				pushEvent(*record, makeResizeEvent(handle, size, Runtime::Internal::WindowMappings::fromWin32SizeParam(static_cast<uint32_t>(v_WParam))));
			}
			// SIZE_MAXSHOW/SIZE_MAXHIDE carry no geometry change worth surfacing - ignored.
		} else if (v_Msg == WM_MOVE) {
			pushEvent(*record, makeMoveEvent(handle, GET_X_LPARAM(v_LParam), GET_Y_LPARAM(v_LParam)));
		} else if (v_Msg == WM_SETFOCUS) {
			pushEvent(*record, makeFocusGainedEvent(handle));
		} else if (v_Msg == WM_KILLFOCUS) {
			pushEvent(*record, makeFocusLostEvent(handle));
		} else if (v_Msg == WM_KEYDOWN || v_Msg == WM_SYSKEYDOWN || v_Msg == WM_KEYUP || v_Msg == WM_SYSKEYUP) {
			const bool isDown = (v_Msg == WM_KEYDOWN || v_Msg == WM_SYSKEYDOWN);
			const bool isRepeat = isDown && ((v_LParam & (1 << 30)) != 0);
			const KeyCode code = Runtime::Internal::WindowMappings::fromWin32VKey(static_cast<uint16_t>(v_WParam));
			const WindowEventType type = isDown ? WindowEventType::KEY_DOWN : WindowEventType::KEY_UP;
			pushEvent(*record, makeKeyEvent(handle, type, code, currentKeyModifiers(), isRepeat));
			// Falls through to DefWindowProcW below so system accelerators (Alt+F4, menu mnemonics) still work.
		} else if (v_Msg == WM_CHAR) {
			const auto codeUnit = static_cast<wchar_t>(v_WParam);
			if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF) {
				g_PendingHighSurrogate = codeUnit;
			} else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF) {
				if (g_PendingHighSurrogate != 0) {
					const char32_t codepoint = 0x10000 + ((static_cast<char32_t>(g_PendingHighSurrogate) - 0xD800) << 10) + (static_cast<char32_t>(codeUnit) - 0xDC00);
					pushEvent(*record, makeTextInputEvent(handle, codepoint));
				}
				g_PendingHighSurrogate = 0;
			} else {
				pushEvent(*record, makeTextInputEvent(handle, static_cast<char32_t>(codeUnit)));
			}
		} else if (v_Msg == WM_MOUSEMOVE) {
			pushEvent(*record, makeMouseMoveEvent(handle, GET_X_LPARAM(v_LParam), GET_Y_LPARAM(v_LParam)));
		} else if (v_Msg == WM_LBUTTONDOWN || v_Msg == WM_LBUTTONUP || v_Msg == WM_RBUTTONDOWN || v_Msg == WM_RBUTTONUP ||
			v_Msg == WM_MBUTTONDOWN || v_Msg == WM_MBUTTONUP || v_Msg == WM_XBUTTONDOWN || v_Msg == WM_XBUTTONUP) {
			const bool isDown = (v_Msg == WM_LBUTTONDOWN || v_Msg == WM_RBUTTONDOWN || v_Msg == WM_MBUTTONDOWN || v_Msg == WM_XBUTTONDOWN);
			const WindowEventType type = isDown ? WindowEventType::MOUSE_BUTTON_DOWN : WindowEventType::MOUSE_BUTTON_UP;

			MouseButton button;
			if (v_Msg == WM_LBUTTONDOWN || v_Msg == WM_LBUTTONUP) button = MouseButton::LEFT;
			else if (v_Msg == WM_RBUTTONDOWN || v_Msg == WM_RBUTTONUP) button = MouseButton::RIGHT;
			else if (v_Msg == WM_MBUTTONDOWN || v_Msg == WM_MBUTTONUP) button = MouseButton::MIDDLE;
			else button = (GET_XBUTTON_WPARAM(v_WParam) == XBUTTON1) ? MouseButton::X1 : MouseButton::X2;

			pushEvent(*record, makeMouseButtonEvent(handle, type, button, GET_X_LPARAM(v_LParam), GET_Y_LPARAM(v_LParam)));

			// MSDN: an app that processes WM_XBUTTONDOWN/UP should return TRUE.
			if (v_Msg == WM_XBUTTONDOWN || v_Msg == WM_XBUTTONUP) return TRUE;
		} else if (v_Msg == WM_MOUSEWHEEL) {
			POINT pt{ GET_X_LPARAM(v_LParam), GET_Y_LPARAM(v_LParam) }; // WM_MOUSEWHEEL delivers screen coords, unlike every other mouse message
			ScreenToClient(v_Hwnd, &pt);
			pushEvent(*record, makeMouseWheelEvent(handle, GET_WHEEL_DELTA_WPARAM(v_WParam), pt.x, pt.y));
		} else if (v_Msg == WM_INPUT) {
			UINT size = 0;
			GetRawInputData(std::bit_cast<HRAWINPUT>(v_LParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
			if (size > 0 && size <= sizeof(RAWINPUT)) {
				RAWINPUT raw{};
				if (GetRawInputData(std::bit_cast<HRAWINPUT>(v_LParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == size) {
					if (raw.header.dwType == RIM_TYPEMOUSE) {
						RawMouseDelta delta{};
						delta.m_DeltaX = raw.data.mouse.lLastX;
						delta.m_DeltaY = raw.data.mouse.lLastY;
						delta.m_ButtonFlags = raw.data.mouse.usButtonFlags;
						delta.m_WheelDelta = (raw.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) ? static_cast<int16_t>(raw.data.mouse.usButtonData) : 0;
						pushEvent(*record, makeRawInputEvent(handle, delta));
					} else if (raw.header.dwType == RIM_TYPEKEYBOARD) {
						RawKeyEvent key{};
						key.m_ScanCode = raw.data.keyboard.MakeCode;
						key.m_Code = Runtime::Internal::WindowMappings::fromWin32VKey(raw.data.keyboard.VKey);
						key.m_IsDown = (raw.data.keyboard.Flags & RI_KEY_BREAK) == 0;
						key.m_IsExtended = (raw.data.keyboard.Flags & RI_KEY_E0) != 0;
						pushEvent(*record, makeRawInputEvent(handle, key));
					}
				}
			}
			// MSDN: the app must still call DefWindowProc for WM_INPUT so the system can clean up.
			return DefWindowProcW(v_Hwnd, v_Msg, v_WParam, v_LParam);
		} else if (v_Msg == WM_INPUT_DEVICE_CHANGE) {
			auto deviceHandle = std::bit_cast<HANDLE>(v_LParam);

			RID_DEVICE_INFO info{};
			info.cbSize = sizeof(RID_DEVICE_INFO);
			UINT infoSize = sizeof(RID_DEVICE_INFO);
			RawInputDeviceType deviceType = RawInputDeviceType::HID;
			if (GetRawInputDeviceInfoW(deviceHandle, RIDI_DEVICEINFO, &info, &infoSize) > 0) {
				deviceType = Runtime::Internal::WindowMappings::fromWin32RimType(info.dwType);
			}

			const WindowEventType type = (v_WParam == GIDC_ARRIVAL) ? WindowEventType::RAW_INPUT_DEVICE_ARRIVAL : WindowEventType::RAW_INPUT_DEVICE_REMOVAL;
			pushEvent(*record, makeRawInputDeviceEvent(handle, type, RawInputDeviceHandle(deviceHandle), deviceType));
		} else if (v_Msg == WM_DISPLAYCHANGE) {
			pushEvent(*record, makeDisplayChangedEvent(handle, DisplayHandle(MonitorFromWindow(v_Hwnd, MONITOR_DEFAULTTONEAREST))));
		} else if (v_Msg == WM_DPICHANGED) {
			pushEvent(*record, makeDpiChangedEvent(handle, LOWORD(v_WParam)));

			// Recommended MSDN handling: reposition/resize to the suggested rect so the
			// window stays visually anchored (same screen-space size) across the DPI change.
			const auto* suggestedRect = std::bit_cast<RECT*>(v_LParam);
			SetWindowPos(v_Hwnd, nullptr, suggestedRect->left, suggestedRect->top,
				suggestedRect->right - suggestedRect->left, suggestedRect->bottom - suggestedRect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			return 0;
		}

		// TODO: WM_DROPFILES / OLE drag-drop translation into DropEvent - blocked on
		// EnableDragDrop's IDropTarget implementation.

		if (v_Msg == WM_NCDESTROY) {
			SetWindowLongPtrW(v_Hwnd, GWLP_USERDATA, 0);
		}

		return DefWindowProcW(v_Hwnd, v_Msg, v_WParam, v_LParam);
	}

	namespace {
		constexpr const wchar_t* WINDOW_CLASS_NAME = L"SpectraWindowClass";
		bool g_IsInitialized = false;
	}

	void PlatformWindow::init() {
		if (g_IsInitialized) return;

		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = Internal::PlatformWindowThunkHelper::wndProcThunk;
		windowClass.hInstance = GetModuleHandleW(nullptr);
		windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, forced wide - the project doesn't define UNICODE globally so the plain macro resolves ANSI
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		windowClass.lpszClassName = WINDOW_CLASS_NAME;
		RegisterClassExW(&windowClass);

		OleInitialize(nullptr);

		g_IsInitialized = true;
	}

	void PlatformWindow::shutdown() {
		if (!g_IsInitialized) return;

		OleUninitialize();
		UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandleW(nullptr));

		g_IsInitialized = false;
	}

	WindowHandle PlatformWindow::create(const WindowDesc& ro_Desc, EventForwardFn v_Forward, void* p_ForwardContext) {
		const size_t slot = findFreeSlot();
		if (slot == INVALID_WINDOW_SLOT) return WindowHandle::getInvalidHandle();

		WindowRecord& record = g_WindowRegistry[slot];
		record.m_Desc = ro_Desc;
		record.m_Forward = v_Forward;
		record.m_ForwardContext = p_ForwardContext;

		const uint32_t style = Runtime::Internal::WindowMappings::toWin32Style(ro_Desc.m_StyleFlags);
		const uint32_t exStyle = Runtime::Internal::WindowMappings::toWin32ExStyle(ro_Desc.m_StyleFlags);

		RECT rect{ 0, 0, static_cast<LONG>(ro_Desc.m_Width), static_cast<LONG>(ro_Desc.m_Height) };
		AdjustWindowRectEx(&rect, style, FALSE, exStyle);
		const int32_t width = rect.right - rect.left;
		const int32_t height = rect.bottom - rect.top;

		const int32_t x = (ro_Desc.m_X == WindowDesc::POSITION_DEFAULT) ? CW_USEDEFAULT : ro_Desc.m_X;
		const int32_t y = (ro_Desc.m_Y == WindowDesc::POSITION_DEFAULT) ? CW_USEDEFAULT : ro_Desc.m_Y;

		HWND parentHwnd = isValidHandle(ro_Desc.m_Parent) ? g_WindowRegistry[ro_Desc.m_Parent.m_Index].m_Hwnd : nullptr;

		// Title source is capped to 255 UTF-8 bytes before conversion (not the
		// destination capacity) - each output UTF-16 unit consumes at least one
		// source byte, so this guarantees the conversion always fits without
		// MultiByteToWideChar failing on ERROR_INSUFFICIENT_BUFFER. Long titles
		// are silently truncated rather than failing window creation outright.
		constexpr size_t TITLE_BUFFER_CAPACITY = 256;
		wchar_t titleBuffer[TITLE_BUFFER_CAPACITY] = {};
		const char* title = ro_Desc.m_Title ? ro_Desc.m_Title : "";
		const size_t titleByteLen = strnlen(title, TITLE_BUFFER_CAPACITY - 1);
		const int converted = MultiByteToWideChar(CP_UTF8, 0, title, static_cast<int>(titleByteLen), titleBuffer, TITLE_BUFFER_CAPACITY - 1);
		titleBuffer[(converted > 0) ? converted : 0] = L'\0';

		HWND hwnd = CreateWindowExW(
			exStyle, WINDOW_CLASS_NAME, titleBuffer, style,
			x, y, width, height,
			parentHwnd, nullptr, GetModuleHandleW(nullptr), &record);

		if (!hwnd) return WindowHandle::getInvalidHandle();

		record.m_Lifecycle = WindowLifecycle::CREATED;
		return WindowHandle(slot, record.m_Generation);
	}

	void PlatformWindow::show(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;

		WindowRecord& record = g_WindowRegistry[ro_Handle.m_Index];
		ShowWindow(record.m_Hwnd, SW_SHOW);
		record.m_Lifecycle = WindowLifecycle::SHOWN;
	}

	void PlatformWindow::destroy(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;

		WindowRecord& record = g_WindowRegistry[ro_Handle.m_Index];

		if (record.m_DropTarget) {
			RevokeDragDrop(record.m_Hwnd);
			record.m_DropTarget.Reset();
			record.m_DropTargetImpl.m_Owner = nullptr;
		}

		if (record.m_Hwnd) DestroyWindow(record.m_Hwnd);

		record.m_Hwnd = nullptr;
		record.m_Lifecycle = WindowLifecycle::DESTROYED;
		record.m_Generation++;
		record.m_Head = 0;
		record.m_Tail = 0;
	}

	void PlatformWindow::setHitTestCallback(const WindowHandle& ro_Handle, HitTestFn v_Callback) noexcept {
		if (!isValidHandle(ro_Handle)) return;
		g_WindowRegistry[ro_Handle.m_Index].m_HitTestCallback = v_Callback;
	}

	void PlatformWindow::setDropRegionCallback(const WindowHandle& ro_Handle, DropRegionFn v_Callback) noexcept {
		if (!isValidHandle(ro_Handle)) return;
		g_WindowRegistry[ro_Handle.m_Index].m_DropRegionCallback = v_Callback;
	}

	void PlatformWindow::enableDragDrop(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;

		WindowRecord& record = g_WindowRegistry[ro_Handle.m_Index];
		if (record.m_DropTarget) return; // already enabled for this window

		record.m_DropTargetImpl.m_Owner = &record;
		record.m_DropTarget = &record.m_DropTargetImpl;
		RegisterDragDrop(record.m_Hwnd, record.m_DropTarget.Get());
	}

	bool PlatformWindow::pollEvent(const WindowHandle& ro_Handle, WindowEvent& ro_Event) noexcept {
		if (!isValidHandle(ro_Handle)) return false;

		WindowRecord& record = g_WindowRegistry[ro_Handle.m_Index];
		if (record.m_Head == record.m_Tail) return false;

		ro_Event = record.m_EventBuffer[record.m_Head];
		record.m_Head = (record.m_Head + 1) % WINDOW_EVENT_QUEUE_CAPACITY;

		if (record.m_Forward) record.m_Forward(record.m_ForwardContext, ro_Event);
		return true;
	}

	void PlatformWindow::minimize(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;
		ShowWindow(g_WindowRegistry[ro_Handle.m_Index].m_Hwnd, Runtime::Internal::WindowMappings::toWin32ShowCmd(WindowVisualState::MINIMIZED));
	}

	void PlatformWindow::maximize(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;
		ShowWindow(g_WindowRegistry[ro_Handle.m_Index].m_Hwnd, Runtime::Internal::WindowMappings::toWin32ShowCmd(WindowVisualState::MAXIMIZED));
	}

	void PlatformWindow::restore(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return;
		ShowWindow(g_WindowRegistry[ro_Handle.m_Index].m_Hwnd, Runtime::Internal::WindowMappings::toWin32ShowCmd(WindowVisualState::RESTORED));
	}

	bool PlatformWindow::isMinimized(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
		return IsIconic(g_WindowRegistry[ro_Handle.m_Index].m_Hwnd) != 0;
	}

	bool PlatformWindow::isMaximized(const WindowHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
		return IsZoomed(g_WindowRegistry[ro_Handle.m_Index].m_Hwnd) != 0;
	}

	namespace {
		struct EnumDisplaysContext final {
			DisplayHandle* m_Out;
			size_t m_Capacity;
			size_t m_Count = 0;
		};
	}

	// PlatformDisplayEnumHelper - only PlatformDisplay/this helper may construct
	// a DisplayHandle, so the EnumDisplayMonitors callback (which must match
	// MONITORENUMPROC's exact signature, HMONITOR/HDC/LPRECT/LPARAM and all)
	// has to live here rather than as a free function - same reason
	// wndProcThunk lives on PlatformWindowThunkHelper instead of file scope.
	namespace Internal {
		struct PlatformDisplayEnumHelper final {
			static BOOL CALLBACK enumMonitorsProc(HMONITOR v_Monitor, HDC, LPRECT, LPARAM v_LParam);
		};
	}

	BOOL CALLBACK Internal::PlatformDisplayEnumHelper::enumMonitorsProc(HMONITOR v_Monitor, HDC, LPRECT, LPARAM v_LParam) {
		auto* context = std::bit_cast<EnumDisplaysContext*>(v_LParam);
		if (context->m_Out && context->m_Count < context->m_Capacity) {
			context->m_Out[context->m_Count] = DisplayHandle(v_Monitor);
		}
		++context->m_Count;
		return TRUE;
	}

	size_t PlatformDisplay::enumerateDisplays(DisplayHandle* p_Out, size_t v_Capacity) {
		EnumDisplaysContext context{ p_Out, v_Capacity };
		EnumDisplayMonitors(nullptr, nullptr, Internal::PlatformDisplayEnumHelper::enumMonitorsProc, std::bit_cast<LPARAM>(&context));
		return context.m_Count;
	}

	DisplayInfo PlatformDisplay::getDisplayInfo(const DisplayHandle& ro_Display) {
		DisplayInfo info{};

		auto monitor = static_cast<HMONITOR>(ro_Display.m_HMonitor);

		MONITORINFOEXW monitorInfo{};
		monitorInfo.cbSize = sizeof(MONITORINFOEXW);
		if (!GetMonitorInfoW(monitor, &monitorInfo)) return info;

		info.m_Bounds.m_Left = monitorInfo.rcMonitor.left;
		info.m_Bounds.m_Top = monitorInfo.rcMonitor.top;
		info.m_Bounds.m_Right = monitorInfo.rcMonitor.right;
		info.m_Bounds.m_Bottom = monitorInfo.rcMonitor.bottom;

		info.m_WorkArea.m_Left = monitorInfo.rcWork.left;
		info.m_WorkArea.m_Top = monitorInfo.rcWork.top;
		info.m_WorkArea.m_Right = monitorInfo.rcWork.right;
		info.m_WorkArea.m_Bottom = monitorInfo.rcWork.bottom;

		info.m_IsPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;

		UINT dpiX = 0, dpiY = 0;
		GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		info.m_Dpi = dpiX;

		WideCharToMultiByte(CP_UTF8, 0, monitorInfo.szDevice, -1, info.m_DeviceName, sizeof(info.m_DeviceName), nullptr, nullptr);

		return info;
	}

	void PlatformInput::registerDevices(const RawInputDeviceDesc* p_Devices, size_t v_Count) {
		if (!p_Devices || v_Count == 0) return;

		// Only two device classes are ever registerable (mouse/keyboard) - HID is
		// enumeration-only - so a small fixed cap is generous, not a real limit.
		RAWINPUTDEVICE rawDevices[8];
		size_t deviceCount = 0;

		for (size_t i = 0; i < v_Count && deviceCount < 8; ++i) {
			const RawInputDeviceDesc& desc = p_Devices[i];

			uint16_t usage;
			switch (desc.m_DeviceType) {
			case RawInputDeviceType::MOUSE:    usage = 0x02; break;
			case RawInputDeviceType::KEYBOARD: usage = 0x06; break;
			default: continue; // HID is enumeration-only, not registerable via RIDEV
			}

			DWORD flags = 0;
			if (desc.m_BackgroundInput) flags |= RIDEV_INPUTSINK;
			if (desc.m_NotifyDeviceChanges) flags |= RIDEV_DEVNOTIFY;

			HWND target = PlatformWindow::isValidHandle(desc.m_Target)
				? g_WindowRegistry[desc.m_Target.m_Index].m_Hwnd
				: nullptr;

			rawDevices[deviceCount].usUsagePage = 0x01;
			rawDevices[deviceCount].usUsage = usage;
			rawDevices[deviceCount].dwFlags = flags;
			rawDevices[deviceCount].hwndTarget = target;
			++deviceCount;
		}

		if (deviceCount > 0) {
			RegisterRawInputDevices(rawDevices, static_cast<UINT>(deviceCount), sizeof(RAWINPUTDEVICE));
		}
	}

	size_t PlatformInput::enumerateDevices(RawInputDeviceInfo* p_Out, size_t v_Capacity) {
		UINT deviceCount = 0;
		GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST));
		if (deviceCount == 0) return 0;

		// Stack buffer - attached-device counts are small in practice.
		RAWINPUTDEVICELIST deviceList[64];
		if (deviceCount > 64) deviceCount = 64;

		const UINT gotCount = GetRawInputDeviceList(deviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));
		if (gotCount == static_cast<UINT>(-1)) return 0;

		size_t outCount = 0;
		for (UINT i = 0; i < gotCount; ++i) {
			RawInputDeviceInfo info{};
			info.m_Handle = RawInputDeviceHandle(deviceList[i].hDevice);
			info.m_DeviceType = Runtime::Internal::WindowMappings::fromWin32RimType(deviceList[i].dwType);

			UINT pathSize = sizeof(info.m_DevicePath);
			GetRawInputDeviceInfoA(deviceList[i].hDevice, RIDI_DEVICENAME, info.m_DevicePath, &pathSize);

			if (deviceList[i].dwType == RIM_TYPEHID) {
				RID_DEVICE_INFO hidInfo{};
				hidInfo.cbSize = sizeof(RID_DEVICE_INFO);
				UINT hidInfoSize = sizeof(RID_DEVICE_INFO);
				if (GetRawInputDeviceInfoA(deviceList[i].hDevice, RIDI_DEVICEINFO, &hidInfo, &hidInfoSize) > 0) {
					info.m_VendorId = static_cast<uint16_t>(hidInfo.hid.dwVendorId);
					info.m_ProductId = static_cast<uint16_t>(hidInfo.hid.dwProductId);
					info.m_UsagePage = hidInfo.hid.usUsagePage;
					info.m_Usage = hidInfo.hid.usUsage;
				}
			}

			if (p_Out && outCount < v_Capacity) {
				p_Out[outCount] = info;
			}
			++outCount;
		}

		return outCount;
	}
}