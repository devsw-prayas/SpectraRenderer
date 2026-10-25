#include "SpecFsFile.h"
#include "SpecFsDiagnostic.h"

#include <PlatformFileSystem.h>

namespace Spectra::FileSystem {
	namespace {
		namespace Native = Platform::Runtime::File;
		using NativeFiles = Native::Files;

		constexpr std::string_view ATOMIC_SUFFIX = ".spectmp~";

		struct Wide {
			wchar_t m_Chars[NATIVE_PATH_CAPACITY];
			uint32_t m_Length = 0;
		};

		bool toWide(const PathBuffer& r_Native, Wide& r_Out) noexcept {
			return Paths::toNative(r_Native, r_Out.m_Chars, r_Out.m_Length);
		}

		// Mounted paths go through their mount, native paths pass through. Relative paths have no anchor here.
		bool locate(const PathBuffer& r_Path, MountHandle& r_Mount, PathBuffer& r_Native) noexcept {
			r_Mount = {};
			if (r_Path.m_Kind == PathKind::MOUNTED) {
				r_Mount = Mounts::find(Paths::mountName(r_Path));
				if (!Mounts::isValid(r_Mount)) return false;  // exec NotFound
			}
			return Mounts::resolve(r_Path, r_Native);  // exec InvalidPath for relative
		}

		bool writable(MountHandle v_Mount) noexcept {
			return !Mounts::isValid(v_Mount) || !has(Mounts::flags(v_Mount), MountFlags::READ_ONLY);  // exec ReadOnlyMount
		}

		Native::NativeShare shareOf(const OpenDesc& r_Desc) noexcept {
			const uint8_t policy = uint8_t(r_Desc.m_Share);
			Native::NativeShare share = Native::NativeShare::NONE;
			if (!(policy & uint8_t(SharePolicy::EXCLUSIVE)) && r_Desc.m_Access == Access::READ) share = Native::NativeShare::READ;
			if (policy & uint8_t(SharePolicy::ALLOW_WRITE)) share = share | Native::NativeShare::READ | Native::NativeShare::WRITE;
			if (policy & uint8_t(SharePolicy::ALLOW_DELETE)) share = share | Native::NativeShare::REMOVE;
			return share;
		}

		Native::FileOpenMode modeOf(Disposition v_Disposition) noexcept {
			switch (v_Disposition) {
				case Disposition::OPEN_EXISTING:     return Native::FileOpenMode::OPEN_EXISTING_FILE;
				case Disposition::OPEN_ALWAYS:       return Native::FileOpenMode::OPEN_ALWAYS_FILE;
				case Disposition::CREATE_NEW:        return Native::FileOpenMode::CREATE_NEW_FILE;
				case Disposition::CREATE_ALWAYS:     return Native::FileOpenMode::CREATE_ALWAYS_FILE;
				case Disposition::TRUNCATE_EXISTING: return Native::FileOpenMode::TRUNCATE_EXISTING_FILE;
			}
			return Native::FileOpenMode::OPEN_EXISTING_FILE;
		}

