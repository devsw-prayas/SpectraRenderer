#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemcompareUsesUnsignedByteOrdering final
	: public Hades::Runtime::IFixture<MemcompareUsesUnsignedByteOrdering, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSizes[] = { 1, 7, 16, 32, 64, 128, 512 };
	static constexpr size_t kMaxSize = 512;

	static int signOf(int v_Val) noexcept {
		if (v_Val < 0) return -1;
		if (v_Val > 0) return 1;
		return 0;
	}

public:
	explicit MemcompareUsesUnsignedByteOrdering(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kMaxSize);
		m_rightBuf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_leftBuf.data(0), size, 88);
			GuardedBuffer::fillDeterministic(m_rightBuf.data(0), size, 88);
			m_leftBuf.resetCanaries();
			m_rightBuf.resetCanaries();

			std::vector<size_t> testPositions = { 0, size / 2, size - 1 };
			uint8_t* leftBytes = m_leftBuf.data(0);
			uint8_t* rightBytes = m_rightBuf.data(0);

			for (size_t pos : testPositions) {
				const uint8_t origLeft = leftBytes[pos];
				const uint8_t origRight = rightBytes[pos];

				// 0x80 (128 unsigned, -128 signed) vs 0x7F (127 unsigned, 127 signed)
				// Unsigned byte comparison MUST report left > right (> 0)
				leftBytes[pos] = 0x80;
				rightBytes[pos] = 0x7F;

				const int res = Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size);
				const int ref = std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size);

				STL_TEST_CHECK(signOf(res) > 0);
				STL_TEST_CHECK(signOf(res) == signOf(ref));

				// Reverse check: 0x7F vs 0x80 must report left < right (< 0)
				leftBytes[pos] = 0x7F;
				rightBytes[pos] = 0x80;

				const int resRev = Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size);
				const int refRev = std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size);

				STL_TEST_CHECK(signOf(resRev) < 0);
				STL_TEST_CHECK(signOf(resRev) == signOf(refRev));

				// Restore
				leftBytes[pos] = origLeft;
				rightBytes[pos] = origRight;
			}

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
