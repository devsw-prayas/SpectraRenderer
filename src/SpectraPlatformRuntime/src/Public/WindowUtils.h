#pragma once

#include "SpectraPlatformRuntime.h"
#include "SpectraCompiler.h"

namespace Spectra::Platform::Runtime::Windows {
	enum class SPECTRA_RUNTIME_API WindowLifecycle : uint8_t {
		CREATED, SHOWN, DESTROYED
	};

	enum class SPECTRA_RUNTIME_API WindowVisualState : uint8_t {
		RESTORED, MINIMIZED, MAXIMIZED, HIDDEN
	};

	enum class SPECTRA_RUNTIME_API WindowStyleFlags : uint32_t {
		NONE = 0,
		RESIZABLE = 1 << 0,
		MINIMIZABLE = 1 << 1,
		MAXIMIZABLE = 1 << 2,
		UNDECORATED = 1 << 3,
		TOPMOST = 1 << 4,
		APP_WINDOW = 1 << 5,
		NO_REDIRECTION_BITMAP = 1 << 6,
	};

	enum class SPECTRA_RUNTIME_API WindowEventType : uint8_t {
		CLOSE = 0,
		RESIZE = 1,
		MOVE = 2,
		FOCUS_GAINED = 3,
		FOCUS_LOST = 4,
		KEY_DOWN = 5,
		KEY_UP = 6,
		TEXT_INPUT = 7,
		MOUSE_MOVE = 8,
		MOUSE_BUTTON_DOWN = 9,
		MOUSE_BUTTON_UP = 10,
		MOUSE_WHEEL = 11,
		RAW_INPUT = 12,
		DISPLAY_CHANGED = 13,
		DPI_CHANGED = 14,
		RAW_INPUT_DEVICE_ARRIVAL = 15,
		RAW_INPUT_DEVICE_REMOVAL = 16,
		DROP = 17,
	};

	enum class SPECTRA_RUNTIME_API WindowHitTestResult : uint8_t {
		CLIENT = 0,
		CAPTION = 1,
		NOWHERE = 2,
	};

	enum class SPECTRA_RUNTIME_API KeyCode : uint16_t {
		KC_UNKNOWN,

		KC_BACKSPACE,
		KC_TAB,
		KC_ENTER,
		KC_SHIFT,
		KC_CONTROL,
		KC_ALT,
		KC_PAUSE,
		KC_CAPS_LOCK,
		KC_ESCAPE,
		KC_SPACE,
		KC_PAGE_UP,
		KC_PAGE_DOWN,
		KC_END,
		KC_HOME,
		KC_LEFT,
		KC_UP,
		KC_RIGHT,
		KC_DOWN,
		KC_PRINT_SCREEN,
		KC_INSERT,
		KC_DELETE,

		KC_0,
		KC_1,
		KC_2,
		KC_3,
		KC_4,
		KC_5,
		KC_6,
		KC_7,
		KC_8,
		KC_9,

		KC_A,
		KC_B,
		KC_C,
		KC_D,
		KC_E,
		KC_F,
		KC_G,
		KC_H,
		KC_I,
		KC_J,
		KC_K,
		KC_L,
		KC_M,
		KC_N,
		KC_O,
		KC_P,
		KC_Q,
		KC_R,
		KC_S,
		KC_T,
		KC_U,
		KC_V,
		KC_W,
		KC_X,
		KC_Y,
		KC_Z,

		KC_LEFT_META,
		KC_RIGHT_META,
		KC_APPLICATION,

		KC_NUMPAD_0,
		KC_NUMPAD_1,
		KC_NUMPAD_2,
		KC_NUMPAD_3,
		KC_NUMPAD_4,
		KC_NUMPAD_5,
		KC_NUMPAD_6,
		KC_NUMPAD_7,
		KC_NUMPAD_8,
		KC_NUMPAD_9,
		KC_NUMPAD_MULTIPLY,
		KC_NUMPAD_ADD,
		KC_NUMPAD_SEPARATOR,
		KC_NUMPAD_SUBTRACT,
		KC_NUMPAD_DECIMAL,
		KC_NUMPAD_DIVIDE,

		KC_F1,
		KC_F2,
		KC_F3,
		KC_F4,
		KC_F5,
		KC_F6,
		KC_F7,
		KC_F8,
		KC_F9,
		KC_F10,
		KC_F11,
		KC_F12,
		KC_F13,
		KC_F14,
		KC_F15,
		KC_F16,
		KC_F17,
		KC_F18,
		KC_F19,
		KC_F20,
		KC_F21,
		KC_F22,
		KC_F23,
		KC_F24,

		KC_NUM_LOCK,
		KC_SCROLL_LOCK,

