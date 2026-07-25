#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"
#include "SpecMemDiagnostics.h"
#include "MemoryRegion.h"

namespace Spectra::Memory {
	using Bytes = size_t;
	namespace Literals {
		constexpr Bytes operator""_KB(unsigned long long v_KB) {
			return v_KB * 1000ULL;
		}

		constexpr Bytes operator""_KiB(unsigned long long v_KiB) {
			return v_KiB * 1024ULL;
		}

		constexpr Bytes operator""_MB(unsigned long long v_MB) {
			return v_MB * 1000ULL * 1000ULL;
		}

		constexpr Bytes operator""_MiB(unsigned long long v_MiB) {
			return v_MiB * 1024ULL * 1024ULL;
		}

		constexpr Bytes operator""_GB(unsigned long long v_GB) {
			return v_GB * 1000ULL * 1000ULL * 1000ULL;
		}

		constexpr Bytes operator""_GiB(unsigned long long v_GiB) {
			return v_GiB * 1024ULL * 1024ULL * 1024ULL;
		}
	}

	using namespace Literals;

	constexpr Bytes KILO_BYTE = 1_KB;
	constexpr Bytes MEGA_BYTE = 1_MB;
	constexpr Bytes GIGA_BYTE = 1_GB;

	constexpr Bytes KIBI_BYTE = 1_KiB;
	constexpr Bytes MEBI_BYTE = 1_MiB;
	constexpr Bytes GIBI_BYTE = 1_GiB;

	struct alignas(32) VARegion final {
		uint8_t* m_Base;
		Bytes    m_Size;

		constexpr VARegion() noexcept
			: m_Base(nullptr), m_Size(0) {
		}

		constexpr VARegion(uint8_t* p_Base, Bytes v_Size) noexcept
			: m_Base(p_Base), m_Size(v_Size) {
		}

		SPEC_MEM_NODISCARD
		constexpr bool isValid() const noexcept {
			return m_Base != nullptr && m_Size != 0;
		}

		VARegion(const VARegion&) = default;
		VARegion& operator=(const VARegion&) = default;

		VARegion(VARegion&&) noexcept = default;
		VARegion& operator=(VARegion&&) noexcept = default;

		~VARegion() = default;
	};

	inline constexpr VARegion INVALID_REGION{};

	// Bump-cursor slicer -- carves a reserved+committed MemoryHandle (or an already-carved
	// VARegion) into smaller, page-aligned named sub-regions. Never reserves/commits itself;
	// the handle it slices must already be committed by the caller via MemoryRegion's API.
	class SPEC_MEM_RUNTIME_API VARegionSlicer final {
	public:
		VARegionSlicer() : m_Cursor(nullptr), m_End(nullptr) {}

		explicit VARegionSlicer(const MemoryHandle& ro_Handle) noexcept
			: m_Cursor(static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress)),
			  m_End(static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + ro_Handle.m_Memory.m_TotalSize) {
		}

		explicit VARegionSlicer(const VARegion& ro_Region) noexcept
			: m_Cursor(ro_Region.m_Base),
			  m_End(ro_Region.m_Base + ro_Region.m_Size) {
		}

		~VARegionSlicer() = default;

		VARegionSlicer(VARegionSlicer&&) noexcept = default;
		VARegionSlicer& operator=(VARegionSlicer&&) noexcept = default;

		VARegionSlicer(const VARegionSlicer&) = delete;
		VARegionSlicer& operator=(const VARegionSlicer&) = delete;

		SPEC_MEM_NODISCARD
		VARegion slice(Bytes v_RequestedSize) noexcept;

		SPEC_MEM_NODISCARD
		SPEC_MEM_FORCEINLINE constexpr Bytes remaining() const noexcept {
			return static_cast<Bytes>(m_End - m_Cursor);
		}

	private:
		uint8_t* m_Cursor;
		uint8_t* m_End;
	};

	SPEC_MEM_NODISCARD SPEC_MEM_RUNTIME_API MemoryHandle segmentFromRegion(const VARegion& ro_Region) noexcept;

	// Marks a guard region PAGE_NOACCESS via Memory::protectRegion. Fail-fast on error,
	// same as the rest of the Memory:: primitives -- no bool/error-code return.
	SPEC_MEM_RUNTIME_API void lockGuard(const VARegion& ro_Guard) noexcept;
}
