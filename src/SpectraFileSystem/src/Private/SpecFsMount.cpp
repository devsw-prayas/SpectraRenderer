#include "SpecFsMount.h"
#include "SpecFsDiagnostic.h"

#include <PlatformFileSystem.h>
#include <ProcessEnvironment.h>

namespace Spectra::FileSystem {
	namespace {
		using Platform::Runtime::File::Files;
		using Platform::Runtime::File::KnownFolder;
		using Platform::Runtime::File::PathAttribute;
		using Platform::Runtime::Intrinsic::MemoryOrder;

		struct MountSlot {
			PathBuffer m_Root;
			PathTree m_Tree;
			char m_Name[MOUNT_NAME_CAPACITY + 1];
			uint8_t m_NameLength;
			MountFlags m_Flags;
			bool m_Used;
			uint32_t m_Generation;
			Platform::Runtime::Atomic::AtomicValue32<uint32_t> m_OpenFiles;
		};

		MountSlot g_Mounts[MAX_MOUNTS];
		Core::Threads::ReadWriteLock g_MountLock;
		bool g_Init = false;
		MountHandle g_Engine, g_User, g_Temp;

		std::string_view nameOf(const MountSlot& r_Slot) noexcept { return { r_Slot.m_Name, r_Slot.m_NameLength }; }

		bool isValidName(std::string_view v_Name) noexcept {
			// Same shape Paths parses as a mount prefix: a letter, then letters, digits or '_'.
			if (v_Name.size() < 2 || v_Name.size() > MOUNT_NAME_CAPACITY) return false;
			const auto isLetter = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
			if (!isLetter(v_Name[0])) return false;
			for (const char c : v_Name)
				if (!(isLetter(c) || (c >= '0' && c <= '9') || c == '_')) return false;
			return true;
		}

		// Caller holds g_MountLock.
		MountSlot* slotOf(MountHandle v_Mount) noexcept {
			if (v_Mount.m_Index >= MAX_MOUNTS) return nullptr;
			MountSlot& slot = g_Mounts[v_Mount.m_Index];
			return slot.m_Used && slot.m_Generation == v_Mount.m_Generation ? &slot : nullptr;
		}

		bool equalsFolded(std::string_view v_A, std::string_view v_B) noexcept {
			if (v_A.size() != v_B.size()) return false;
			for (size_t i = 0; i < v_A.size(); ++i) {
				char a = v_A[i], b = v_B[i];
				if (a >= 'A' && a <= 'Z') a = char(a + 32);
				if (b >= 'A' && b <= 'Z') b = char(b + 32);
				if (a != b) return false;
			}
			return true;
		}

		// Case-insensitive, like path segments: "User:/" and "user:/" are the same mount.
		uint32_t findIndex(std::string_view v_Name) noexcept {
			for (uint32_t i = 0; i < MAX_MOUNTS; ++i)
				if (g_Mounts[i].m_Used && equalsFolded(nameOf(g_Mounts[i]), v_Name)) return i;
			return UINT32_MAX;
		}

		bool joinRoot(const MountSlot& r_Slot, const PathBuffer& r_Path, PathBuffer& r_Native) noexcept {
			PathBuffer native = r_Slot.m_Root;
			if (!Paths::join(native, Paths::relativePart(r_Path))) return false;
			r_Native = native;
			return true;
		}

		bool knownFolder(KnownFolder v_Folder, PathBuffer& r_Out) noexcept {
			wchar_t wide[NATIVE_PATH_CAPACITY];
			uint32_t length = 0;
			return Files::getKnownFolder(v_Folder, wide, NATIVE_PATH_CAPACITY, length) && Paths::makeFromNative(r_Out, { wide, length });
		}

		bool ensureDirectory(const PathBuffer& r_Path) noexcept {
			wchar_t wide[NATIVE_PATH_CAPACITY];
			uint32_t length = 0;
			return Paths::toNative(r_Path, wide, length) && Files::createDirectory(wide);
		}

		bool isDirectory(const PathBuffer& r_Path) noexcept {
			wchar_t wide[NATIVE_PATH_CAPACITY];
			uint32_t length = 0;
			return Paths::toNative(r_Path, wide, length) && Files::getAttribute(wide) == PathAttribute::DIRECTORY;
		}

		bool removeTree(const PathBuffer& r_Path) noexcept {
			wchar_t wide[NATIVE_PATH_CAPACITY];
			uint32_t length = 0;
			return Paths::toNative(r_Path, wide, length) && Files::removeTree(wide);
		}

		// temp:/ lives in <TEMP>/Spectra/<pid>; folders left by dead processes are removed at boot.
		struct StaleSweep {
			PathBuffer m_Parent;
			uint32_t m_Self;
		};

