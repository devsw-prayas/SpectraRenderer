#include "S_int4.h"
#include "SpectraInstrumentation.h"

namespace spectra::core::math {
	S_int4::S_int4(int value) {
		bits = BIT_MASK_4(value);
	}
	int S_int4::value() const {
		return static_cast<int>((bits & 0x08) ? (bits | 0xFFFFFFF0) : bits);
	}
	void S_int4::print() const {
		instrumentation::Instrumentation::log(instrumentation::E_LogLevel::INFO, "spectra::core::math",
			"S_int4", "Value: {}", instrumentation::E_LogComponent::MATH, value());
	}

	S_int4::S_int4(const S_int4& other) = default;

	S_int4::S_int4(S_int4&& other) noexcept : bits(other.bits) {
		other.bits = 0;
	}

	S_int4& S_int4::operator=(const S_int4& other) {
		if (this != &other) {
			bits = other.bits;
		}
		return *this;
	}

	S_int4& S_int4::operator=(S_int4&& other) noexcept {
		if (this != &other) {
			bits = other.bits;
			other.bits = 0;
		}
		return *this;
	}

	S_int4 S_int4::operator+(const S_int4& other) const {
		int a = SIGN_EXTEND_4(bits);
		int b = SIGN_EXTEND_4(other.bits);
		int sum = a + b;
		if (((a & b & ~sum) | (~a & ~b & sum)) & 0x08) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::WARNING, "spectra::core::math", "S_int4",
				"Overflow in addition: {} + {} = {}", instrumentation::E_LogComponent::MATH, a, b, sum);
		}
		return { sum };
	}

	S_int4 S_int4::operator-(const S_int4& other) const {
		int a = SIGN_EXTEND_4(bits);
		int b = SIGN_EXTEND_4(other.bits);
		int diff = a - b;
		if (((a & ~b & ~diff) | (~a & b & diff)) & 0x08) {
		}
		return { diff };
	}

	S_int4 S_int4::operator*(const S_int4& other) const {
		int a = SIGN_EXTEND_4(bits);
		int b = SIGN_EXTEND_4(other.bits);
		int product = a * b;
		if (SIGN_EXTEND_4((BIT_MASK_4(product))) != product) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::WARNING, "spectra::core::math",
				"S_int4", "Overflow in multiplication: {} * {} = {}", instrumentation::E_LogComponent::MATH, a, b, product);
		}
		return { product };
	}

	S_int4 S_int4::operator/(const S_int4& other) const {
		int a = SIGN_EXTEND_4(bits);
		int b = SIGN_EXTEND_4(other.bits);
		if (b == 0) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math", ""
				"S_int4", "Division by zero: {} / {}", instrumentation::E_LogComponent::MATH, a, b);
		}
		if (a == SIGN_EXTEND_4(0x08) && b == SIGN_EXTEND_4(0x0f)) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math",
				"S_int4", "Division overflow: {} / {}", instrumentation::E_LogComponent::MATH, a, b);
		}

		int result = a / b;
		if (SIGN_EXTEND_4((BIT_MASK_4(result))) != result) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::WARNING, "spectra::core::math",
				"S_int4", "Overflow in division: {} / {} = {}", instrumentation::E_LogComponent::MATH, a, b, result);
		}
		return { result };
	}

	S_int4 S_int4::operator%(const S_int4& other) const {
		int a = SIGN_EXTEND_4(bits);
		int b = SIGN_EXTEND_4(other.bits);
		if (b == 0) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math",
				"S_int4", "Division by zero: {} / {}", instrumentation::E_LogComponent::MATH, a, b);
		}
		if (a == SIGN_EXTEND_4(0x08) && b == SIGN_EXTEND_4(0x0f)) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math",
				"S_int4", "Division overflow: {} / {}", instrumentation::E_LogComponent::MATH, a, b);
		}
		int remainder = a % b;
		if (SIGN_EXTEND_4((BIT_MASK_4(remainder))) != remainder) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::WARNING, "spectra::core::math",
				"S_int4", "Overflow in modulo: {} % {} = {}", instrumentation::E_LogComponent::MATH, a, b, remainder);
		}
		return { remainder };
	}

	S_int4 S_int4::operator&(const S_int4& other) const {
		return { bits & other.bits };
	}

	S_int4 S_int4::operator^(const S_int4& other) const {
		return { bits ^ other.bits };
	}

	S_int4 S_int4::operator|(const S_int4& other) const {
		return { bits | other.bits };
	}

	S_int4 S_int4::operator~() const {
		return { ~bits };
	}

	bool S_int4::operator!=(const S_int4& other) const {
		return bits != other.bits;
	}

	S_int4 S_int4::operator<<(const S_int4& other) const {
		return { bits << BIT_MASK_4(other.bits) };
	}

	S_int4 S_int4::operator>>(const S_int4& other) const {
		return { bits >> BIT_MASK_4(other.bits) };
	}

	S_int4 S_int4::operator+=(const S_int4& other) {
		*this = *this + other;
		return *this;
	}

	S_int4 S_int4::operator-=(const S_int4& other) {
		*this = *this - other;
		return *this;
	}

	S_int4 S_int4::operator*=(const S_int4& other) {
		*this = *this * other;
		return *this;
	}

	S_int4 S_int4::operator/=(const S_int4& other) {
		*this = *this / other;
		return *this;
	}

	S_int4 S_int4::operator%=(const S_int4& other) {
		*this = *this % other;
		return *this;
	}

	S_int4 S_int4::operator&=(const S_int4& other) {
		*this = *this & other;
		return *this;
	}

	S_int4 S_int4::operator|=(const S_int4& other) {
		*this = *this | other;
		return *this;
	}

	S_int4 S_int4::operator^=(const S_int4& other) {
		*this = *this ^ other;
		return *this;
	}

	S_int4 S_int4::operator<<=(const S_int4& other) {
		*this = *this << other;
		return *this;
	}

	S_int4 S_int4::operator>>=(const S_int4& other) {
		*this = *this >> other;
		return *this;
	}

	bool S_int4::operator==(const S_int4& other) const {
		return bits == other.bits;
	}

	bool S_int4::operator<(const S_int4& other) const {
		return value() < other.value();
	}

	bool S_int4::operator>(const S_int4& other) const {
		return value() > other.value();
	}

	bool S_int4::operator<=(const S_int4& other) const {
		return value() <= other.value();
	}

	bool S_int4::operator>=(const S_int4& other) const {
		return value() >= other.value();
	}

	S_int4 S_int4::operator++() {
		*this += 1;
		return *this;
	}

	S_int4 S_int4::operator++(int) {
		S_int4 temp = *this;
		*this += 1;
		return temp;
	}

	S_int4 S_int4::operator--() {
		*this -= 1;
		return *this;
	}

	S_int4 S_int4::operator--(int) {
		S_int4 temp = *this;
		*this -= 1;
		return temp;
	}

	S_int4 S_int4::operator-() const {
		return { -value() };
	}

	void S_int4::bitFlip(int pos) {
		if (pos > 3 | pos < 0) {
		}
		bits ^= (1 << pos);
	}

	bool S_int4::getBit(int pos) const {
		if (pos > 3 | pos < 0) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math",
				"S_int4", "Invalid bit position: {}", instrumentation::E_LogComponent::MATH, pos);
		}
		return (bits >> pos) & 1;
	}

	void S_int4::setBit(int pos, bool bit) {
		if (pos > 3 | pos < 0) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR, "spectra::core::math",
				"S_int4", "Invalid bit position: {}", instrumentation::E_LogComponent::MATH, pos);
		}
		int val = value();
		if (val)bits |= 1 << pos;
		else bits &= ~(1 << pos);
	}

	S_int4 S_int4::rol(int shift) const {
		shift %= 4;
		return { (bits << shift) | (bits >> (4 - shift)) };
	}

	S_int4 S_int4::ror(int shift) const {
		shift %= 4;
		return { (bits >> shift) | (bits << (4 - shift)) };
	}

	S_int4::operator int() const {
		return value();
	}
}