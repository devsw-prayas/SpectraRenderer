#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemfindbyteMatchesMemchrSizeSweep final
	: public Hades::Runtime::IFixture<MemfindbyteMatchesMemchrSizeSweep, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSizes[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024,
		4095, 4096, 4097, 65535, 65536, 65537
	};
	static constexpr size_t kMaxSize = 65537;

public:
	explicit MemfindbyteMatchesMemchrSizeSweep(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		constexpr uint8_t baseFill = 0x55;
		constexpr uint8_t needle = 0xAA;

		for (size_t size : kSizes) {
			std::memset(m_buf.data(0), baseFill, size);
			m_buf.resetCanaries();

			// Test absent needle
			const void* resAbsent = Stl::Memory::memFindByte(m_buf.data(0), size, needle);
			const void* refAbsent = std::memchr(m_buf.data(0), needle, size);
			STL_TEST_CHECK(resAbsent == nullptr);
			STL_TEST_CHECK(resAbsent == refAbsent);
			STL_TEST_CHECK(m_buf.canariesIntact());

			if (size == 0) {
				continue;
			}

			// Test needle placed at first, middle, last position
			std::vector<size_t> testPositions = { 0, size / 2, size - 1 };
			uint8_t* bytes = m_buf.data(0);

			for (size_t pos : testPositions) {
				bytes[pos] = needle;

				const void* resFound = Stl::Memory::memFindByte(m_buf.data(0), size, needle);
				const void* refFound = std::memchr(m_buf.data(0), needle, size);
				STL_TEST_CHECK(resFound == refFound);
				STL_TEST_CHECK(resFound == (bytes + pos));

				// Restore to baseFill
				bytes[pos] = baseFill;
			}

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
