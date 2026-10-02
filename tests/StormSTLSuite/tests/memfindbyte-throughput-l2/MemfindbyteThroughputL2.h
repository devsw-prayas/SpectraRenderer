#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemfindbyteThroughputL2 final
	: public Hades::Runtime::IFixture<MemfindbyteThroughputL2, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSize = 256 * 1024; // 256 KiB
	static constexpr uint8_t kBaseVal = 0xAA;
	static constexpr uint8_t kNeedle = 0x55;
	static constexpr uint32_t kRepeats = 32;
	uint64_t m_sink{ 0 };

public:
	explicit MemfindbyteThroughputL2(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kSize);
		std::memset(m_buf.data(0), kBaseVal, kSize);
		// Needle placed at the very last byte to force full scan across L2 cache
		m_buf.data(0)[kSize - 1] = kNeedle;
		m_sink = 0;
	}

	void executeImpl() noexcept {
		m_sink = 0;
		for (uint32_t i = 0; i < kRepeats; ++i) {
			const void* res = Stl::Memory::memFindByte(m_buf.data(0), kSize, kNeedle);
			m_sink += reinterpret_cast<uintptr_t>(res);
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_buf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return m_sink;
	}
};
