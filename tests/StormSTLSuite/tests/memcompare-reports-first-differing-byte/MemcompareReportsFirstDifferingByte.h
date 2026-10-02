#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemcompareReportsFirstDifferingByte final
	: public Hades::Runtime::IFixture<MemcompareReportsFirstDifferingByte, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSizes[] = { 4, 16, 32, 64, 128, 512, 4097 };
	static constexpr size_t kMaxSize = 4097;

	static int signOf(int v_Val) noexcept {
		if (v_Val < 0) return -1;
		if (v_Val > 0) return 1;
		return 0;
	}

public:
	explicit MemcompareReportsFirstDifferingByte(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kMaxSize);
		m_rightBuf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_leftBuf.data(0), size, 99);
			GuardedBuffer::fillDeterministic(m_rightBuf.data(0), size, 99);
			m_leftBuf.resetCanaries();
			m_rightBuf.resetCanaries();

			// Two difference positions: early (pos1) and later (pos2)
			const size_t pos1 = size / 4;
			const size_t pos2 = (size * 3) / 4;

			uint8_t* leftBytes = m_leftBuf.data(0);
			uint8_t* rightBytes = m_rightBuf.data(0);

			// Test case A: Earlier diff decides <0, later diff would have decided >0
			// At pos1: left (0x10) < right (0x20)
			// At pos2: left (0x90) > right (0x30)
			leftBytes[pos1] = 0x10;
			rightBytes[pos1] = 0x20;
			leftBytes[pos2] = 0x90;
			rightBytes[pos2] = 0x30;

			int resA = signOf(Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size));
			int refA = signOf(std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size));
			STL_TEST_CHECK(resA < 0);
			STL_TEST_CHECK(resA == refA);

			// Test case B: Earlier diff decides >0, later diff would have decided <0
			// At pos1: left (0x20) > right (0x10)
			// At pos2: left (0x30) < right (0x90)
			leftBytes[pos1] = 0x20;
			rightBytes[pos1] = 0x10;
			leftBytes[pos2] = 0x30;
			rightBytes[pos2] = 0x90;

			int resB = signOf(Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size));
			int refB = signOf(std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size));
			STL_TEST_CHECK(resB > 0);
			STL_TEST_CHECK(resB == refB);

			STL_TEST_CHECK(m_leftBuf.canariesIntact());
			STL_TEST_CHECK(m_rightBuf.canariesIntact());
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
		return 0;
	}
};