		// v_Mount may be invalid (native path): no node, no mount accounting.
		bool openAt(MountHandle v_Mount, const PathBuffer& r_Path, const PathBuffer& r_Native, const OpenDesc& r_Desc, File& r_Out) noexcept {
			Files::close(r_Out);
			const bool mounted = Mounts::isValid(v_Mount);
			if (r_Desc.m_Access != Access::READ && !writable(v_Mount)) return false;
			if (mounted && !Mounts::acquire(v_Mount)) return false;

			Wide wide;
			const Native::NativeOpenDesc desc{
				r_Desc.m_Access == Access::READ ? Native::FileAccess::READ : r_Desc.m_Access == Access::WRITE ? Native::FileAccess::WRITE : Native::FileAccess::READ_WRITE,
				modeOf(r_Desc.m_Disposition), shareOf(r_Desc),
				r_Desc.m_Mode == IoMode::DIRECT ? Native::NativeBuffering::DIRECT : r_Desc.m_Mode == IoMode::WRITE_THROUGH ? Native::NativeBuffering::WRITE_THROUGH : Native::NativeBuffering::BUFFERED };
			void* native = toWide(r_Native, wide) ? NativeFiles::openNative(wide.m_Chars, desc) : nullptr;
			if (native == nullptr) {
				if (mounted) Mounts::release(v_Mount);
				return false;  // exec from NativeFiles::lastError(): NotFound / AlreadyExists / AccessDenied / SharingViolation
			}

			NodeHandle node{};
			PathTree* tree = mounted ? Mounts::tree(v_Mount) : nullptr;
			if (tree != nullptr && !(Trees::insert(*tree, r_Path, node) && Trees::pin(*tree, node))) {
				(void)NativeFiles::closeNative(native);
				Mounts::release(v_Mount);
				return false;  // exec TreeFull
			}

			r_Out.m_Native = native;
			r_Out.m_Mount = mounted ? v_Mount : MountHandle{};
			r_Out.m_Node = node;
			(void)NativeFiles::getSize(native, r_Out.m_SizeAtOpen);
			r_Out.m_SectorSize = NativeFiles::sectorSize(native);
			r_Out.m_Access = r_Desc.m_Access;
			r_Out.m_Mode = r_Desc.m_Mode;
			r_Out.m_Share = r_Desc.m_Share;
			return true;
		}

		bool aligned(const File& r_File, const void* p_Buffer, size_t v_Bytes, uint64_t v_Offset) noexcept {
			if (r_File.m_Mode != IoMode::DIRECT) return true;
			const uint64_t mask = r_File.m_SectorSize - 1;
			return ((reinterpret_cast<uintptr_t>(p_Buffer) | v_Bytes | v_Offset) & mask) == 0;  // exec InvalidAlignment
		}

		// Keeps the trie in step with a delete or rename done on disk.
		void forgetNode(MountHandle v_Mount, const PathBuffer& r_Path) noexcept {
			PathTree* tree = Mounts::isValid(v_Mount) ? Mounts::tree(v_Mount) : nullptr;
			if (tree == nullptr) return;
			const NodeHandle node = Trees::lookup(*tree, r_Path);
			if (node.m_Index != NODE_NONE) (void)Trees::remove(*tree, node);
		}

		bool createDirectoryChain(const PathBuffer& r_Native, uint32_t v_Depth) noexcept {
			Wide wide;
			if (!toWide(r_Native, wide)) return false;
			if (NativeFiles::createDirectory(wide.m_Chars)) return true;
			if (NativeFiles::lastError() != Native::NativeError::NOT_FOUND || v_Depth > MAX_MOUNTS * 8) return false;
			PathBuffer parent = r_Native;
			Paths::toParent(parent);
			return Paths::view(parent) != Paths::view(r_Native) && createDirectoryChain(parent, v_Depth + 1) && NativeFiles::createDirectory(wide.m_Chars);
		}

		bool writeWhole(const PathBuffer& r_Native, std::span<const std::byte> v_Bytes, bool v_Flush) noexcept {
			File file;
			const OpenDesc desc{ Access::WRITE, Disposition::CREATE_ALWAYS, IoMode::BUFFERED, SharePolicy::EXCLUSIVE };
			if (!openAt({}, r_Native, r_Native, desc, file)) return false;
			size_t written = 0;
			const bool ok = Files::write(file, v_Bytes, 0, written) && (!v_Flush || Files::flush(file));
			Files::close(file);
			return ok;
		}
	}

	File::~File() {
		Files::close(*this);
	}

	File::File(File&& u_Other) noexcept {
		*this = std::move(u_Other);
	}

