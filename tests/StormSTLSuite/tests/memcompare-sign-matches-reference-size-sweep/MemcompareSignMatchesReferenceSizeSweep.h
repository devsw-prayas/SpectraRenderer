#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemcompareSignMatchesReferenceSizeSweep final
	: public Hades::Runtime::IFixture<MemcompareSignMatchesReferenceSizeSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537
	};
	static constexpr size_t kMaxSize = 65537;

	static int signOf(int v_Val) noexcept {
		if (v_Val < 0) return -1;
		if (v_Val > 0) return 1;
		return 0;
	}

public:
	explicit MemcompareSignMatchesReferenceSizeSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kMaxSize);
		m_rightBuf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_leftBuf.data(0), size, 66);
			GuardedBuffer::fillDeterministic(m_rightBuf.data(0), size, 66);
			m_leftBuf.resetCanaries();
			m_rightBuf.resetCanaries();

			// Equal buffers -> 0
			const int cmpEqual = Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size);
			const int refEqual = std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size);
			STL_TEST_CHECK(signOf(cmpEqual) == 0);
			STL_TEST_CHECK(signOf(cmpEqual) == signOf(refEqual));
			STL_TEST_CHECK(m_leftBuf.canariesIntact());
			STL_TEST_CHECK(m_rightBuf.canariesIntact());

			if (size == 0) {
				continue;
			}

			// Positions to test: first (0), middle (size / 2), last (size - 1)
			std::vector<size_t> diffPositions = { 0, size / 2, size - 1 };
			uint8_t* leftBytes = m_leftBuf.data(0);
			uint8_t* rightBytes = m_rightBuf.data(0);

			for (size_t pos : diffPositions) {
				const uint8_t origLeft = leftBytes[pos];
				const uint8_t origRight = rightBytes[pos];

				// Direction 1: left < right
				leftBytes[pos] = 0x10;
				rightBytes[pos] = 0x20;
				int testSign1 = signOf(Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size));
				int refSign1 = signOf(std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size));
				STL_TEST_CHECK(testSign1 == refSign1);
				STL_TEST_CHECK(testSign1 < 0);

				// Direction 2: left > right
				leftBytes[pos] = 0x20;
				rightBytes[pos] = 0x10;
				int testSign2 = signOf(Stl::Memory::memCompare(m_leftBuf.data(0), m_rightBuf.data(0), size));
				int refSign2 = signOf(std::memcmp(m_leftBuf.data(0), m_rightBuf.data(0), size));
				STL_TEST_CHECK(testSign2 == refSign2);
				STL_TEST_CHECK(testSign2 > 0);

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