		bool sweepStale(const wchar_t* p_Name, bool v_IsDirectory, void* p_Context) {
			if (!v_IsDirectory) return true;
			uint32_t pid = 0;
			for (const wchar_t* c = p_Name; *c; ++c) {
				if (*c < L'0' || *c > L'9') return true;
				pid = pid * 10 + uint32_t(*c - L'0');
			}
			StaleSweep& sweep = *static_cast<StaleSweep*>(p_Context);
			if (pid == sweep.m_Self || Platform::Runtime::Environment::PlatformProcess::isProcessAlive(pid)) return true;

			PathBuffer stale = sweep.m_Parent;
			char name[16];
			size_t length = 0;
			for (const wchar_t* c = p_Name; *c && length < sizeof(name); ++c) name[length++] = char(*c);
			if (Paths::join(stale, { name, length })) (void)removeTree(stale);
			return true;
		}

		bool mountTemp() noexcept {
			PathBuffer root;
			if (!knownFolder(KnownFolder::TEMP, root) || !Paths::join(root, "Spectra") || !ensureDirectory(root)) return false;

			StaleSweep sweep{ root, Platform::Runtime::Environment::PlatformProcess::getCurrentProcessId() };
			wchar_t wide[NATIVE_PATH_CAPACITY];
			uint32_t length = 0;
			if (Paths::toNative(root, wide, length)) (void)Files::forEachChild(wide, &sweepStale, &sweep);

			char pid[16];
			const int pidLength = std::snprintf(pid, sizeof(pid), "%u", sweep.m_Self);
			return Paths::join(root, { pid, size_t(pidLength) }) && ensureDirectory(root)
				&& Mounts::add("temp", root, MountFlags::WIPE_ON_EXIT, Trees::DEFAULT_BLOCKS / 4, g_Temp);
		}
	}

	bool Mounts::add(std::string_view v_Name, const PathBuffer& r_NativeRoot, MountFlags v_Flags, uint32_t v_TreeBlocks, MountHandle& r_Out) noexcept {
		if (!isValidName(v_Name) || r_NativeRoot.m_Kind != PathKind::NATIVE_ABSOLUTE) return false;  // exec InvalidPath
		if (!isDirectory(r_NativeRoot)) return false;  // exec NotFound

		g_MountLock.lockWrite();
		uint32_t index = findIndex(v_Name) == UINT32_MAX ? 0 : MAX_MOUNTS;  // exec AlreadyExists
		while (index < MAX_MOUNTS && g_Mounts[index].m_Used) ++index;
		bool ok = index < MAX_MOUNTS;  // exec TooManyMounts
		if (ok) {
			MountSlot& slot = g_Mounts[index];
			ok = Trees::create(slot.m_Tree, v_Name, v_TreeBlocks, 0);
			if (ok) {
				slot.m_Root = r_NativeRoot;
				std::memcpy(slot.m_Name, v_Name.data(), v_Name.size());
				slot.m_NameLength = uint8_t(v_Name.size());
				slot.m_Flags = v_Flags;
				slot.m_OpenFiles.store(0, MemoryOrder::RELAXED);
				slot.m_Used = true;
				r_Out = { index, slot.m_Generation };
			}
		}
		g_MountLock.unlockWrite();
		return ok;
	}

	bool Mounts::remove(MountHandle v_Mount) noexcept {
		g_MountLock.lockWrite();
		MountSlot* slot = slotOf(v_Mount);
		const bool ok = slot != nullptr && slot->m_OpenFiles.load(MemoryOrder::ACQUIRE) == 0;  // exec NotFound / MountHasOpenFiles
		if (ok) {
			Trees::destroy(slot->m_Tree);
			if (has(slot->m_Flags, MountFlags::WIPE_ON_EXIT)) (void)removeTree(slot->m_Root);
			slot->m_Used = false;
			++slot->m_Generation;
		}
		g_MountLock.unlockWrite();
		return ok;
	}

	MountHandle Mounts::find(std::string_view v_Name) noexcept {
		g_MountLock.lockRead();
		const uint32_t index = findIndex(v_Name);
		const MountHandle handle = index == UINT32_MAX ? MountHandle{} : MountHandle{ index, g_Mounts[index].m_Generation };
		g_MountLock.unlockRead();
		return handle;
	}
 
	bool Mounts::isValid(MountHandle v_Mount) noexcept {
		g_MountLock.lockRead();
		const bool valid = slotOf(v_Mount) != nullptr;
		g_MountLock.unlockRead();
		return valid;
	}

