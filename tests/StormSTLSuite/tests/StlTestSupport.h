#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#define STL_TEST_CHECK(expr) \
	do { \
		if (!(expr)) { \
			std::fprintf(stderr, "CHECK FAILED %s:%d: %s\n", __FILE__, __LINE__, #expr); \
			std::abort(); \
		} \
	} while (false)

class GuardedBuffer final {
private:
	uint8_t* m_rawAlloc{ nullptr };
	size_t m_payloadCapacity{ 0 };
	static constexpr size_t kCanarySize = 64;
	static constexpr uint8_t kCanaryByte = 0xCD;

public:
	GuardedBuffer() noexcept = default;

	~GuardedBuffer() noexcept {
		freeBuffer();
	}

	GuardedBuffer(const GuardedBuffer&) = delete;
	GuardedBuffer& operator=(const GuardedBuffer&) = delete;
	GuardedBuffer(GuardedBuffer&&) = delete;
	GuardedBuffer& operator=(GuardedBuffer&&) = delete;

	void allocate(size_t v_Size) noexcept {
		freeBuffer();
		m_payloadCapacity = v_Size;
		const size_t totalBytes = v_Size + 2 * kCanarySize;
		m_rawAlloc = static_cast<uint8_t*>(_aligned_malloc(totalBytes, 64));
		STL_TEST_CHECK(m_rawAlloc != nullptr);
		resetCanaries();
	}

	void freeBuffer() noexcept {
		if (m_rawAlloc != nullptr) {
			_aligned_free(m_rawAlloc);
			m_rawAlloc = nullptr;
		}
		m_payloadCapacity = 0;
	}

	void resetCanaries() noexcept {
		if (m_rawAlloc == nullptr) {
			return;
		}
		std::memset(m_rawAlloc, kCanaryByte, kCanarySize);
		std::memset(m_rawAlloc + kCanarySize + m_payloadCapacity, kCanaryByte, kCanarySize);
	}

	uint8_t* data(size_t v_Offset = 0) noexcept {
		STL_TEST_CHECK(m_rawAlloc != nullptr);
		STL_TEST_CHECK(v_Offset <= m_payloadCapacity);
		return m_rawAlloc + kCanarySize + v_Offset;
	}

	const uint8_t* data(size_t v_Offset = 0) const noexcept {
		STL_TEST_CHECK(m_rawAlloc != nullptr);
		STL_TEST_CHECK(v_Offset <= m_payloadCapacity);
		return m_rawAlloc + kCanarySize + v_Offset;
	}

	bool canariesIntact() const noexcept {
		if (m_rawAlloc == nullptr) {
			return true;
		}
		for (size_t i = 0; i < kCanarySize; ++i) {
			if (m_rawAlloc[i] != kCanaryByte) {
				return false;
			}
		}
		const uint8_t* postCanary = m_rawAlloc + kCanarySize + m_payloadCapacity;
		for (size_t i = 0; i < kCanarySize; ++i) {
			if (postCanary[i] != kCanaryByte) {
				return false;
			}
		}
		return true;
	}

	static void fillDeterministic(void* p_Dst, size_t v_Size, uint32_t v_Seed = 1) noexcept {
		auto* dst = static_cast<uint8_t*>(p_Dst);
		for (size_t i = 0; i < v_Size; ++i) {
			dst[i] = static_cast<uint8_t>((i * 131 + v_Seed) & 0xFF);
		}
	}
};
