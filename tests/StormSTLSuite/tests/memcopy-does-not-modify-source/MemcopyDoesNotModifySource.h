#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyDoesNotModifySource final
	: public Hades::Runtime::IFixture<MemcopyDoesNotModifySource, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_srcRefBuf;
	GuardedBuffer m_dstBuf;
	static constexpr size_t kSize32MiB = 32 * 1024 * 1024;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537,
		kSize32MiB
	};
	static constexpr size_t kMaxSize = kSize32MiB;

public:
	explicit MemcopyDoesNotModifySource(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Allocate once in startupImpl
		m_srcBuf.allocate(kMaxSize);
		m_srcRefBuf.allocate(kMaxSize);
		m_dstBuf.allocate(kMaxSize);

		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kMaxSize, 919);
		GuardedBuffer::fillDeterministic(m_srcRefBuf.data(0), kMaxSize, 919);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			m_srcBuf.resetCanaries();
			m_srcRefBuf.resetCanaries();
			m_dstBuf.resetCanaries();

			Stl::Memory::memCopy(m_dstBuf.data(0), m_srcBuf.data(0), size);

			// Source buffer and canaries must remain completely untouched
			STL_TEST_CHECK(m_srcBuf.canariesIntact());
			STL_TEST_CHECK(std::memcmp(m_srcBuf.data(0), m_srcRefBuf.data(0), kMaxSize) == 0);
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_srcBuf.freeBuffer();
		m_srcRefBuf.freeBuffer();
		m_dstBuf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
