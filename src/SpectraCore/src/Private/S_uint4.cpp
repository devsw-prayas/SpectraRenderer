#include "S_uint4.h"

namespace spectra::core::math {
	S_uint4::S_uint4(int value) {
		bits = BIT_MASK_4(value);
	}
	int S_uint4::value() const {
		return bits;
	}
	void S_uint4::print() const {
		instrumentation::Instrumentation::log(instrumentation::E_LogLevel::INFO, "spectra::core::math",
			"S_uint4", "Value: {}", instrumentation::E_LogComponent::MATH, value());
	}

	S_uint4::S_uint4(const S_uint4& other) = default;

	S_uint4::S_uint4(S_uint4&& other) noexcept : bits(other.bits) {
		other.bits = 0;
	}

	S_uint4& S_uint4::operator=(const S_uint4& other) {
		if (this != &other) {
			bits = other.bits;
		}
		return *this;
	}

	S_uint4& S_uint4::operator=(S_uint4&& other) noexcept {
		if (this != &other) {
			bits = other.bits;
			other.bits = 0;
		}
		return *this;
	}
}