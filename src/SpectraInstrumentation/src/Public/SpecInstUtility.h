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

	class SPEC_INST_RUNTIME_API InstrumentationAllocator {
		RegionHandle m_Handle;
		// Logical bump position - deliberately NOT m_Handle.m_CommittedSize, which
		// commitPageIfNeeded owns as the OS-committed high-water mark (same split
		// as SpectraMemory's LinearArena/StackArena).
		size_t m_Cursor;
	public:
		explicit InstrumentationAllocator(const RegionHandle& ro_Handle) : m_Handle(ro_Handle), m_Cursor(0) {}

		void* allocate(size_t v_Bytes);
		void reset() const {}

		template<typename T>
		T* allocate(size_t v_Count) {
			return static_cast<T*>(allocate(v_Count * sizeof(T)));
		}

		template<typename T, typename...Args>
		void construct(T* p_Mem, Args&&...u_Args) {
			new (p_Mem) T(std::forward<Args>(u_Args)...);
		}

		template<typename T>
		void destroy(T* p_Mem) const {
			p_Mem->~T();
		}

		template<typename T>
		T* constructAt(size_t v_Count) {
			T* p_Mem = allocate<T>(v_Count);
			for (size_t i = 0; i < v_Count; ++i) construct(p_Mem + i);
			return p_Mem;
		}
	};
}
