#pragma once
#include "SpectraPlatformRuntime.h"
#include "SpectraCompiler.h"
#include "WindowUtils.h"

namespace Spectra::Platform::Runtime::Windows {
	class SPECTRA_RUNTIME_API PlatformWindow final {
	public:
		static void init();
		static void shutdown();

		SPECTRA_NODISCARD static WindowHandle create(const WindowDesc& ro_Desc);
		static void show(const WindowHandle& ro_Handle) noexcept;
		static void destroy(const WindowHandle& ro_Handle) noexcept;

		SPECTRA_NODISCARD static bool pollEvent(const WindowHandle& ro_Handle, WindowEvent& ro_Event) noexcept;

		static void setHitTestCallback(const WindowHandle& ro_Handle, HitTestFn v_Callback) noexcept;
		static void setDropRegionCallback(const WindowHandle& ro_Handle, DropRegionFn v_Callback) noexcept;

		static void minimize(const WindowHandle& ro_Handle) noexcept;
		static void maximize(const WindowHandle& ro_Handle) noexcept;
		static void restore(const WindowHandle& ro_Handle) noexcept;
		SPECTRA_NODISCARD static bool isMinimized(const WindowHandle& ro_Handle) noexcept;
		SPECTRA_NODISCARD static bool isMaximized(const WindowHandle& ro_Handle) noexcept;

		SPECTRA_NODISCARD static bool isValidHandle(const WindowHandle& ro_Handle) noexcept;
	};

	class SPECTRA_RUNTIME_API PlatformDisplay final {
	public:
		// Call once with p_Out == nullptr to get the display count, then again
		// with a caller-supplied buffer sized to that count.
		SPECTRA_NODISCARD static size_t enumerateDisplays(DisplayHandle* p_Out, size_t v_Capacity);
		SPECTRA_NODISCARD static DisplayInfo getDisplayInfo(const DisplayHandle& ro_Display);
	};

	class SPECTRA_RUNTIME_API PlatformInput final {
	public:
		static void registerDevices(const RawInputDeviceDesc* p_Devices, size_t v_Count);

		// Same caller-supplied-buffer convention as PlatformDisplay::enumerateDisplays.
		SPECTRA_NODISCARD static size_t enumerateDevices(RawInputDeviceInfo* p_Out, size_t v_Capacity);
	};
}