		KC_LEFT_SHIFT,
		KC_RIGHT_SHIFT,
		KC_LEFT_CONTROL,
		KC_RIGHT_CONTROL,
		KC_LEFT_ALT,
		KC_RIGHT_ALT,

		KC_SEMICOLON,
		KC_EQUAL,
		KC_COMMA,
		KC_MINUS,
		KC_PERIOD,
		KC_SLASH,
		KC_BACKTICK,
		KC_LEFT_BRACKET,
		KC_BACKSLASH,
		KC_RIGHT_BRACKET,
		KC_APOSTROPHE
	};

	enum class SPECTRA_RUNTIME_API KeyModifierFlags : uint16_t {
		NONE = 0,
		SHIFT = 1 << 0,
		CONTROL = 1 << 1,
		ALT = 1 << 2,
		META = 1 << 3,
		CAPS_LOCK = 1 << 4,
		NUM_LOCK = 1 << 5,
		LEFT_SHIFT = 1 << 6,
		RIGHT_SHIFT = 1 << 7,
		LEFT_CONTROL = 1 << 8,
		RIGHT_CONTROL = 1 << 9,
		LEFT_ALT = 1 << 10,
		RIGHT_ALT = 1 << 11,
		LEFT_META = 1 << 12,
		RIGHT_META = 1 << 13
	};

	enum class SPECTRA_RUNTIME_API MouseButton : uint8_t {
		LEFT = 0,
		RIGHT = 1,
		MIDDLE = 2,
		X1 = 3,
		X2 = 4
	};

	enum class SPECTRA_RUNTIME_API RawInputDeviceType : uint8_t {
		MOUSE,
		KEYBOARD,
		HID
	};

	struct alignas(16) SPECTRA_RUNTIME_API WindowHandle final {
		size_t m_Index      = 0;
		size_t m_Generation = 0;
	private:
		WindowHandle(size_t v_Index, size_t v_Generation) : m_Index(v_Index), m_Generation(v_Generation) {}
	public:
		WindowHandle() = default;
		~WindowHandle() = default;
		WindowHandle(const WindowHandle&) = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API DisplayHandle final {
		friend class PlatformDisplay;
		void* m_HMonitor = nullptr;
	private:
		explicit DisplayHandle(void* v_HMonitor) : m_HMonitor(v_HMonitor) {}
	public:
		DisplayHandle() = default;
		~DisplayHandle() = default;
		DisplayHandle(const DisplayHandle&) = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API RawInputDeviceHandle final {
		friend class PlatformInput;
		void* m_DeviceHandle = nullptr;
	private:
		explicit RawInputDeviceHandle(void* v_Handle) : m_DeviceHandle(v_Handle) {}
	public:
		RawInputDeviceHandle() = default;
		~RawInputDeviceHandle() = default;
		RawInputDeviceHandle(const RawInputDeviceHandle&) = default;
	};

	struct SPECTRA_RUNTIME_API DisplayBounds final {
		int32_t m_Left   = 0;
		int32_t m_Top    = 0;
		int32_t m_Right  = 0;
		int32_t m_Bottom = 0;
	};

	struct alignas(8) SPECTRA_RUNTIME_API WindowGeometry final {
		uint32_t m_Width  = 0;
		uint32_t m_Height = 0;
	};

	struct alignas(16) SPECTRA_RUNTIME_API DisplayInfo final {
		DisplayBounds m_Bounds;
		DisplayBounds m_WorkArea;
		uint32_t      m_Dpi        = 0;
		bool          m_IsPrimary  = false;
		char          m_DeviceName[64] = {};
	};

	struct alignas(16) SPECTRA_RUNTIME_API WindowDesc final {
		static constexpr int32_t POSITION_DEFAULT = INT32_MIN;

		const char*      m_Title         = nullptr;
		uint32_t         m_Width         = 0;
		uint32_t         m_Height        = 0;
		int32_t          m_X             = POSITION_DEFAULT;
		int32_t          m_Y             = POSITION_DEFAULT;
		DisplayHandle    m_TargetMonitor = {};
		WindowStyleFlags m_StyleFlags    = WindowStyleFlags::NONE;
		WindowGeometry   m_MinSize       = {};
		WindowGeometry   m_MaxSize       = {};
		WindowHandle     m_Parent        = {};
	};

	struct alignas(16) SPECTRA_RUNTIME_API RawInputDeviceDesc final {
		RawInputDeviceType m_DeviceType          = RawInputDeviceType::MOUSE;
		WindowHandle       m_Target              = {};
		bool               m_BackgroundInput     = false;
		bool               m_NotifyDeviceChanges = false;
	};
}
