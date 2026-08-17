#pragma once

#include "PlatformChrono.h"
#include "SpectraPlatformRuntime.h"
#include "SpectraCompiler.h"

namespace Spectra::Platform::Runtime::Windows {
	namespace Internal {
		struct PlatformWindowThunkHelper;
		struct PlatformDisplayEnumHelper;
		struct WindowDropTargetImpl;
	}

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

	constexpr WindowStyleFlags operator|(const WindowStyleFlags v_Lhs, const WindowStyleFlags v_Rhs) {
		return static_cast<WindowStyleFlags>(static_cast<uint32_t>(v_Lhs) | static_cast<uint32_t>(v_Rhs));
	}

	constexpr WindowStyleFlags operator&(const WindowStyleFlags v_Lhs, const WindowStyleFlags v_Rhs) {
		return static_cast<WindowStyleFlags>(static_cast<uint32_t>(v_Lhs) & static_cast<uint32_t>(v_Rhs));
	}

	constexpr WindowStyleFlags operator^(const WindowStyleFlags v_Lhs, const WindowStyleFlags v_Rhs) {
		return static_cast<WindowStyleFlags>(static_cast<uint32_t>(v_Lhs) ^ static_cast<uint32_t>(v_Rhs));
	}

	constexpr WindowStyleFlags operator~(const WindowStyleFlags v_Val) {
		return static_cast<WindowStyleFlags>(~static_cast<uint32_t>(v_Val));
	}

