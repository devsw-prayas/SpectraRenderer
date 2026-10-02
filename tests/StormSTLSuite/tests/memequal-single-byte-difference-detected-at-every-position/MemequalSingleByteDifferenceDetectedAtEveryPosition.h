#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemequalSingleByteDifferenceDetectedAtEveryPosition final
	: public Hades::Runtime::IFixture<MemequalSingleByteDifferenceDetectedAtEveryPosition, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSizes[] = { 1, 16, 17, 64, 65, 257, 4097 };
	static constexpr size_t kMaxSize = 4097;

public:
	explicit MemequalSingleByteDifferenceDetectedAtEveryPosition(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kMaxSize);
		m_rightBuf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			GuardedBuffer::fillDeterministic(m_leftBuf.data(0), size, 55);
			GuardedBuffer::fillDeterministic(m_rightBuf.data(0), size, 55);
			m_leftBuf.resetCanaries();
			m_rightBuf.resetCanaries();

			// Generate test positions for this size
			std::vector<size_t> testPositions;
			if (size <= 257) {
				testPositions.reserve(size);
				for (size_t p = 0; p < size; ++p) {
					testPositions.push_back(p);
				}
			} else {
				// For 4097: positions 0, 1, every 64th, last
				testPositions.push_back(0);
				testPositions.push_back(1);
				for (size_t p = 64; p < size; p += 64) {
					testPositions.push_back(p);
				}
				if (testPositions.back() != size - 1) {
					testPositions.push_back(size - 1);
				}
			}

			uint8_t* rightBytes = m_rightBuf.data(0);
			for (size_t pos : testPositions) {
				const uint8_t originalVal = rightBytes[pos];
				rightBytes[pos] = static_cast<uint8_t>(originalVal ^ 0xFF);

				const bool eqFlipped = Stl::Memory::memEqual(m_leftBuf.data(0), m_rightBuf.data(0), size);
				STL_TEST_CHECK(eqFlipped == false);

				// Restore and expect true
				rightBytes[pos] = originalVal;
				const bool eqRestored = Stl::Memory::memEqual(m_leftBuf.data(0), m_rightBuf.data(0), size);
				STL_TEST_CHECK(eqRestored == true);
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
