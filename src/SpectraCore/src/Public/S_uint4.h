#pragma once
#include "SpectraCore.h"
#include "SpectraInstrumentation.h"

namespace spectra::core::math {
	class SPECTRA_CORE S_uint4 {
		unsigned char bits;

	public:
		S_uint4(int value = 0);
		[[nodiscard]] int value() const;

		~S_uint4() = default;
		S_uint4(const S_uint4& other);
		S_uint4(S_uint4&& other) noexcept;

		S_uint4& operator=(const S_uint4& other);
		S_uint4& operator=(S_uint4&& other) noexcept;

		void print() const;
		S_uint4 operator+(const S_uint4& other) const;
		S_uint4 operator-(const S_uint4& other) const;
		S_uint4 operator*(const S_uint4& other) const;
		S_uint4 operator/(const S_uint4& other) const;
		S_uint4 operator%(const S_uint4& other) const;

		S_uint4 operator&(const S_uint4& other) const;
		S_uint4 operator|(const S_uint4& other) const;
		S_uint4 operator^(const S_uint4& other) const;
		S_uint4 operator~() const;

		S_uint4 operator<<(const S_uint4& other) const;
		S_uint4 operator>>(const S_uint4& other) const;

		S_uint4 operator+=(const S_uint4& other);
		S_uint4 operator-=(const S_uint4& other);
		S_uint4 operator*=(const S_uint4& other);
		S_uint4 operator/=(const S_uint4& other);
		S_uint4 operator%=(const S_uint4& other);
		S_uint4 operator&=(const S_uint4& other);
		S_uint4 operator|=(const S_uint4& other);
		S_uint4 operator^=(const S_uint4& other);
		S_uint4 operator<<=(const S_uint4& other);
		S_uint4 operator>>=(const S_uint4& other);

		bool operator==(const S_uint4& other) const;
		bool operator!=(const S_uint4& other) const;
		bool operator<(const S_uint4& other) const;
		bool operator>(const S_uint4& other) const;
		bool operator<=(const S_uint4& other) const;
		bool operator>=(const S_uint4& other) const;

		S_uint4 operator++();
		S_uint4 operator++(int);
		S_uint4 operator--();
		S_uint4 operator--(int);

		S_uint4 operator-() const;

		void bitFlip(int pos);
		void setBit(int pos, bool bit);
		[[nodiscard]] bool getBit(int pos) const;

		S_uint4 rol(int shift) const;
		S_uint4 ror(int shift) const;

		friend std::ostream& operator<<(std::ostream& os, const S_uint4& obj);

		operator int()const;
	};
}