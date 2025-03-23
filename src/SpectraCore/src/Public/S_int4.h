#pragma once
#include <ostream>

#include "SpectraCore.h"

namespace spectra {
	namespace core {
		namespace math {
			class SPECTRA_CORE S_int4 {
				unsigned char bits;

			public:
				S_int4(int value = 0);
				[[nodiscard]] int value() const;

				~S_int4() = default;
				S_int4(const S_int4& other);
				S_int4(S_int4&& other) noexcept;

				S_int4& operator=(const S_int4& other);
				S_int4& operator=(S_int4&& other) noexcept;

				void print() const;
				S_int4 operator+(const S_int4& other) const;
				S_int4 operator-(const S_int4& other) const;
				S_int4 operator*(const S_int4& other) const;
				S_int4 operator/(const S_int4& other) const;
				S_int4 operator%(const S_int4& other) const;

				S_int4 operator&(const S_int4& other) const;
				S_int4 operator|(const S_int4& other) const;
				S_int4 operator^(const S_int4& other) const;
				S_int4 operator~() const;

				S_int4 operator<<(const S_int4& other) const;
				S_int4 operator>>(const S_int4& other) const;

				S_int4 operator+=(const S_int4& other);
				S_int4 operator-=(const S_int4& other);
				S_int4 operator*=(const S_int4& other);
				S_int4 operator/=(const S_int4& other);
				S_int4 operator%=(const S_int4& other);
				S_int4 operator&=(const S_int4& other);
				S_int4 operator|=(const S_int4& other);
				S_int4 operator^=(const S_int4& other);
				S_int4 operator<<=(const S_int4& other);
				S_int4 operator>>=(const S_int4& other);

				bool operator==(const S_int4& other) const;
				bool operator!=(const S_int4& other) const;
				bool operator<(const S_int4& other) const;
				bool operator>(const S_int4& other) const;
				bool operator<=(const S_int4& other) const;
				bool operator>=(const S_int4& other) const;

				S_int4 operator++();
				S_int4 operator++(int);
				S_int4 operator--();
				S_int4 operator--(int);

				S_int4 operator-() const;

				void bitFlip(int pos);
				void setBit(int pos, bool bit);
				[[nodiscard]] bool getBit(int pos) const;

				S_int4 rol(int shift) const;
				S_int4 ror(int shift) const;

				friend std::ostream& operator<<(std::ostream& os, const S_int4& obj);

				operator int()const;
			};
		}
	}
}