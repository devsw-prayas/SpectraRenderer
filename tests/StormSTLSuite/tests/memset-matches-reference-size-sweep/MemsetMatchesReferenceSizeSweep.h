#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemsetMatchesReferenceSizeSweep final
	: public Hades::Runtime::IFixture<MemsetMatchesReferenceSizeSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_testBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537
	};
	static constexpr size_t kMaxSize = 65537;

public:
	explicit MemsetMatchesReferenceSizeSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_testBuf.allocate(kMaxSize);
		m_refBuf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		constexpr uint8_t fillVal = 0xA5;

		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_testBuf.data(0), kMaxSize, 42);
			GuardedBuffer::fillDeterministic(m_refBuf.data(0), kMaxSize, 42);
			m_testBuf.resetCanaries();
			m_refBuf.resetCanaries();

			Stl::Memory::memSet(m_testBuf.data(0), fillVal, size);
			std::memset(m_refBuf.data(0), fillVal, size);

			STL_TEST_CHECK(m_testBuf.canariesIntact());
			STL_TEST_CHECK(std::memcmp(m_testBuf.data(0), m_refBuf.data(0), kMaxSize) == 0);
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
