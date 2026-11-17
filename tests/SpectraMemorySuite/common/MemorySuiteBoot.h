#pragma once
#include <CoriumRuntime.h>
#include <SpecInstAddrSpace.h>
#include <SpecMemAddrSpace.h>
#include <SpectraHeap.h>
#include <SpectraAllocators.h>
#include <cstdint>

namespace MemorySuite {
	// Same order as SpectraLauncher: Instrumentation's VA must exist before SpectraMemory boots.
	inline bool boot() noexcept {
		static const bool s_Booted = Spectra::Instrumentation::Internal::init()
			&& Spectra::Memory::Internal::init()
			&& (Corium::CoriumRuntime::initRuntime(), true);
		return s_Booted;
	}

	struct XorShift64 {
		uint64_t m_State;
		uint64_t next() noexcept {
			m_State ^= m_State << 13;
			m_State ^= m_State >> 7;
			m_State ^= m_State << 17;
			return m_State;
		}
	};
}
