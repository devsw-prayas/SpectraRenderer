#include "SpecFsPath.h"

namespace Spectra::FileSystem {
	namespace {
		// Exec reporting lands with Instrumentation's PushException; until then failures only return false.
		bool fail(PathBuffer& r_Out) noexcept {
			r_Out.m_Length = 0;
			r_Out.m_Kind = PathKind::EMPTY;
			r_Out.m_MountEnd = 0;
			r_Out.m_Chars[0] = '\0';
			return false;
		}

		bool isSeparator(char v_Char) noexcept { return v_Char == '/' || v_Char == '\\'; }
		bool isAsciiLetter(char v_Char) noexcept { return (v_Char >= 'a' && v_Char <= 'z') || (v_Char >= 'A' && v_Char <= 'Z'); }
		char foldAscii(char v_Char) noexcept { return (v_Char >= 'A' && v_Char <= 'Z') ? char(v_Char + 32) : v_Char; }

		uint16_t rootLength(const PathBuffer& r_Path) noexcept {
			switch (r_Path.m_Kind) {
				case PathKind::MOUNTED:         return r_Path.m_MountEnd;
				case PathKind::NATIVE_ABSOLUTE: return 3;  // "C:/"
				default:                        return 0;
			}
		}

		bool equalsFolded(std::string_view v_A, std::string_view v_B) noexcept {
			if (v_A.size() != v_B.size()) return false;
			for (size_t i = 0; i < v_A.size(); ++i)
				if (foldAscii(v_A[i]) != foldAscii(v_B[i])) return false;
			return true;
		}

		// Device names are reserved with or without an extension: "nul.txt" opens the null device.
		bool isReservedName(std::string_view v_Segment) noexcept {
			const std::string_view base = v_Segment.substr(0, v_Segment.find('.'));
			for (const char* name : { "CON", "PRN", "AUX", "NUL" })
				if (equalsFolded(base, name)) return true;
			if (base.size() == 4 && base[3] >= '1' && base[3] <= '9')
				return equalsFolded(base.substr(0, 3), "COM") || equalsFolded(base.substr(0, 3), "LPT");
			return false;
		}

		bool isValidUtf8(std::string_view v_Text) noexcept {
			size_t i = 0;
			while (i < v_Text.size()) {
				const uint8_t lead = uint8_t(v_Text[i]);
				const size_t extra = lead < 0x80 ? 0 : (lead >> 5) == 0x6 ? 1 : (lead >> 4) == 0xE ? 2 : (lead >> 3) == 0x1E ? 3 : 4;
				if (extra == 4 || i + extra >= v_Text.size()) return false;
				for (size_t j = 1; j <= extra; ++j)
					if ((uint8_t(v_Text[i + j]) & 0xC0) != 0x80) return false;
				i += extra + 1;
			}
			return true;
		}

		bool isValidSegment(std::string_view v_Segment) noexcept {
			for (const char c : v_Segment)
				if (uint8_t(c) < 0x20 || std::strchr("<>:\"|?*", c) != nullptr) return false;
			const char last = v_Segment.back();
			return last != '.' && last != ' ' && !isReservedName(v_Segment) && isValidUtf8(v_Segment);
		}
		
		// Appends normalised segments of v_Text after the root. '..' never climbs above v_Root.
		bool appendSegments(PathBuffer& r_Path, uint16_t v_Root, std::string_view v_Text) noexcept {
			size_t i = 0;
			while (i < v_Text.size()) {
				while (i < v_Text.size() && isSeparator(v_Text[i])) ++i;
				size_t end = i;
				while (end < v_Text.size() && !isSeparator(v_Text[end])) ++end;
				const std::string_view segment = v_Text.substr(i, end - i);
				i = end;

				if (segment.empty() || segment == ".") continue;
				if (segment == "..") {
					if (r_Path.m_Length == v_Root) return false;  // exec OutsideMount
					uint16_t cut = r_Path.m_Length;
					while (cut > v_Root && r_Path.m_Chars[cut - 1] != '/') --cut;
					r_Path.m_Length = cut > v_Root ? uint16_t(cut - 1) : v_Root;
					continue;
				}
				if (!isValidSegment(segment)) return false;  // exec InvalidPath

				const size_t separator = r_Path.m_Length > v_Root ? 1 : 0;
				if (r_Path.m_Length + separator + segment.size() > PATH_CAPACITY) return false;  // exec PathTooLong
				if (separator) r_Path.m_Chars[r_Path.m_Length++] = '/';
				std::memcpy(r_Path.m_Chars + r_Path.m_Length, segment.data(), segment.size());
				r_Path.m_Length = uint16_t(r_Path.m_Length + segment.size());
			}
			r_Path.m_Chars[r_Path.m_Length] = '\0';
			return true;
		}