	constexpr WindowStyleFlags& operator|=(WindowStyleFlags& r_Lhs, const WindowStyleFlags v_Rhs) {
		r_Lhs = static_cast<WindowStyleFlags>(static_cast<uint32_t>(r_Lhs) | static_cast<uint32_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr WindowStyleFlags& operator&=(WindowStyleFlags& r_Lhs, const WindowStyleFlags v_Rhs) {
		r_Lhs = static_cast<WindowStyleFlags>(static_cast<uint32_t>(r_Lhs) & static_cast<uint32_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr WindowStyleFlags& operator^=(WindowStyleFlags& r_Lhs, const WindowStyleFlags v_Rhs) {
		r_Lhs = static_cast<WindowStyleFlags>(static_cast<uint32_t>(r_Lhs) ^ static_cast<uint32_t>(v_Rhs));
		return r_Lhs;
	}

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

	constexpr KeyModifierFlags operator|(const KeyModifierFlags v_Lhs, const KeyModifierFlags v_Rhs) {
		return static_cast<KeyModifierFlags>(static_cast<uint16_t>(v_Lhs) | static_cast<uint16_t>(v_Rhs));
	}

	constexpr KeyModifierFlags operator&(const KeyModifierFlags v_Lhs, const KeyModifierFlags v_Rhs) {
		return static_cast<KeyModifierFlags>(static_cast<uint16_t>(v_Lhs) & static_cast<uint16_t>(v_Rhs));
	}

	constexpr KeyModifierFlags operator^(const KeyModifierFlags v_Lhs, const KeyModifierFlags v_Rhs) {
		return static_cast<KeyModifierFlags>(static_cast<uint16_t>(v_Lhs) ^ static_cast<uint16_t>(v_Rhs));
	}

	constexpr KeyModifierFlags operator~(const KeyModifierFlags v_Val) {
		return static_cast<KeyModifierFlags>(~static_cast<uint16_t>(v_Val));
	}

	constexpr KeyModifierFlags& operator|=(KeyModifierFlags& r_Lhs, const KeyModifierFlags v_Rhs) {
		r_Lhs = static_cast<KeyModifierFlags>(static_cast<uint16_t>(r_Lhs) | static_cast<uint16_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr KeyModifierFlags& operator&=(KeyModifierFlags& r_Lhs, const KeyModifierFlags v_Rhs) {
		r_Lhs = static_cast<KeyModifierFlags>(static_cast<uint16_t>(r_Lhs) & static_cast<uint16_t>(v_Rhs));
		return r_Lhs;
	}

	constexpr KeyModifierFlags& operator^=(KeyModifierFlags& r_Lhs, const KeyModifierFlags v_Rhs) {
		r_Lhs = static_cast<KeyModifierFlags>(static_cast<uint16_t>(r_Lhs) ^ static_cast<uint16_t>(v_Rhs));
		return r_Lhs;
	}

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

	using HitTestFn = WindowHitTestResult(*)(int32_t v_X, int32_t v_Y);
	using DropRegionFn = bool(*)(int32_t v_X, int32_t v_Y);

	// See project_display_manager_frame_graph.md - the UI-thread-only pump loop
	// calls this once per drained event to hand it off into a second, genuinely
	// cross-thread queue owned by the caller. No-op (nullptr) by default.
	struct WindowEvent;
	using EventForwardFn = void(*)(void* p_Context, const WindowEvent& ro_Event);

	struct alignas(16) SPECTRA_RUNTIME_API WindowHandle final {
		friend class PlatformWindow;
		friend class PlatformInput;
		friend struct Internal::PlatformWindowThunkHelper;
		friend struct Internal::WindowDropTargetImpl;
	private:
		size_t m_Index = SIZE_MAX;
		size_t m_Generation = 0;

		WindowHandle(size_t v_Index, size_t v_Generation) : m_Index(v_Index), m_Generation(v_Generation) {}
	public:
		WindowHandle() = default;
		~WindowHandle() = default;
		WindowHandle(const WindowHandle&) = default;

		static SPECTRA_FORCEINLINE WindowHandle getInvalidHandle() {
			return {};
		}
	};

	struct alignas(8) SPECTRA_RUNTIME_API DisplayHandle final {
		friend class PlatformDisplay;
		friend struct Internal::PlatformDisplayEnumHelper;
		friend struct Internal::PlatformWindowThunkHelper;
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
		friend struct Internal::PlatformWindowThunkHelper;
		void* m_DeviceHandle = nullptr;
	private:
		explicit RawInputDeviceHandle(void* v_Handle) : m_DeviceHandle(v_Handle) {}
	public:
		RawInputDeviceHandle() = default;
		~RawInputDeviceHandle() = default;
		RawInputDeviceHandle(const RawInputDeviceHandle&) = default;
	};

	struct SPECTRA_RUNTIME_API DisplayBounds final {
		int32_t m_Left = 0;
		int32_t m_Top = 0;
		int32_t m_Right = 0;
		int32_t m_Bottom = 0;
	};

	struct alignas(8) SPECTRA_RUNTIME_API WindowGeometry final {
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

	struct alignas(16) SPECTRA_RUNTIME_API DisplayInfo final {
		DisplayBounds m_Bounds;
		DisplayBounds m_WorkArea;
		uint32_t      m_Dpi = 0;
		bool          m_IsPrimary = false;
		char          m_DeviceName[64] = {};
	};

	struct alignas(16) SPECTRA_RUNTIME_API WindowDesc final {
		static constexpr int32_t POSITION_DEFAULT = INT32_MIN;

		const char* m_Title = nullptr;
		uint32_t         m_Width = 0;
		uint32_t         m_Height = 0;
		int32_t          m_X = POSITION_DEFAULT;
		int32_t          m_Y = POSITION_DEFAULT;
		DisplayHandle    m_TargetMonitor = {};
		WindowStyleFlags m_StyleFlags = WindowStyleFlags::NONE;
		WindowGeometry   m_MinSize = {};
		WindowGeometry   m_MaxSize = {};
		WindowHandle     m_Parent = {};
	};

	struct alignas(16) SPECTRA_RUNTIME_API RawInputDeviceDesc final {
		RawInputDeviceType m_DeviceType = RawInputDeviceType::MOUSE;
		WindowHandle       m_Target = {};
		bool               m_BackgroundInput = false;
		bool               m_NotifyDeviceChanges = false;
	};

	struct alignas(8) SPECTRA_RUNTIME_API RawInputDeviceInfo final {
		RawInputDeviceHandle m_Handle = {};
		RawInputDeviceType   m_DeviceType = RawInputDeviceType::MOUSE;
		uint16_t             m_VendorId = 0;
		uint16_t             m_ProductId = 0;
		uint16_t             m_UsagePage = 0;
		uint16_t             m_Usage = 0;
		char                 m_DevicePath[260] = {};
	};

	struct alignas(8) SPECTRA_RUNTIME_API CloseEvent final {
		CloseEvent() = default;
		~CloseEvent() = default;
		CloseEvent(const CloseEvent&) = default;
		CloseEvent(CloseEvent&&) noexcept = default;
		CloseEvent& operator=(const CloseEvent&) = default;
		CloseEvent& operator=(CloseEvent&&) noexcept = default;

	private:
		SPECTRA_MAYBE_UNUSED uint64_t m_Padding = 0;
	};

	struct alignas(16) SPECTRA_RUNTIME_API ResizeEvent final {
		WindowGeometry    m_Size;
		WindowVisualState m_State = WindowVisualState::RESTORED;

		ResizeEvent() = default;
		~ResizeEvent() = default;
		ResizeEvent(const ResizeEvent&) = default;
		ResizeEvent(ResizeEvent&&) noexcept = default;
		ResizeEvent& operator=(const ResizeEvent&) = default;
		ResizeEvent& operator=(ResizeEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API MoveEvent final {
		int32_t m_X = 0;
		int32_t m_Y = 0;

		MoveEvent() = default;
		~MoveEvent() = default;
		MoveEvent(const MoveEvent&) = default;
		MoveEvent(MoveEvent&&) noexcept = default;
		MoveEvent& operator=(const MoveEvent&) = default;
		MoveEvent& operator=(MoveEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API FocusGainedEvent final {
		FocusGainedEvent() = default;
		~FocusGainedEvent() = default;
		FocusGainedEvent(const FocusGainedEvent&) = default;
		FocusGainedEvent(FocusGainedEvent&&) noexcept = default;
		FocusGainedEvent& operator=(const FocusGainedEvent&) = default;
		FocusGainedEvent& operator=(FocusGainedEvent&&) noexcept = default;

	private:
		SPECTRA_MAYBE_UNUSED uint64_t m_Padding = 0;
	};

	struct alignas(8) SPECTRA_RUNTIME_API FocusLostEvent final {
		FocusLostEvent() = default;
		~FocusLostEvent() = default;
		FocusLostEvent(const FocusLostEvent&) = default;
		FocusLostEvent(FocusLostEvent&&) noexcept = default;
		FocusLostEvent& operator=(const FocusLostEvent&) = default;
		FocusLostEvent& operator=(FocusLostEvent&&) noexcept = default;

	private:
		SPECTRA_MAYBE_UNUSED uint64_t m_Padding = 0;
	};

	struct alignas(8) SPECTRA_RUNTIME_API KeyEvent final {
		KeyCode          m_Code      = KeyCode::KC_UNKNOWN;
		KeyModifierFlags m_Modifiers = KeyModifierFlags::NONE;
		bool             m_IsRepeat  = false;

		KeyEvent() = default;
		~KeyEvent() = default;
		KeyEvent(const KeyEvent&) = default;
		KeyEvent(KeyEvent&&) noexcept = default;
		KeyEvent& operator=(const KeyEvent&) = default;
		KeyEvent& operator=(KeyEvent&&) noexcept = default;
	};

	struct alignas(4) SPECTRA_RUNTIME_API TextInputEvent final {
		char32_t m_Codepoint = 0;

		TextInputEvent() = default;
		~TextInputEvent() = default;
		TextInputEvent(const TextInputEvent&) = default;
		TextInputEvent(TextInputEvent&&) noexcept = default;
		TextInputEvent& operator=(const TextInputEvent&) = default;
		TextInputEvent& operator=(TextInputEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API MouseMoveEvent final {
		int32_t m_X = 0;
		int32_t m_Y = 0;

		MouseMoveEvent() = default;
		~MouseMoveEvent() = default;
		MouseMoveEvent(const MouseMoveEvent&) = default;
		MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
		MouseMoveEvent& operator=(const MouseMoveEvent&) = default;
		MouseMoveEvent& operator=(MouseMoveEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API MouseButtonEvent final {
		MouseButton m_Button = MouseButton::LEFT;
		int32_t     m_X      = 0;
		int32_t     m_Y      = 0;

		MouseButtonEvent() = default;
		~MouseButtonEvent() = default;
		MouseButtonEvent(const MouseButtonEvent&) = default;
		MouseButtonEvent(MouseButtonEvent&&) noexcept = default;
		MouseButtonEvent& operator=(const MouseButtonEvent&) = default;
		MouseButtonEvent& operator=(MouseButtonEvent&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API MouseWheelEvent final {
		int32_t m_Delta = 0;
		int32_t m_X     = 0;
		int32_t m_Y     = 0;

		MouseWheelEvent() = default;
		~MouseWheelEvent() = default;
		MouseWheelEvent(const MouseWheelEvent&) = default;
		MouseWheelEvent(MouseWheelEvent&&) noexcept = default;
		MouseWheelEvent& operator=(const MouseWheelEvent&) = default;
		MouseWheelEvent& operator=(MouseWheelEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API RawMouseDelta final {
		int32_t  m_DeltaX      = 0;
		int32_t  m_DeltaY      = 0;
		uint32_t m_ButtonFlags = 0;
		int32_t  m_WheelDelta  = 0;

		RawMouseDelta() = default;
		~RawMouseDelta() = default;
		RawMouseDelta(const RawMouseDelta&) = default;
		RawMouseDelta(RawMouseDelta&&) noexcept = default;
		RawMouseDelta& operator=(const RawMouseDelta&) = default;
		RawMouseDelta& operator=(RawMouseDelta&&) noexcept = default;
	};

	struct alignas(4) SPECTRA_RUNTIME_API RawKeyEvent final {
		uint16_t m_ScanCode   = 0;
		KeyCode  m_Code       = KeyCode::KC_UNKNOWN;
		bool     m_IsDown     = false;
		bool     m_IsExtended = false;

		RawKeyEvent() = default;
		~RawKeyEvent() = default;
		RawKeyEvent(const RawKeyEvent&) = default;
		RawKeyEvent(RawKeyEvent&&) noexcept = default;
		RawKeyEvent& operator=(const RawKeyEvent&) = default;
		RawKeyEvent& operator=(RawKeyEvent&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API RawInputEvent final {
		RawInputDeviceType m_DeviceType = RawInputDeviceType::MOUSE;
		union {
			RawMouseDelta m_Mouse;
			RawKeyEvent   m_Key;
		};

		RawInputEvent() : m_DeviceType(RawInputDeviceType::MOUSE), m_Mouse() {}
		~RawInputEvent() = default;
		RawInputEvent(const RawInputEvent&) = default;
		RawInputEvent(RawInputEvent&&) noexcept = default;
		RawInputEvent& operator=(const RawInputEvent&) = default;
		RawInputEvent& operator=(RawInputEvent&&) noexcept = default;
	};

	struct alignas(8) SPECTRA_RUNTIME_API DisplayChangedEvent final {
		DisplayHandle m_Display = {};

		DisplayChangedEvent() = default;
		~DisplayChangedEvent() = default;
		DisplayChangedEvent(const DisplayChangedEvent&) = default;
		DisplayChangedEvent(DisplayChangedEvent&&) noexcept = default;
		DisplayChangedEvent& operator=(const DisplayChangedEvent&) = default;
		DisplayChangedEvent& operator=(DisplayChangedEvent&&) noexcept = default;
	};

	struct alignas(4) SPECTRA_RUNTIME_API DpiChangedEvent final {
		uint32_t m_Dpi = 0;

		DpiChangedEvent() = default;
		~DpiChangedEvent() = default;
		DpiChangedEvent(const DpiChangedEvent&) = default;
		DpiChangedEvent(DpiChangedEvent&&) noexcept = default;
		DpiChangedEvent& operator=(const DpiChangedEvent&) = default;
		DpiChangedEvent& operator=(DpiChangedEvent&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API RawInputDeviceEvent final {
		RawInputDeviceHandle m_Device     = {};
		RawInputDeviceType   m_DeviceType = RawInputDeviceType::MOUSE;

		RawInputDeviceEvent() = default;
		~RawInputDeviceEvent() = default;
		RawInputDeviceEvent(const RawInputDeviceEvent&) = default;
		RawInputDeviceEvent(RawInputDeviceEvent&&) noexcept = default;
		RawInputDeviceEvent& operator=(const RawInputDeviceEvent&) = default;
		RawInputDeviceEvent& operator=(RawInputDeviceEvent&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API DropEvent final {
		int32_t            m_X         = 0;
		int32_t            m_Y         = 0;
		uint32_t           m_FileCount = 0;
		const char* const* m_FilePaths = nullptr;

		DropEvent() = default;
		~DropEvent() = default;
		DropEvent(const DropEvent&) = default;
		DropEvent(DropEvent&&) noexcept = default;
		DropEvent& operator=(const DropEvent&) = default;
		DropEvent& operator=(DropEvent&&) noexcept = default;
	};

	struct alignas(16) SPECTRA_RUNTIME_API WindowEvent final {
		WindowEventType   m_Type;
		WindowHandle      m_Handle;
		Chrono::Timestamp m_Timestamp;

		union {
			CloseEvent           m_Close;
			ResizeEvent          m_Resize;
			MoveEvent            m_Move;
			FocusGainedEvent     m_FocusGained;
			FocusLostEvent       m_FocusLost;
			KeyEvent             m_Key;
			TextInputEvent       m_TextInput;
			MouseMoveEvent       m_MouseMove;
			MouseButtonEvent     m_MouseButton;
			MouseWheelEvent      m_MouseWheel;
			RawInputEvent        m_RawInput;
			DisplayChangedEvent  m_DisplayChanged;
			DpiChangedEvent      m_DpiChanged;
			RawInputDeviceEvent  m_RawInputDevice;
			DropEvent            m_Drop;
		};

		WindowEvent() : m_Type(WindowEventType::CLOSE), m_Handle(), m_Timestamp(), m_Close() {}
	};

	static_assert(sizeof(WindowEvent) <= 128, "WindowEvent exceeds the L2 struct budget");

	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeCloseEvent(WindowHandle v_Handle);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeResizeEvent(WindowHandle v_Handle, WindowGeometry v_Size, WindowVisualState v_State);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeMoveEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeFocusGainedEvent(WindowHandle v_Handle);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeFocusLostEvent(WindowHandle v_Handle);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeKeyEvent(WindowHandle v_Handle, WindowEventType v_Type, KeyCode v_Code, KeyModifierFlags v_Modifiers, bool v_IsRepeat);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeTextInputEvent(WindowHandle v_Handle, char32_t v_Codepoint);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeMouseMoveEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeMouseButtonEvent(WindowHandle v_Handle, WindowEventType v_Type, MouseButton v_Button, int32_t v_X, int32_t v_Y);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeMouseWheelEvent(WindowHandle v_Handle, int32_t v_Delta, int32_t v_X, int32_t v_Y);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeRawInputEvent(WindowHandle v_Handle, const RawMouseDelta& ro_Delta);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeRawInputEvent(WindowHandle v_Handle, const RawKeyEvent& ro_Key);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeDisplayChangedEvent(WindowHandle v_Handle, DisplayHandle v_Display);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeDpiChangedEvent(WindowHandle v_Handle, uint32_t v_Dpi);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeRawInputDeviceEvent(WindowHandle v_Handle, WindowEventType v_Type, RawInputDeviceHandle v_Device, RawInputDeviceType v_DeviceType);
	SPECTRA_NODISCARD WindowEvent SPECTRA_RUNTIME_API makeDropEvent(WindowHandle v_Handle, int32_t v_X, int32_t v_Y, uint32_t v_FileCount, const char* const* p_FilePaths);
}
