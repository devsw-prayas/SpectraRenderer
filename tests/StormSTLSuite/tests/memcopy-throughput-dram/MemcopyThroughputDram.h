#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyThroughputDram final
	: public Hades::Runtime::IFixture<MemcopyThroughputDram, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_dstBuf;
	static constexpr size_t kSize = 64 * 1024 * 1024; // 64 MiB
	static constexpr uint32_t kRepeats = 1;

public:
	explicit MemcopyThroughputDram(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_srcBuf.allocate(kSize);
		m_dstBuf.allocate(kSize);
		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kSize, 88);
		GuardedBuffer::fillDeterministic(m_dstBuf.data(0), kSize, 99);
	}

	void executeImpl() noexcept {
		for (uint32_t i = 0; i < kRepeats; ++i) {
			Stl::Memory::memCopy(m_dstBuf.data(0), m_srcBuf.data(0), kSize);
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_srcBuf.freeBuffer();
		m_dstBuf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		const uint8_t* p = m_dstBuf.data(0);
		return static_cast<uint64_t>(p[0]) ^
			(static_cast<uint64_t>(p[kSize / 2]) << 8) ^
			(static_cast<uint64_t>(p[kSize - 1]) << 16);
	}
};
