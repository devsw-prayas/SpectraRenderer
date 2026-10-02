#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemequalThroughputL2 final
	: public Hades::Runtime::IFixture<MemequalThroughputL2, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSize = 256 * 1024; // 256 KiB
	static constexpr uint32_t kRepeats = 32;
	uint64_t m_sink{ 0 };

public:
	explicit MemequalThroughputL2(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kSize);
		m_rightBuf.allocate(kSize);
		GuardedBuffer::fillDeterministic(m_leftBuf.data(0), kSize, 123);
		GuardedBuffer::fillDeterministic(m_rightBuf.data(0), kSize, 123);
		m_sink = 0;
	}

	void executeImpl() noexcept {
		m_sink = 0;
		for (uint32_t i = 0; i < kRepeats; ++i) {
			m_sink += Stl::Memory::memEqual(m_leftBuf.data(0), m_rightBuf.data(0), kSize) ? 1 : 0;
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_leftBuf.freeBuffer();
		m_rightBuf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		const uint8_t* p = m_leftBuf.data(0);
		return (m_sink << 32) ^
			static_cast<uint64_t>(p[0]) ^
			(static_cast<uint64_t>(p[kSize / 2]) << 8) ^
			(static_cast<uint64_t>(p[kSize - 1]) << 16);
	}
};
