#pragma once

#include "SpectraInstrumentation.h"
#include "PlatformMemory.h"
#include "SpecInstDiagnostic.h"
#include <utility>
#include <new>

namespace Spectra::Instrumentation::Utils {
	struct SPEC_INST_ALIGNAS(16) SPEC_INST_RUNTIME_API Region final {
		void* m_BaseAddr;
		size_t m_MaxSize;

		constexpr Region(void* p_Addr, size_t v_MaxSize)
			: m_BaseAddr(p_Addr), m_MaxSize(v_MaxSize) {}
		constexpr Region() : Region(nullptr, 0) {}

		Region(const Region&) = default;
		Region& operator=(const Region&) = default;

		Region(Region&&) noexcept = default;
		Region& operator=(Region&&) noexcept = default;

		bool isValid() const {
			return this->m_BaseAddr != nullptr && this->m_MaxSize != 0;
		}
	};

	inline constexpr Region INVALID_REGION{ nullptr, 0 };

	SPEC_INST_RUNTIME_API Region slice(Region& ro_Region, size_t v_Len);
	SPEC_INST_RUNTIME_API void lockGuard(const Region& ro_Guard);

	// Region is pure geometry (base+size); commit-tracking state lives here instead,
	// kept off Region on purpose so no allocator is ever tempted to reuse it as its
	// own bump cursor. m_CommittedSize is a high-water mark only commitPageIfNeeded
	// may mutate.
	struct SPEC_INST_RUNTIME_API RegionHandle final {
		Spectra::Platform::Runtime::Memory::VirtualMemoryHandle m_Memory;
		size_t m_CommittedSize;

		RegionHandle() : m_Memory(), m_CommittedSize(0) {}
		explicit RegionHandle(const Region& ro_Region) : m_Memory(), m_CommittedSize(0) {
			m_Memory.m_BaseAddress = ro_Region.m_BaseAddr;
			m_Memory.m_TotalSize = ro_Region.m_MaxSize;
		}
	};

	// On-demand, per-page commit for regions carved by SpecInstAddrSpace::init(),
	// which start out RESERVE-only. Goes straight through PlatformVirtualMemory
	SPEC_INST_RUNTIME_API bool commitPageIfNeeded(RegionHandle& ro_Handle, size_t v_Offset);

	SPEC_INST_RUNTIME_API RegionHandle createHandle(Region& ro_Region);

	// Single-tier on purpose: one flat block doesn't need SpectraMemory's claim/carve split.
	// Over-budget terminates rather than returning an invalid region.
	class SPEC_INST_RUNTIME_API InstrumentationVACarver final {
		RegionHandle m_Handle;
		size_t m_Watermark = 0;
	public:
		InstrumentationVACarver() = default;
		explicit InstrumentationVACarver(const Region& ro_Region) : m_Handle(ro_Region) {}

		InstrumentationVACarver(const InstrumentationVACarver&) = delete;
		InstrumentationVACarver& operator=(const InstrumentationVACarver&) = delete;

		InstrumentationVACarver(InstrumentationVACarver&&) noexcept = default;
		InstrumentationVACarver& operator=(InstrumentationVACarver&&) noexcept = default;

		// v_Alignment must be a power of two.
		SPEC_INST_NODISCARD Region carve(size_t v_Size, size_t v_Alignment);

		SPEC_INST_NODISCARD size_t used() const { return m_Watermark; }
		SPEC_INST_NODISCARD size_t capacity() const { return m_Handle.m_Memory.m_TotalSize; }
	};
}