	MountFlags Mounts::flags(MountHandle v_Mount) noexcept {
		g_MountLock.lockRead();
		const MountSlot* slot = slotOf(v_Mount);
		const MountFlags result = slot ? slot->m_Flags : MountFlags::NONE;
		g_MountLock.unlockRead();
		return result;
	}

	// The tree outlives the lock only while the caller holds a file on the mount (acquire), or owns the mount.
	PathTree* Mounts::tree(MountHandle v_Mount) noexcept {
		g_MountLock.lockRead();
		MountSlot* slot = slotOf(v_Mount);
		g_MountLock.unlockRead();
		return slot ? &slot->m_Tree : nullptr;
	}

	bool Mounts::root(MountHandle v_Mount, PathBuffer& r_Out) noexcept {
		g_MountLock.lockRead();
		const MountSlot* slot = slotOf(v_Mount);
		if (slot) r_Out = slot->m_Root;
		g_MountLock.unlockRead();
		return slot != nullptr;
	}

	bool Mounts::resolve(const PathBuffer& r_Path, PathBuffer& r_Native) noexcept {
		if (r_Path.m_Kind == PathKind::NATIVE_ABSOLUTE) {
			r_Native = r_Path;
			return true;
		}
		if (r_Path.m_Kind != PathKind::MOUNTED) return false;  // exec InvalidPath

		g_MountLock.lockRead();
		const uint32_t index = findIndex(Paths::mountName(r_Path));
		const bool ok = index != UINT32_MAX && joinRoot(g_Mounts[index], r_Path, r_Native);  // exec NotFound
		g_MountLock.unlockRead();
		return ok;
	}

	bool Mounts::resolve(MountHandle v_Mount, const PathBuffer& r_Path, PathBuffer& r_Native) noexcept {
		if (r_Path.m_Kind != PathKind::RELATIVE && r_Path.m_Kind != PathKind::MOUNTED && r_Path.m_Kind != PathKind::EMPTY) return false;  // exec InvalidPath
		g_MountLock.lockRead();
		const MountSlot* slot = slotOf(v_Mount);
		const bool ok = slot != nullptr && joinRoot(*slot, r_Path, r_Native);  // exec NotFound
		g_MountLock.unlockRead();
		return ok;
	}

	bool Mounts::acquire(MountHandle v_Mount) noexcept {
		g_MountLock.lockRead();
		MountSlot* slot = slotOf(v_Mount);
		if (slot) slot->m_OpenFiles.fetchAdd(1, MemoryOrder::ACQ_REL);
		g_MountLock.unlockRead();
		return slot != nullptr;
	}

	void Mounts::release(MountHandle v_Mount) noexcept {
		g_MountLock.lockRead();
		MountSlot* slot = slotOf(v_Mount);
		if (slot) {
			const uint32_t previous = slot->m_OpenFiles.fetchAdd(UINT32_MAX, MemoryOrder::ACQ_REL);
			SPEC_FS_ASSERT(previous != 0);
		}
		g_MountLock.unlockRead();
	}

	bool init(std::string_view v_EngineRoot) noexcept {
		SPEC_FS_ASSERT(!g_Init);
		if (g_Init) return true;

		PathBuffer user;
		if (!knownFolder(KnownFolder::LOCAL_APP_DATA, user) || !Paths::join(user, "Spectra") || !ensureDirectory(user)) return false;  // err: no user:/
		if (!Mounts::add("user", user, MountFlags::NONE, Trees::DEFAULT_BLOCKS / 4, g_User)) return false;
		if (!mountTemp()) {
			// Leave nothing mounted so a retry starts clean.
			(void)Mounts::remove(g_User);
			g_User = {};
			return false;
		}

		// engine:/ is optional at this layer: a missing data folder only fails the loads that need it.
		PathBuffer engine;
		const bool haveRoot = v_EngineRoot.empty()
			? knownFolder(KnownFolder::EXECUTABLE_DIR, engine) && Paths::join(engine, "Engine")
			: Paths::make(engine, v_EngineRoot) && engine.m_Kind == PathKind::NATIVE_ABSOLUTE;
		if (haveRoot) (void)Mounts::add("engine", engine, MountFlags::READ_ONLY, Trees::DEFAULT_BLOCKS / 4, g_Engine);

		g_Init = true;
		return true;
	}

	void shutdown() noexcept {
		if (!g_Init) return;
		for (uint32_t i = 0; i < MAX_MOUNTS; ++i) {
			if (!g_Mounts[i].m_Used) continue;
			const bool removed = Mounts::remove({ i, g_Mounts[i].m_Generation });
			SPEC_FS_ASSERT(removed);
		}
		g_Init = false;
	}

	bool isInit() noexcept {
		return g_Init;
	}
}
