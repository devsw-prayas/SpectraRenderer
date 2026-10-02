#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <StlMemOps.h>
#include "../StlTestSupport.h"

class MemfindbyteConstAndMutableOverloadsAgree final
	: public Hades::Runtime::IFixture<MemfindbyteConstAndMutableOverloadsAgree, Hades::Runtime::NullDeviceAdapter> {
private:
	GuardedBuffer m_buf;
	static constexpr size_t kSizes[] = { 0, 1, 16, 64, 128, 4097 };
	static constexpr size_t kMaxSize = 4097;

public:
	explicit MemfindbyteConstAndMutableOverloadsAgree(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_buf.allocate(kMaxSize);
	}

	void executeImpl() noexcept {
		constexpr uint8_t baseFill = 0x44;
		constexpr uint8_t needle = 0x99;

		for (size_t size : kSizes) {
			std::memset(m_buf.data(0), baseFill, size);
			m_buf.resetCanaries();

			// Case 1: Needle absent
			{
				void* mutPtr = m_buf.data(0);
				const void* constPtr = m_buf.data(0);

				void* resMut = Stl::Memory::memFindByte(mutPtr, size, needle);
				const void* resConst = Stl::Memory::memFindByte(constPtr, size, needle);

				STL_TEST_CHECK(resMut == nullptr);
				STL_TEST_CHECK(resConst == nullptr);
				STL_TEST_CHECK(resMut == resConst);
			}

			if (size == 0) {
				continue;
			}

			// Case 2: Needle present at middle
			{
				uint8_t* mutSlice = m_buf.data(0);
				const size_t pos = size / 2;
				mutSlice[pos] = needle;

				void* mutPtr = mutSlice;
				const void* constPtr = mutSlice;

				void* resMut = Stl::Memory::memFindByte(mutPtr, size, needle);
				const void* resConst = Stl::Memory::memFindByte(constPtr, size, needle);

				STL_TEST_CHECK(resMut != nullptr);
				STL_TEST_CHECK(resMut == resConst);
				STL_TEST_CHECK(resMut == (mutSlice + pos));

				mutSlice[pos] = baseFill;
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
