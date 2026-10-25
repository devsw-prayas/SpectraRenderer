#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"
#include "SpecFsPath.h"
#include "SpecFsTree.h"

namespace Spectra::FileSystem {
	constexpr uint32_t MAX_MOUNTS = 32;
	constexpr uint32_t MOUNT_NAME_CAPACITY = 31;

	enum class MountFlags : uint8_t {
		NONE = 0,
		READ_ONLY = 1 << 0,
		WIPE_ON_EXIT = 1 << 1  // root directory removed with the mount
	};

	constexpr MountFlags operator|(MountFlags v_A, MountFlags v_B) noexcept { return MountFlags(uint8_t(v_A) | uint8_t(v_B)); }
	constexpr MountFlags operator&(MountFlags v_A, MountFlags v_B) noexcept { return MountFlags(uint8_t(v_A) & uint8_t(v_B)); }
	constexpr bool has(MountFlags v_Flags, MountFlags v_Bit) noexcept { return (v_Flags & v_Bit) != MountFlags::NONE; }

	struct MountHandle {
		uint32_t m_Index = UINT32_MAX;
		uint32_t m_Generation = 0;
	};

	// Named roots on disk, each with its own path tree. Knows nothing about projects:
	// SpectraProject mounts a project under its own name and resolves "project:/" through the handle overloads.
	class SPEC_FS_RUNTIME_API Mounts final {
	public:
		Mounts() = delete;

		// Root must be NATIVE_ABSOLUTE and an existing directory. Names are unique.
		SPEC_FS_NODISCARD static bool add(std::string_view v_Name, const PathBuffer& r_NativeRoot, MountFlags v_Flags, uint32_t v_TreeBlocks, MountHandle& r_Out) noexcept;
		// Refused while files are open on the mount.
		SPEC_FS_NODISCARD static bool remove(MountHandle v_Mount) noexcept;

		SPEC_FS_NODISCARD static MountHandle find(std::string_view v_Name) noexcept;
		SPEC_FS_NODISCARD static bool isValid(MountHandle v_Mount) noexcept;
		SPEC_FS_NODISCARD static MountFlags flags(MountHandle v_Mount) noexcept;
		SPEC_FS_NODISCARD static PathTree* tree(MountHandle v_Mount) noexcept;
		SPEC_FS_NODISCARD static bool root(MountHandle v_Mount, PathBuffer& r_Out) noexcept;

		// Mounted path by name, or a native path passed through.
		SPEC_FS_NODISCARD static bool resolve(const PathBuffer& r_Path, PathBuffer& r_Native) noexcept;
		// Relative or mounted path on this mount; the mount name in the path is not checked.
		SPEC_FS_NODISCARD static bool resolve(MountHandle v_Mount, const PathBuffer& r_Path, PathBuffer& r_Native) noexcept;

		// Open-file accounting for handles.
		SPEC_FS_NODISCARD static bool acquire(MountHandle v_Mount) noexcept;
		static void release(MountHandle v_Mount) noexcept;
	};

	// Brings up engine:/ (read-only), user:/ and temp:/ (wiped on exit). Call after Core::boot().
	// Empty v_EngineRoot means <exe dir>/Engine. Fails if user:/ can't be created.
	SPEC_FS_NODISCARD SPEC_FS_RUNTIME_API bool init(std::string_view v_EngineRoot = {}) noexcept;
	// Removes every mount. Files still open are a leak.
	SPEC_FS_RUNTIME_API void shutdown() noexcept;
	SPEC_FS_NODISCARD SPEC_FS_RUNTIME_API bool isInit() noexcept;
}
