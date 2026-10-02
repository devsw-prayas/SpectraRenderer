#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemsetZeroSizeWritesNothing final
	: public Hades::Runtime::IFixture<MemsetZeroSizeWritesNothing, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_testBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kOffsets[] = { 0, 1, 7, 15, 31, 33, 63 };
	static constexpr size_t kBufferSize = 128;

public:
	explicit MemsetZeroSizeWritesNothing(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_testBuf.allocate(kBufferSize);
		m_refBuf.allocate(kBufferSize);
	}

	void executeImpl() noexcept {
		for (size_t offset : kOffsets) {
			GuardedBuffer::fillDeterministic(m_testBuf.data(0), kBufferSize, 123);
			GuardedBuffer::fillDeterministic(m_refBuf.data(0), kBufferSize, 123);
			m_testBuf.resetCanaries();
			m_refBuf.resetCanaries();

			Stl::Memory::memSet(m_testBuf.data(offset), 0xFF, 0);

			// Buffer and canaries must remain completely untouched
			STL_TEST_CHECK(m_testBuf.canariesIntact());
			STL_TEST_CHECK(std::memcmp(m_testBuf.data(0), m_refBuf.data(0), kBufferSize) == 0);
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_testBuf.freeBuffer();
		m_refBuf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