		// "name:/" with a 2+ letter name is a mount; a single letter is a drive.
		size_t mountPrefixLength(std::string_view v_Text) noexcept {
			size_t i = 0;
			if (v_Text.empty() || !isAsciiLetter(v_Text[0])) return 0;
			while (i < v_Text.size() && (isAsciiLetter(v_Text[i]) || (v_Text[i] >= '0' && v_Text[i] <= '9') || v_Text[i] == '_')) ++i;
			if (i < 2 || i + 1 >= v_Text.size() || v_Text[i] != ':' || !isSeparator(v_Text[i + 1])) return 0;
			return i + 2;
		}

		bool isDriveRoot(std::string_view v_Text) noexcept {
			return v_Text.size() >= 3 && isAsciiLetter(v_Text[0]) && v_Text[1] == ':' && isSeparator(v_Text[2]);
		}

		size_t fileNameStart(const PathBuffer& r_Path) noexcept {
			const uint16_t root = rootLength(r_Path);
			size_t start = r_Path.m_Length;
			while (start > root && r_Path.m_Chars[start - 1] != '/') --start;
			return start;
		}

		// Dot at the start of a name (".gitignore") is part of the stem, not an extension.
		size_t extensionDot(const PathBuffer& r_Path) noexcept {
			const size_t start = fileNameStart(r_Path);
			for (size_t i = r_Path.m_Length; i > start + 1; --i)
				if (r_Path.m_Chars[i - 1] == '.') return i - 1;
			return r_Path.m_Length;
		}

		bool appendUtf8(char* p_Out, size_t& r_Length, size_t v_Capacity, uint32_t v_Code) noexcept {
			const size_t size = v_Code < 0x80 ? 1 : v_Code < 0x800 ? 2 : v_Code < 0x10000 ? 3 : 4;
			if (r_Length + size > v_Capacity) return false;
			if (size == 1) { p_Out[r_Length++] = char(v_Code); return true; }
			static constexpr uint8_t LEAD[] = { 0, 0, 0xC0, 0xE0, 0xF0 };
			p_Out[r_Length++] = char(LEAD[size] | (v_Code >> (6 * (size - 1))));
			for (size_t i = size - 1; i > 0; --i)
				p_Out[r_Length++] = char(0x80 | ((v_Code >> (6 * (i - 1))) & 0x3F));
			return true;
		}
	}

	bool Paths::make(PathBuffer& r_Out, std::string_view v_Text) noexcept {
		fail(r_Out);
		if (v_Text.empty()) return true;

		uint16_t root = 0;
		if (const size_t mount = mountPrefixLength(v_Text)) {
			std::memcpy(r_Out.m_Chars, v_Text.data(), mount - 1);
			r_Out.m_Chars[mount - 1] = '/';
			r_Out.m_Kind = PathKind::MOUNTED;
			r_Out.m_MountEnd = uint8_t(mount);
			root = uint16_t(mount);
		} else if (isDriveRoot(v_Text)) {
			r_Out.m_Chars[0] = char(v_Text[0] & ~0x20);  // drive letters upper-cased so equal paths compare equal
			r_Out.m_Chars[1] = ':';
			r_Out.m_Chars[2] = '/';
			r_Out.m_Kind = PathKind::NATIVE_ABSOLUTE;
			root = 3;
		} else if (isSeparator(v_Text[0]) || v_Text.find(':') != std::string_view::npos) {
			return fail(r_Out);  // exec InvalidPath: rooted without a drive, UNC, or a stray ':'
		} else {
			r_Out.m_Kind = PathKind::RELATIVE;
		}

		r_Out.m_Length = root;
		if (!appendSegments(r_Out, root, v_Text.substr(root))) return fail(r_Out);
		if (r_Out.m_Kind == PathKind::RELATIVE && r_Out.m_Length == 0) r_Out.m_Kind = PathKind::EMPTY;
		return true;
	}

