#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyZeroSizeWritesNothing final
	: public Hades::Runtime::IFixture<MemcopyZeroSizeWritesNothing, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_dstBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kOffsets[] = { 0, 1, 7, 15, 31, 33, 63 };
	static constexpr size_t kBufferSize = 128;

public:
	explicit MemcopyZeroSizeWritesNothing(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_srcBuf.allocate(kBufferSize);
		m_dstBuf.allocate(kBufferSize);
		m_refBuf.allocate(kBufferSize);
		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kBufferSize, 707);
	}

	void executeImpl() noexcept {
		for (size_t dstOffset : kOffsets) {
			for (size_t srcOffset : kOffsets) {
				GuardedBuffer::fillDeterministic(m_dstBuf.data(0), kBufferSize, 808);
				GuardedBuffer::fillDeterministic(m_refBuf.data(0), kBufferSize, 808);
				m_srcBuf.resetCanaries();
				m_dstBuf.resetCanaries();
				m_refBuf.resetCanaries();

				Stl::Memory::memCopy(m_dstBuf.data(dstOffset), m_srcBuf.data(srcOffset), 0);

				// Destination buffer and canaries must remain completely untouched
				STL_TEST_CHECK(m_srcBuf.canariesIntact());
				STL_TEST_CHECK(m_dstBuf.canariesIntact());
				STL_TEST_CHECK(std::memcmp(m_dstBuf.data(0), m_refBuf.data(0), kBufferSize) == 0);
			}
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_srcBuf.freeBuffer();
		m_dstBuf.freeBuffer();
		m_refBuf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