	File& File::operator=(File&& u_Other) noexcept {
		if (this == &u_Other) return *this;
		Files::close(*this);
		std::memcpy(static_cast<void*>(this), &u_Other, sizeof(File));
		u_Other.m_Native = nullptr;
		u_Other.m_Mount = {};
		u_Other.m_Node = {};
		return *this;
	}

	bool File::isOpen() const noexcept {
		return m_Native != nullptr;
	}

	bool FileReader::read(std::span<std::byte> v_Buffer, size_t& r_Read) noexcept {
		const bool ok = Files::read(*m_File, v_Buffer, m_Position, r_Read);
		m_Position += r_Read;
		return ok;
	}

	bool FileWriter::write(std::span<const std::byte> v_Bytes) noexcept {
		size_t written = 0;
		const bool ok = Files::write(*m_File, v_Bytes, m_Position, written);
		m_Position += written;
		return ok;
	}

	bool Files::open(const PathBuffer& r_Path, const OpenDesc& r_Desc, File& r_Out) noexcept {
		MountHandle mount;
		PathBuffer native;
		return locate(r_Path, mount, native) && openAt(mount, r_Path, native, r_Desc, r_Out);
	}

	bool Files::open(MountHandle v_Mount, const PathBuffer& r_Path, const OpenDesc& r_Desc, File& r_Out) noexcept {
		PathBuffer native;
		return Mounts::resolve(v_Mount, r_Path, native) && openAt(v_Mount, r_Path, native, r_Desc, r_Out);
	}

	void Files::close(File& r_File) noexcept {
		if (r_File.m_Native == nullptr) return;
		const bool closed = NativeFiles::closeNative(r_File.m_Native);
		SPEC_FS_ASSERT(closed);  // err: a failed close means a corrupt handle
		if (r_File.m_Node.m_Index != NODE_NONE)
			if (PathTree* tree = Mounts::tree(r_File.m_Mount)) Trees::unpin(*tree, r_File.m_Node);
		if (Mounts::isValid(r_File.m_Mount)) Mounts::release(r_File.m_Mount);
		r_File.m_Native = nullptr;
		r_File.m_Mount = {};
		r_File.m_Node = {};
	}

	bool Files::read(const File& r_File, std::span<std::byte> v_Buffer, uint64_t v_Offset, size_t& r_Read) noexcept {
		r_Read = 0;
		if (!r_File.isOpen() || r_File.m_Access == Access::WRITE) return false;  // exec InvalidHandle
		if (!aligned(r_File, v_Buffer.data(), v_Buffer.size(), v_Offset)) return false;
		return NativeFiles::readAt(r_File.m_Native, v_Buffer.data(), v_Buffer.size(), v_Offset, r_Read);  // exec ReadFailed
	}

	bool Files::write(const File& r_File, std::span<const std::byte> v_Bytes, uint64_t v_Offset, size_t& r_Written) noexcept {
		r_Written = 0;
		if (!r_File.isOpen() || r_File.m_Access == Access::READ) return false;  // exec InvalidHandle
		if (!aligned(r_File, v_Bytes.data(), v_Bytes.size(), v_Offset)) return false;
		return NativeFiles::writeAt(r_File.m_Native, v_Bytes.data(), v_Bytes.size(), v_Offset, r_Written);  // exec WriteFailed / DiskFull
	}

	bool Files::flush(const File& r_File) noexcept {
		return r_File.isOpen() && NativeFiles::flushNative(r_File.m_Native);
	}

	bool Files::size(const File& r_File, uint64_t& r_Size) noexcept {
		return r_File.isOpen() && NativeFiles::getSize(r_File.m_Native, r_Size);
	}

	bool Files::setSize(const File& r_File, uint64_t v_Size) noexcept {
		return r_File.isOpen() && r_File.m_Access != Access::READ && NativeFiles::setSize(r_File.m_Native, v_Size);
	}

