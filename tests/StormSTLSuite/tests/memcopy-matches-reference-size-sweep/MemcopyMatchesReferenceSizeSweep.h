#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyMatchesReferenceSizeSweep final
	: public Hades::Runtime::IFixture<MemcopyMatchesReferenceSizeSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_dstBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537
	};
	static constexpr size_t kMaxSize = 65537;

public:
	explicit MemcopyMatchesReferenceSizeSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_srcBuf.allocate(kMaxSize);
		m_dstBuf.allocate(kMaxSize);
		m_refBuf.allocate(kMaxSize);
		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kMaxSize, 101);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_dstBuf.data(0), kMaxSize, 202);
			GuardedBuffer::fillDeterministic(m_refBuf.data(0), kMaxSize, 202);
			m_srcBuf.resetCanaries();
			m_dstBuf.resetCanaries();
			m_refBuf.resetCanaries();

			Stl::Memory::memCopy(m_dstBuf.data(0), m_srcBuf.data(0), size);
			std::memcpy(m_refBuf.data(0), m_srcBuf.data(0), size);

			STL_TEST_CHECK(m_srcBuf.canariesIntact());
			STL_TEST_CHECK(m_dstBuf.canariesIntact());
			STL_TEST_CHECK(std::memcmp(m_dstBuf.data(0), m_refBuf.data(0), kMaxSize) == 0);
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
