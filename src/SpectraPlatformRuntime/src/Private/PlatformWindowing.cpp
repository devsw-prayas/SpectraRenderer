#include "SpectraPlatformRuntime.h"
#include "PlatformWindowing.h"
#include "WindowUtils.h"

#define ALLOW_SYSCALL
#define WINDOW_BUILDER

#include "SpectraSyscalls.h"
#include "InternalUtils.h"

namespace Spectra::Platform::Runtime::Windows {
	constexpr size_t INVALID_WINDOW_SLOT = static_cast<size_t>(-1);
	constexpr size_t WINDOW_EVENT_QUEUE_CAPACITY = 512;

	using EventForwardFn = void(*)(void* p_Context, const WindowEvent& ro_Event);

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

		// Only non-null once EnableDragDrop() has been called for this window.
		Microsoft::WRL::ComPtr<IDropTarget> m_DropTarget;
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
	}

	bool PlatformWindow::isValidHandle(const WindowHandle& ro_Handle) noexcept {
		const size_t slot = ro_Handle.m_Index;
		if (slot >= SPECTRA_PLATFORM_MAX_WINDOWS) return false;

		const WindowRecord& record = g_WindowRegistry[slot];
		if (record.m_Generation != ro_Handle.m_Generation) return false;
		if (record.m_Lifecycle == WindowLifecycle::DESTROYED) return false;
		return true;
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

		// TODO: translate the remaining message types into WindowEvents via the
		// makeXEvent builders, push onto record's ring buffer, call
		// record->m_Forward if set.

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

	WindowHandle PlatformWindow::create(const WindowDesc& ro_Desc) {
		const size_t slot = findFreeSlot();
		if (slot == INVALID_WINDOW_SLOT) return WindowHandle::getInvalidHandle();

		WindowRecord& record = g_WindowRegistry[slot];
		record.m_Desc = ro_Desc;

		const uint32_t style = Runtime::Internal::WindowMappings::toWin32Style(ro_Desc.m_StyleFlags);
		const uint32_t exStyle = Runtime::Internal::WindowMappings::toWin32ExStyle(ro_Desc.m_StyleFlags);

		RECT rect{ 0, 0, static_cast<LONG>(ro_Desc.m_Width), static_cast<LONG>(ro_Desc.m_Height) };
		AdjustWindowRectEx(&rect, style, FALSE, exStyle);
		const int32_t width = rect.right - rect.left;
		const int32_t height = rect.bottom - rect.top;

		const int32_t x = (ro_Desc.m_X == WindowDesc::POSITION_DEFAULT) ? CW_USEDEFAULT : ro_Desc.m_X;
		const int32_t y = (ro_Desc.m_Y == WindowDesc::POSITION_DEFAULT) ? CW_USEDEFAULT : ro_Desc.m_Y;

		// TODO: resolve ro_Desc.m_Parent -> HWND once window-to-window parenting is needed.
		HWND parentHwnd = nullptr;

		wchar_t titleBuffer[256] = {};
		MultiByteToWideChar(CP_UTF8, 0, ro_Desc.m_Title ? ro_Desc.m_Title : "", -1, titleBuffer, 256);

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