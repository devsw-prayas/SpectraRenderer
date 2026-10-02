#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemfindbyteAlignmentSweep final
	: public Hades::Runtime::IFixture<MemfindbyteAlignmentSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kOffsets[] = { 0, 1, 7, 15, 31, 33, 63 };
	static constexpr size_t kSizes[] = { 17, 64, 65, 4097 };
	static constexpr size_t kMaxOffset = 63;
	static constexpr size_t kMaxSize = 4097;
	static constexpr size_t kTotalCapacity = kMaxOffset + kMaxSize;

public:
	explicit MemfindbyteAlignmentSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kTotalCapacity);
	}

	void executeImpl() noexcept {
		constexpr uint8_t baseFill = 0x12;
		constexpr uint8_t needle = 0x34;

		for (size_t size : kSizes) {
			for (size_t offset : kOffsets) {
				std::memset(m_buf.data(offset), baseFill, size);
				m_buf.resetCanaries();

				// Needle at last byte
				uint8_t* slice = m_buf.data(offset);
				slice[size - 1] = needle;

				const void* res = Stl::Memory::memFindByte(slice, size, needle);
				const void* ref = std::memchr(slice, needle, size);

				STL_TEST_CHECK(res == ref);
				STL_TEST_CHECK(res == (slice + size - 1));
				STL_TEST_CHECK(m_buf.canariesIntact());
			}
		}
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
		m_buf.freeBuffer();
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
