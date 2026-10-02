#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemsetLargeStreamingPathMatchesReference final
	: public Hades::Runtime::IFixture<MemsetLargeStreamingPathMatchesReference, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_testBuf;
	GuardedBuffer m_refBuf;
	static constexpr size_t kSize32MiB = 32 * 1024 * 1024;
	static constexpr size_t kSizes[] = { kSize32MiB, kSize32MiB + 37 };
	static constexpr size_t kOffsets[] = { 0, 1 };
	static constexpr uint8_t kValues[] = { 0x00, 0xFF };
	static constexpr size_t kMaxOffset = 1;
	static constexpr size_t kMaxSize = kSize32MiB + 37;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemsetLargeStreamingPathMatchesReference(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// Allocate large buffers once in startupImpl
		m_testBuf.allocate(kTotalCapacity);
		m_refBuf.allocate(kTotalCapacity);
	}

	void executeImpl() noexcept {
		for (size_t size : kSizes) {
			for (size_t offset : kOffsets) {
				for (uint8_t val : kValues) {
					GuardedBuffer::fillDeterministic(m_testBuf.data(0), kTotalCapacity, 99);
					GuardedBuffer::fillDeterministic(m_refBuf.data(0), kTotalCapacity, 99);
					m_testBuf.resetCanaries();
					m_refBuf.resetCanaries();

					Stl::Memory::memSet(m_testBuf.data(offset), val, size);
					std::memset(m_refBuf.data(offset), val, size);

					STL_TEST_CHECK(m_testBuf.canariesIntact());
					STL_TEST_CHECK(std::memcmp(m_testBuf.data(0), m_refBuf.data(0), kTotalCapacity) == 0);
				}
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
