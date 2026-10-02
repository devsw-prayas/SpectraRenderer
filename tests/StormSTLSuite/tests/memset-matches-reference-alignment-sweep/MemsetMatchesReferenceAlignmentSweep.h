#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemsetMatchesReferenceAlignmentSweep final
	: public Hades::Runtime::IFixture<MemsetMatchesReferenceAlignmentSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_testBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kOffsets[] = { 0, 1, 7, 15, 31, 33, 63 };
	static constexpr size_t kSizes[] = { 1, 31, 64, 65, 4097 };
	static constexpr size_t kMaxOffset = 63;
	static constexpr size_t kMaxSize = 4097;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemsetMatchesReferenceAlignmentSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_testBuf.allocate(kTotalCapacity);
		m_refBuf.allocate(kTotalCapacity);
	}

	void executeImpl() noexcept {
		constexpr uint8_t fillVal = 0x5A;

		for (size_t size : kSizes) {
			for (size_t offset : kOffsets) {
				GuardedBuffer::fillDeterministic(m_testBuf.data(0), kTotalCapacity, 77);
				GuardedBuffer::fillDeterministic(m_refBuf.data(0), kTotalCapacity, 77);
				m_testBuf.resetCanaries();
				m_refBuf.resetCanaries();

				Stl::Memory::memSet(m_testBuf.data(offset), fillVal, size);
				std::memset(m_refBuf.data(offset), fillVal, size);

				STL_TEST_CHECK(m_testBuf.canariesIntact());
				STL_TEST_CHECK(std::memcmp(m_testBuf.data(0), m_refBuf.data(0), kTotalCapacity) == 0);
			}
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
