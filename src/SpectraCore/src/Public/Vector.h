#pragma once
#include "SpectraCore.h"
#include "AlignedVector.h"
#include "Traits.h"

namespace spectra::core::math {
	template<typename T, size_t dimensions >
	class SPECTRA_CORE Vector {
		static_assert(dimensions > 0, "Vector dimensions must be greater than 0");
		static_assert(stl::is_floating_point<T>(), "Only floating point types are allowed");
		stl::AlignedVector<T> components;

	public:
		Vector();
		~Vector();

		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		Vector& operator=(const Vector& other);
		Vector& operator=(Vector&& other) noexcept;

		Vector(const T* components);
		Vector(std::initializer_list<T> components);
		Vector& operator=(std::initializer_list<T> components);
		Vector(T val);
		template<typename ...Args>
		Vector(Args&&... args) : components{ static_cast<T>(std::forward<Args>(args))... } {
			static_assert(sizeof...(Args) == dimensions, "Number of arguments must match vector dimensions");
		}
	};
}

#include "Vector.inl"