	bool Files::stat(const PathBuffer& r_Path, FileStat& r_Out) noexcept {
		MountHandle mount;
		PathBuffer native;
		Wide wide;
		Native::NativeStat info{};
		if (!locate(r_Path, mount, native) || !toWide(native, wide) || !NativeFiles::stat(wide.m_Chars, info)) return false;
		r_Out = { info.m_Size, info.m_CreationTime, info.m_WriteTime, info.m_IsDirectory, info.m_IsReadOnly, info.m_IsLink };
		return true;
	}

	bool Files::exists(const PathBuffer& r_Path) noexcept {
		FileStat info;
		return stat(r_Path, info);
	}

	bool Files::isFile(const PathBuffer& r_Path) noexcept {
		FileStat info;
		return stat(r_Path, info) && !info.m_IsDirectory;
	}

	bool Files::isDirectory(const PathBuffer& r_Path) noexcept {
		FileStat info;
		return stat(r_Path, info) && info.m_IsDirectory;
	}

	bool Files::readAll(const PathBuffer& r_Path, const SourceHandle& r_Source, Blob& r_Out) noexcept {
		File file;
		uint64_t length = 0;
		if (!open(r_Path, OpenDesc{}, file) || !size(file, length)) return false;
		std::byte* data = static_cast<std::byte*>(Sources::acquire(r_Source, size_t(length)));
		if (data == nullptr) return false;  // exec OutOfMemory
		size_t got = 0;
		if (!read(file, { data, size_t(length) }, 0, got) || got != length) {
			Sources::release(r_Source, data, size_t(length));
			return false;
		}
		r_Out = { data, size_t(length) };
		return true;
	}

	bool Files::readText(const PathBuffer& r_Path, const SourceHandle& r_Source, Blob& r_Out) noexcept {
		File file;
		uint64_t length = 0;
		if (!open(r_Path, OpenDesc{}, file) || !size(file, length)) return false;
		std::byte* data = static_cast<std::byte*>(Sources::acquire(r_Source, size_t(length) + 1));
		if (data == nullptr) return false;  // exec OutOfMemory
		size_t got = 0;
		if (!read(file, { data, size_t(length) }, 0, got) || got != length) {
			Sources::release(r_Source, data, size_t(length) + 1);
			return false;
		}
		size_t start = 0;
		if (got >= 3 && data[0] == std::byte(0xEF) && data[1] == std::byte(0xBB) && data[2] == std::byte(0xBF)) start = 3;
		std::memmove(data, data + start, got - start);
		data[got - start] = std::byte(0);
		r_Out = { data, got - start };
		return true;
	}

	bool Files::writeAll(const PathBuffer& r_Path, std::span<const std::byte> v_Bytes, bool v_Atomic) noexcept {
		MountHandle mount;
		PathBuffer native;
		if (!locate(r_Path, mount, native) || !writable(mount)) return false;
		if (!v_Atomic) {
			File file;
			const OpenDesc desc{ Access::WRITE, Disposition::CREATE_ALWAYS, IoMode::BUFFERED, SharePolicy::EXCLUSIVE };
			size_t written = 0;
			return openAt(mount, r_Path, native, desc, file) && write(file, v_Bytes, 0, written);
		}

		// The temp sibling is opened natively, so it never gets a node; the target's node survives the swap.
		char text[PATH_CAPACITY + 1];
		const std::string_view target = Paths::view(native);
		if (target.size() + ATOMIC_SUFFIX.size() > PATH_CAPACITY) return false;  // exec PathTooLong
		std::memcpy(text, target.data(), target.size());
		std::memcpy(text + target.size(), ATOMIC_SUFFIX.data(), ATOMIC_SUFFIX.size());
		PathBuffer temp;
		Wide tempWide, targetWide;
		if (!Paths::make(temp, { text, target.size() + ATOMIC_SUFFIX.size() }) || !writeWhole(temp, v_Bytes, true)) return false;
		if (toWide(temp, tempWide) && toWide(native, targetWide) && NativeFiles::renameFile(tempWide.m_Chars, targetWide.m_Chars)) return true;
		(void)NativeFiles::deleteFile(tempWide.m_Chars);
		return false;  // exec WriteFailed
	}

