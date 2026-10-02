#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class PrefetchCallsDoNotAlterMemory final
	: public Hades::Runtime::IFixture<PrefetchCallsDoNotAlterMemory, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_testBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kBufferSize = 4096;

public:
	explicit PrefetchCallsDoNotAlterMemory(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_testBuf.allocate(kBufferSize);
		m_refBuf.allocate(kBufferSize);
		GuardedBuffer::fillDeterministic(m_testBuf.data(0), kBufferSize, 321);
		GuardedBuffer::fillDeterministic(m_refBuf.data(0), kBufferSize, 321);
	}

	void executeImpl() noexcept {
		m_testBuf.resetCanaries();
		m_refBuf.resetCanaries();

		uint8_t* testData = m_testBuf.data(0);

		// Test prefetchRead with locality 0..3 over the buffer
		for (int locality = 0; locality <= 3; ++locality) {
			for (size_t offset = 0; offset < kBufferSize; offset += 64) {
				Stl::Memory::prefetchRead(testData + offset, locality);
			}
		}

		// Test prefetchWrite over the buffer
		for (size_t offset = 0; offset < kBufferSize; offset += 64) {
			Stl::Memory::prefetchWrite(testData + offset);
		}

		// Memory contents and canaries must remain completely unaltered
		STL_TEST_CHECK(m_testBuf.canariesIntact());
		STL_TEST_CHECK(std::memcmp(m_testBuf.data(0), m_refBuf.data(0), kBufferSize) == 0);
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
