#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyMatchesReferenceAlignmentSweep final
	: public Hades::Runtime::IFixture<MemcopyMatchesReferenceAlignmentSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_dstBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kOffsets[] = { 0, 1, 7, 15, 31, 33, 63 };
	static constexpr size_t kSizes[] = { 1, 33, 128, 129, 4097 };
	static constexpr size_t kMaxOffset = 63;
	static constexpr size_t kMaxSize = 4097;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemcopyMatchesReferenceAlignmentSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_srcBuf.allocate(kTotalCapacity);
		m_dstBuf.allocate(kTotalCapacity);
		m_refBuf.allocate(kTotalCapacity);
		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kTotalCapacity, 303);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			for (size_t dstOffset : kOffsets) {
				for (size_t srcOffset : kOffsets) {
					GuardedBuffer::fillDeterministic(m_dstBuf.data(0), kTotalCapacity, 404);
					GuardedBuffer::fillDeterministic(m_refBuf.data(0), kTotalCapacity, 404);
					m_srcBuf.resetCanaries();
					m_dstBuf.resetCanaries();
					m_refBuf.resetCanaries();

					Stl::Memory::memCopy(m_dstBuf.data(dstOffset), m_srcBuf.data(srcOffset), size);
					std::memcpy(m_refBuf.data(dstOffset), m_srcBuf.data(srcOffset), size);

					STL_TEST_CHECK(m_srcBuf.canariesIntact());
					STL_TEST_CHECK(m_dstBuf.canariesIntact());
					STL_TEST_CHECK(std::memcmp(m_dstBuf.data(0), m_refBuf.data(0), kTotalCapacity) == 0);
				}
			}
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