	bool Paths::makeFromNative(PathBuffer& r_Out, std::wstring_view v_Native) noexcept {
		if (v_Native.substr(0, 4) == L"\\\\?\\") v_Native.remove_prefix(4);

		// Normalisation only shrinks, so anything over 4x capacity in UTF-8 can't fit.
		char utf8[PATH_CAPACITY * 4];
		size_t length = 0;
		for (size_t i = 0; i < v_Native.size(); ++i) {
			uint32_t code = v_Native[i];
			if (code >= 0xD800 && code <= 0xDBFF && i + 1 < v_Native.size() && v_Native[i + 1] >= 0xDC00 && v_Native[i + 1] <= 0xDFFF)
				code = 0x10000 + ((code - 0xD800) << 10) + (v_Native[++i] - 0xDC00);
			else if (code >= 0xD800 && code <= 0xDFFF)
				return fail(r_Out);  // exec InvalidPath: unpaired surrogate
			if (!appendUtf8(utf8, length, sizeof(utf8), code)) return fail(r_Out);  // exec PathTooLong
		}
		return make(r_Out, std::string_view(utf8, length));
	}

	bool Paths::join(PathBuffer& r_Path, std::string_view v_Relative) noexcept {
		if (mountPrefixLength(v_Relative) || isDriveRoot(v_Relative) || (!v_Relative.empty() && isSeparator(v_Relative[0])))
			return false;  // exec InvalidPath

		PathBuffer result = r_Path;
		if (result.m_Kind == PathKind::EMPTY) result.m_Kind = PathKind::RELATIVE;
		if (!appendSegments(result, rootLength(result), v_Relative)) return false;
		if (result.m_Kind == PathKind::RELATIVE && result.m_Length == 0) result.m_Kind = PathKind::EMPTY;
		r_Path = result;
		return true;
	}

	bool Paths::replaceExtension(PathBuffer& r_Path, std::string_view v_Extension) noexcept {
		const size_t nameStart = fileNameStart(r_Path);
		if (nameStart == r_Path.m_Length) return false;  // exec InvalidPath: no file name
		const size_t dot = extensionDot(r_Path);
		const size_t newLength = dot + (v_Extension.empty() ? 0 : v_Extension.size() + 1);
		if (newLength > PATH_CAPACITY) return false;  // exec PathTooLong

		PathBuffer result = r_Path;
		result.m_Length = uint16_t(dot);
		if (!v_Extension.empty()) {
			result.m_Chars[result.m_Length++] = '.';
			std::memcpy(result.m_Chars + result.m_Length, v_Extension.data(), v_Extension.size());
			result.m_Length = uint16_t(newLength);
		}
		result.m_Chars[result.m_Length] = '\0';

		const std::string_view name(result.m_Chars + nameStart, result.m_Length - nameStart);
		if (v_Extension.find_first_of("/\\") != std::string_view::npos || !isValidSegment(name)) return false;  // exec InvalidPath
		r_Path = result;
		return true;
	}

	void Paths::toParent(PathBuffer& r_Path) noexcept {
		const uint16_t root = rootLength(r_Path);
		if (r_Path.m_Length <= root) return;
		const size_t start = fileNameStart(r_Path);
		r_Path.m_Length = uint16_t(start > root ? start - 1 : root);
		r_Path.m_Chars[r_Path.m_Length] = '\0';
		if (r_Path.m_Kind == PathKind::RELATIVE && r_Path.m_Length == 0) r_Path.m_Kind = PathKind::EMPTY;
	}

