#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemcopyLargeStreamingPathMatchesReference final
	: public Hades::Runtime::IFixture<MemcopyLargeStreamingPathMatchesReference, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_srcBuf;
	GuardedBuffer m_dstBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kSize32MiB = 32 * 1024 * 1024;
	static constexpr size_t kSizes[] = { kSize32MiB, kSize32MiB + 37 };
	struct OffsetPair {
		size_t dstOffset;
		size_t srcOffset;
	};
	static constexpr OffsetPair kOffsetPairs[] = {
		{ 0, 0 },
		{ 0, 1 },
		{ 1, 0 },
		{ 17, 33 }
	};
	static constexpr size_t kMaxOffset = 33;
	static constexpr size_t kMaxSize = kSize32MiB + 37;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemcopyLargeStreamingPathMatchesReference(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Allocate large buffers once in startupImpl
		m_srcBuf.allocate(kTotalCapacity);
		m_dstBuf.allocate(kTotalCapacity);
		m_refBuf.allocate(kTotalCapacity);
		GuardedBuffer::fillDeterministic(m_srcBuf.data(0), kTotalCapacity, 505);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			for (const auto& pair : kOffsetPairs) {
				GuardedBuffer::fillDeterministic(m_dstBuf.data(0), kTotalCapacity, 606);
				GuardedBuffer::fillDeterministic(m_refBuf.data(0), kTotalCapacity, 606);
				m_srcBuf.resetCanaries();
				m_dstBuf.resetCanaries();
				m_refBuf.resetCanaries();

				Stl::Memory::memCopy(m_dstBuf.data(pair.dstOffset), m_srcBuf.data(pair.srcOffset), size);
				std::memcpy(m_refBuf.data(pair.dstOffset), m_srcBuf.data(pair.srcOffset), size);

				STL_TEST_CHECK(m_srcBuf.canariesIntact());
				STL_TEST_CHECK(m_dstBuf.canariesIntact());
				STL_TEST_CHECK(std::memcmp(m_dstBuf.data(0), m_refBuf.data(0), kTotalCapacity) == 0);
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
