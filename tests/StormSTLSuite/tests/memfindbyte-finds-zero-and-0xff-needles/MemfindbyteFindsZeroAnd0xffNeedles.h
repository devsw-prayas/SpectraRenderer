#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

#include <vector>

class MemfindbyteFindsZeroAnd0xffNeedles final
	: public Hades::Runtime::IFixture<MemfindbyteFindsZeroAnd0xffNeedles, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSizes[] = { 1, 15, 16, 31, 32, 63, 64, 127, 128, 512, 4097 };
	static constexpr size_t kMaxSize = 4097;

public:
	explicit MemfindbyteFindsZeroAnd0xffNeedles(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		struct TestCase {
			uint8_t baseFill;
			uint8_t needle;
		};
		const TestCase cases[] = {
			{ 0xFF, 0x00 },
			{ 0x00, 0xFF }
		};

		for (const auto& tc : cases) {
			for (size_t size : kSizes) {
				std::memset(m_buf.data(0), tc.baseFill, size);
				m_buf.resetCanaries();

				// Absent check
				const void* resAbsent = Stl::Memory::memFindByte(m_buf.data(0), size, tc.needle);
				const void* refAbsent = std::memchr(m_buf.data(0), tc.needle, size);
				STL_TEST_CHECK(resAbsent == nullptr);
				STL_TEST_CHECK(resAbsent == refAbsent);

				// Present check at first, middle, last
				std::vector<size_t> testPositions = { 0, size / 2, size - 1 };
				uint8_t* bytes = m_buf.data(0);

				for (size_t pos : testPositions) {
					bytes[pos] = tc.needle;

					const void* resFound = Stl::Memory::memFindByte(m_buf.data(0), size, tc.needle);
					const void* refFound = std::memchr(m_buf.data(0), tc.needle, size);
					STL_TEST_CHECK(resFound == refFound);
					STL_TEST_CHECK(resFound == (bytes + pos));

					bytes[pos] = tc.baseFill;
				}

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
