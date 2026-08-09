#include "SpectraPlatformRuntime.h"
#include "WindowUtils.h"

namespace Spectra::Platform::Runtime::Windows {
	WindowEvent makeCloseEvent(WindowHandle v_Handle) {
		WindowEvent event;
		event.m_Type = WindowEventType::CLOSE;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		return event;
	}

	WindowEvent makeResizeEvent(WindowHandle v_Handle, WindowGeometry v_Size, WindowVisualState v_State) {
		WindowEvent event;
		event.m_Type = WindowEventType::RESIZE;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_Resize.m_Size = v_Size;
		event.m_Resize.m_State = v_State;
		return event;
	}

	WindowEvent makeMoveEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y) {
		WindowEvent event;
		event.m_Type = WindowEventType::MOVE;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_Move.m_X = v_X;
		event.m_Move.m_Y = v_Y;
		return event;
	}

	WindowEvent makeFocusGainedEvent(WindowHandle v_Handle) {
		WindowEvent event;
		event.m_Type = WindowEventType::FOCUS_GAINED;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		return event;
	}

	WindowEvent makeFocusLostEvent(WindowHandle v_Handle) {
		WindowEvent event;
		event.m_Type = WindowEventType::FOCUS_LOST;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		return event;
	}

	WindowEvent makeKeyEvent(WindowHandle v_Handle, WindowEventType v_Type, KeyCode v_Code, KeyModifierFlags v_Modifiers, bool v_IsRepeat) {
		WindowEvent event;
		event.m_Type = v_Type;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_Key.m_Code = v_Code;
		event.m_Key.m_Modifiers = v_Modifiers;
		event.m_Key.m_IsRepeat = v_IsRepeat;
		return event;
	}

	WindowEvent makeTextInputEvent(WindowHandle v_Handle, char32_t v_Codepoint) {
		WindowEvent event;
		event.m_Type = WindowEventType::TEXT_INPUT;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_TextInput.m_Codepoint = v_Codepoint;
		return event;
	}

	WindowEvent makeMouseMoveEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y) {
		WindowEvent event;
		event.m_Type = WindowEventType::MOUSE_MOVE;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_MouseMove.m_X = v_X;
		event.m_MouseMove.m_Y = v_Y;
		return event;
	}

	WindowEvent makeMouseButtonEvent(WindowHandle v_Handle, WindowEventType v_Type, MouseButton v_Button, int32_t v_X, int32_t v_Y) {
		WindowEvent event;
		event.m_Type = v_Type;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_MouseButton.m_Button = v_Button;
		event.m_MouseButton.m_X = v_X;
		event.m_MouseButton.m_Y = v_Y;
		return event;
	}

	WindowEvent makeMouseWheelEvent(WindowHandle v_Handle, int32_t v_Delta, int32_t v_X, int32_t v_Y) {
		WindowEvent event;
		event.m_Type = WindowEventType::MOUSE_WHEEL;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_MouseWheel.m_Delta = v_Delta;
		event.m_MouseWheel.m_X = v_X;
		event.m_MouseWheel.m_Y = v_Y;
		return event;
	}

	WindowEvent makeRawInputEvent(WindowHandle v_Handle, const RawMouseDelta& ro_Delta) {
		WindowEvent event;
		event.m_Type = WindowEventType::RAW_INPUT;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_RawInput.m_DeviceType = RawInputDeviceType::MOUSE;
		event.m_RawInput.m_Mouse = ro_Delta;
		return event;
	}

	WindowEvent makeRawInputEvent(WindowHandle v_Handle, const RawKeyEvent& ro_Key) {
		WindowEvent event;
		event.m_Type = WindowEventType::RAW_INPUT;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_RawInput.m_DeviceType = RawInputDeviceType::KEYBOARD;
		event.m_RawInput.m_Key = ro_Key;
		return event;
	}

	WindowEvent makeDisplayChangedEvent(WindowHandle v_Handle, DisplayHandle v_Display) {
		WindowEvent event;
		event.m_Type = WindowEventType::DISPLAY_CHANGED;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_DisplayChanged.m_Display = v_Display;
		return event;
	}

	WindowEvent makeDpiChangedEvent(WindowHandle v_Handle, uint32_t v_Dpi) {
		WindowEvent event;
		event.m_Type = WindowEventType::DPI_CHANGED;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_DpiChanged.m_Dpi = v_Dpi;
		return event;
	}

	WindowEvent makeRawInputDeviceEvent(WindowHandle v_Handle, WindowEventType v_Type, RawInputDeviceHandle v_Device, RawInputDeviceType v_DeviceType) {
		WindowEvent event;
		event.m_Type = v_Type;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_RawInputDevice.m_Device = v_Device;
		event.m_RawInputDevice.m_DeviceType = v_DeviceType;
		return event;
	}

	WindowEvent makeDropEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y, uint32_t v_FileCount, const char* const* p_FilePaths) {
		WindowEvent event;
		event.m_Type = WindowEventType::DROP;
		event.m_Handle = v_Handle;
		event.m_Timestamp = Chrono::MonotonicClock::now();
		event.m_Drop.m_X = v_X;
		event.m_Drop.m_Y = v_Y;
		event.m_Drop.m_FileCount = v_FileCount;
		event.m_Drop.m_FilePaths = p_FilePaths;
		return event;
	}
}