	bool Files::writeText(const PathBuffer& r_Path, std::string_view v_Text, bool v_Atomic) noexcept {
		return writeAll(r_Path, std::as_bytes(std::span<const char>(v_Text.data(), v_Text.size())), v_Atomic);
	}

	bool Files::appendAll(const PathBuffer& r_Path, std::span<const std::byte> v_Bytes) noexcept {
		File file;
		uint64_t end = 0;
		size_t written = 0;
		const OpenDesc desc{ Access::WRITE, Disposition::OPEN_ALWAYS, IoMode::BUFFERED, SharePolicy::EXCLUSIVE };
		return open(r_Path, desc, file) && size(file, end) && write(file, v_Bytes, end, written);
	}

	bool Files::copy(const PathBuffer& r_From, const PathBuffer& r_To, bool v_Overwrite) noexcept {
		MountHandle fromMount, toMount;
		PathBuffer fromNative, toNative;
		Wide fromWide, destWide;
		return locate(r_From, fromMount, fromNative) && locate(r_To, toMount, toNative) && writable(toMount)
			&& toWide(fromNative, fromWide) && toWide(toNative, destWide)
			&& NativeFiles::copyFile(fromWide.m_Chars, destWide.m_Chars, v_Overwrite);  // exec AlreadyExists / NotFound
	}

	bool Files::move(const PathBuffer& r_From, const PathBuffer& r_To) noexcept {
		MountHandle fromMount, toMount;
		PathBuffer fromNative, toNative;
		Wide fromWide, destWide;
		if (!locate(r_From, fromMount, fromNative) || !locate(r_To, toMount, toNative) || !writable(fromMount) || !writable(toMount)) return false;
		if (!toWide(fromNative, fromWide) || !toWide(toNative, destWide) || !NativeFiles::renameFile(fromWide.m_Chars, destWide.m_Chars)) return false;

		PathTree* tree = Mounts::isValid(fromMount) ? Mounts::tree(fromMount) : nullptr;
		const NodeHandle node = tree ? Trees::lookup(*tree, r_From) : NodeHandle{};
		if (node.m_Index == NODE_NONE) return true;
		const bool sameMount = fromMount.m_Index == toMount.m_Index && fromMount.m_Generation == toMount.m_Generation;
		forgetNode(toMount, r_To);  // the rename replaced whatever was there
		if (!sameMount || !Trees::rename(*tree, node, r_To)) (void)Trees::remove(*tree, node);
		return true;
	}

	bool Files::remove(const PathBuffer& r_Path) noexcept {
		MountHandle mount;
		PathBuffer native;
		Wide wide;
		if (!locate(r_Path, mount, native) || !writable(mount) || !toWide(native, wide) || !NativeFiles::deleteFile(wide.m_Chars)) return false;
		forgetNode(mount, r_Path);
		return true;
	}

	bool Dirs::create(const PathBuffer& r_Path) noexcept {
		MountHandle mount;
		PathBuffer native;
		return locate(r_Path, mount, native) && writable(mount) && createDirectoryChain(native, 0);
	}

	bool Dirs::remove(const PathBuffer& r_Path) noexcept {
		MountHandle mount;
		PathBuffer native;
		Wide wide;
		if (!locate(r_Path, mount, native) || !writable(mount) || !toWide(native, wide) || !NativeFiles::removeDirectory(wide.m_Chars)) return false;
		forgetNode(mount, r_Path);
		return true;
	}

	bool Dirs::removeAll(const PathBuffer& r_Path) noexcept {
		MountHandle mount;
		PathBuffer native;
		Wide wide;
		if (!locate(r_Path, mount, native) || !writable(mount) || !toWide(native, wide) || !NativeFiles::removeTree(wide.m_Chars)) return false;
		forgetNode(mount, r_Path);
		return true;
	}
}
