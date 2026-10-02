#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemfindbyteReturnsFirstOfMultipleOccurrences final
	: public Hades::Runtime::IFixture<MemfindbyteReturnsFirstOfMultipleOccurrences, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSizes[] = { 32, 64, 128, 256, 1024, 4097 };
	static constexpr size_t kMaxSize = 4097;

public:
	explicit MemfindbyteReturnsFirstOfMultipleOccurrences(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		constexpr uint8_t baseFill = 0x33;
		constexpr uint8_t needle = 0x77;

		for (size_t size : kSizes) {
			std::memset(m_buf.data(0), baseFill, size);
			m_buf.resetCanaries();

			// Place needle at multiple positions across vector lane boundaries
			// e.g., first at posFirst, then later at posSecond, posThird...
			const size_t posFirst = size / 8;
			const size_t posSecond = size / 4;
			const size_t posThird = size / 2;
			const size_t posFourth = size - 1;

			uint8_t* bytes = m_buf.data(0);
			bytes[posFirst] = needle;
			bytes[posSecond] = needle;
			bytes[posThird] = needle;
			bytes[posFourth] = needle;

			const void* res = Stl::Memory::memFindByte(m_buf.data(0), size, needle);
			const void* ref = std::memchr(m_buf.data(0), needle, size);

			STL_TEST_CHECK(res == ref);
			STL_TEST_CHECK(res == (bytes + posFirst));
			STL_TEST_CHECK(m_buf.canariesIntact());
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
