#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemsetThroughputL2 final
	: public Hades::Runtime::IFixture<MemsetThroughputL2, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSize = 256 * 1024; // 256 KiB
	static constexpr uint32_t kRepeats = 32;

public:
	explicit MemsetThroughputL2(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kSize);
		GuardedBuffer::fillDeterministic(m_buf.data(0), kSize, 22);
	}

	void executeImpl() noexcept {
		for (uint32_t i = 0; i < kRepeats; ++i) {
			Stl::Memory::memSet(m_buf.data(0), static_cast<uint8_t>(i), kSize);
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_buf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		const uint8_t* p = m_buf.data(0);
		return static_cast<uint64_t>(p[0]) ^
			(static_cast<uint64_t>(p[kSize / 2]) << 8) ^
			(static_cast<uint64_t>(p[kSize - 1]) << 16);
	}
};