	std::string_view Paths::view(const PathBuffer& r_Path) noexcept {
		return { r_Path.m_Chars, r_Path.m_Length };
	}

	std::string_view Paths::mountName(const PathBuffer& r_Path) noexcept {
		if (r_Path.m_Kind != PathKind::MOUNTED) return {};
		return { r_Path.m_Chars, size_t(r_Path.m_MountEnd - 2) };
	}

	std::string_view Paths::relativePart(const PathBuffer& r_Path) noexcept {
		const uint16_t root = rootLength(r_Path);
		return { r_Path.m_Chars + root, size_t(r_Path.m_Length - root) };
	}

	std::string_view Paths::fileName(const PathBuffer& r_Path) noexcept {
		const size_t start = fileNameStart(r_Path);
		return { r_Path.m_Chars + start, r_Path.m_Length - start };
	}

	std::string_view Paths::stem(const PathBuffer& r_Path) noexcept {
		const size_t start = fileNameStart(r_Path);
		return { r_Path.m_Chars + start, extensionDot(r_Path) - start };
	}

	std::string_view Paths::extension(const PathBuffer& r_Path) noexcept {
		const size_t dot = extensionDot(r_Path);
		if (dot == r_Path.m_Length) return {};
		return { r_Path.m_Chars + dot + 1, r_Path.m_Length - dot - 1 };
	}

	bool Paths::nextSegment(const PathBuffer& r_Path, uint16_t& r_Offset, std::string_view& r_Segment) noexcept {
		uint16_t start = r_Offset == 0 ? rootLength(r_Path) : r_Offset;
		if (start < r_Path.m_Length && r_Path.m_Chars[start] == '/') ++start;
		if (start >= r_Path.m_Length) return false;

		uint16_t end = start;
		while (end < r_Path.m_Length && r_Path.m_Chars[end] != '/') ++end;
		r_Segment = { r_Path.m_Chars + start, size_t(end - start) };
		r_Offset = end;
		return true;
	}

	bool Paths::equals(const PathBuffer& r_A, const PathBuffer& r_B) noexcept {
		return r_A.m_Kind == r_B.m_Kind && view(r_A) == view(r_B);
	}

	bool Paths::equalsIgnoreCase(const PathBuffer& r_A, const PathBuffer& r_B) noexcept {
		return r_A.m_Kind == r_B.m_Kind && equalsFolded(view(r_A), view(r_B));
	}

	bool Paths::toNative(const PathBuffer& r_Path, wchar_t (&r_Out)[NATIVE_PATH_CAPACITY], uint32_t& r_Length) noexcept {
		r_Length = 0;
		r_Out[0] = L'\0';
		if (r_Path.m_Kind == PathKind::MOUNTED || r_Path.m_Kind == PathKind::EMPTY) return false;  // exec InvalidPath

		// UTF-16 never needs more units than UTF-8 bytes, so 4 + PATH_CAPACITY always fits.
		if (r_Path.m_Kind == PathKind::NATIVE_ABSOLUTE)
			for (const wchar_t c : L"\\\\?\\") if (c) r_Out[r_Length++] = c;

		size_t i = 0;
		while (i < r_Path.m_Length) {
			const uint8_t lead = uint8_t(r_Path.m_Chars[i]);
			const size_t extra = lead < 0x80 ? 0 : lead < 0xE0 ? 1 : lead < 0xF0 ? 2 : 3;
			uint32_t code = extra == 0 ? lead : lead & (0x3F >> extra);
			for (size_t j = 1; j <= extra; ++j) code = (code << 6) | (uint8_t(r_Path.m_Chars[i + j]) & 0x3F);
			i += extra + 1;

			if (code >= 0x10000) {
				code -= 0x10000;
				r_Out[r_Length++] = wchar_t(0xD800 + (code >> 10));
				r_Out[r_Length++] = wchar_t(0xDC00 + (code & 0x3FF));
			} else {
				r_Out[r_Length++] = code == '/' ? L'\\' : wchar_t(code);
			}
		}
		r_Out[r_Length] = L'\0';
		return true;
	}
}
