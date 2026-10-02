#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemequalIdenticalBuffersReturnTrueSizeSweep final
	: public Hades::Runtime::IFixture<MemequalIdenticalBuffersReturnTrueSizeSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_leftBuf;
	GuardedBuffer m_rightBuf;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537
	};
	struct OffsetPair {
		size_t leftOffset;
		size_t rightOffset;
	};
	static constexpr OffsetPair kOffsetPairs[] = {
		{ 0, 0 },
		{ 1, 3 }
	};
	static constexpr size_t kMaxOffset = 3;
	static constexpr size_t kMaxSize = 65537;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemequalIdenticalBuffersReturnTrueSizeSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_leftBuf.allocate(kTotalCapacity);
		m_rightBuf.allocate(kTotalCapacity);
	}

	void executeImpl() noexcept {
		for (const auto& pair : kOffsetPairs) {
			for (size_t size : kSizes) {
				GuardedBuffer::fillDeterministic(m_leftBuf.data(pair.leftOffset), size, 42);
				GuardedBuffer::fillDeterministic(m_rightBuf.data(pair.rightOffset), size, 42);
				m_leftBuf.resetCanaries();
				m_rightBuf.resetCanaries();

				const bool eq = Stl::Memory::memEqual(m_leftBuf.data(pair.leftOffset), m_rightBuf.data(pair.rightOffset), size);

				STL_TEST_CHECK(m_leftBuf.canariesIntact());
				STL_TEST_CHECK(m_rightBuf.canariesIntact());
				STL_TEST_CHECK(eq == true);
			}
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
