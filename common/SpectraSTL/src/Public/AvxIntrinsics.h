#pragma once
#include "AVX.h"
#include "SpectraSTL.h"
#include "Traits.h"
#include "AlignedVector.h"

namespace spectra::stl {
	/**
	* This struct defines all the high-level methods that utilize the intrinsics by using aligned vectors
	* @note Fully Optimized for AVX-512 or AVX2, with scalar fallback for remaining elements.
	*/
	struct SPECTRA_STL Intrinsics {
		// ================= High-Level Operations for AlignedVector<T> =================

		/**
		* @brief Performs element wise addition of two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param dest Reference to the destination vector, which is modified in place with the sum.
		* @param src Const reference to the source vector to add to dest.
		* @throws std::runtime_error If the sizes of dest and src do not match.
		* @note Optimized for AVX-512 or AVX2, with scalar fallback for remaining elements. Because who hates love speed?
		*/
		template <typename T>
		static void add(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::add_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::add_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::add_i8_512(AVXUtils::load_i8_512(d + i), AVXUtils::load_i8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::add_i16_512(AVXUtils::load_i16_512(d + i), AVXUtils::load_i16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::add_i32_512(AVXUtils::load_i32_512(d + i), AVXUtils::load_i32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::add_i64_512(AVXUtils::load_i64_512(d + i), AVXUtils::load_i64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8_512(d + i, AVXUtils::add_u8_512(AVXUtils::load_u8_512(d + i), AVXUtils::load_u8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16_512(d + i, AVXUtils::add_u16_512(AVXUtils::load_u16_512(d + i), AVXUtils::load_u16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32_512(d + i, AVXUtils::add_u32_512(AVXUtils::load_u32_512(d + i), AVXUtils::load_u32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64_512(d + i, AVXUtils::add_u64_512(AVXUtils::load_u64_512(d + i), AVXUtils::load_u64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::add_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::add_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::add_i8(AVXUtils::load_i8(d + i), AVXUtils::load_i8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::add_i16(AVXUtils::load_i16(d + i), AVXUtils::load_i16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::add_i32(AVXUtils::load_i32(d + i), AVXUtils::load_i32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::add_i64(AVXUtils::load_i64(d + i), AVXUtils::load_i64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8(d + i, AVXUtils::add_u8(AVXUtils::load_u8(d + i), AVXUtils::load_u8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16(d + i, AVXUtils::add_u16(AVXUtils::load_u16(d + i), AVXUtils::load_u16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32(d + i, AVXUtils::add_u32(AVXUtils::load_u32(d + i), AVXUtils::load_u32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64(d + i, AVXUtils::add_u64(AVXUtils::load_u64(d + i), AVXUtils::load_u64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] += s[i];
			}
#endif
		}

		/**
		* @brief Performs element-wise subtraction of two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param dest Reference to the destination vector, which is modified in place with the difference.
		* @param src Const reference to the source vector to subtract from dest.
		* @throws std::runtime_error If the sizes of dest and src do not match.
		* @note Leverages AVX-512 or AVX2 for performance, with scalar cleanup. Subtraction: the unsung hero of math.
		*/
		template <typename T>
		static void subtract(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::sub_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::sub_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::sub_i8_512(AVXUtils::load_i8_512(d + i), AVXUtils::load_i8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::sub_i16_512(AVXUtils::load_i16_512(d + i), AVXUtils::load_i16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::sub_i32_512(AVXUtils::load_i32_512(d + i), AVXUtils::load_i32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::sub_i64_512(AVXUtils::load_i64_512(d + i), AVXUtils::load_i64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8_512(d + i, AVXUtils::sub_u8_512(AVXUtils::load_u8_512(d + i), AVXUtils::load_u8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16_512(d + i, AVXUtils::sub_u16_512(AVXUtils::load_u16_512(d + i), AVXUtils::load_u16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32_512(d + i, AVXUtils::sub_u32_512(AVXUtils::load_u32_512(d + i), AVXUtils::load_u32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64_512(d + i, AVXUtils::sub_u64_512(AVXUtils::load_u64_512(d + i), AVXUtils::load_u64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::sub_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::sub_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::sub_i8(AVXUtils::load_i8(d + i), AVXUtils::load_i8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::sub_i16(AVXUtils::load_i16(d + i), AVXUtils::load_i16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::sub_i32(AVXUtils::load_i32(d + i), AVXUtils::load_i32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::sub_i64(AVXUtils::load_i64(d + i), AVXUtils::load_i64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8(d + i, AVXUtils::sub_u8(AVXUtils::load_u8(d + i), AVXUtils::load_u8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16(d + i, AVXUtils::sub_u16(AVXUtils::load_u16(d + i), AVXUtils::load_u16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32(d + i, AVXUtils::sub_u32(AVXUtils::load_u32(d + i), AVXUtils::load_u32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64(d + i, AVXUtils::sub_u64(AVXUtils::load_u64(d + i), AVXUtils::load_u64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] -= s[i];
			}
#endif
		}

		/**
		* @brief Performs element-wise multiplication of two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param dest Reference to the destination vector, which is modified in place with the product.
		* @param src Const reference to the source vector to multiply with dest.
		* @throws std::runtime_error If the sizes of dest and src do not match.
		* @note Uses AVX-512 or AVX2 intrinsics for vectorized multiplication. Multiply like you mean it!
		*/
		template <typename T>
		static void multiply(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				throw instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::mul_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::mul_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::mul_i8_512(AVXUtils::load_i8_512(d + i), AVXUtils::load_i8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::mul_i16_512(AVXUtils::load_i16_512(d + i), AVXUtils::load_i16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::mul_i32_512(AVXUtils::load_i32_512(d + i), AVXUtils::load_i32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::mul_i64_512(AVXUtils::load_i64_512(d + i), AVXUtils::load_i64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8_512(d + i, AVXUtils::mul_u8_512(AVXUtils::load_u8_512(d + i), AVXUtils::load_u8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16_512(d + i, AVXUtils::mul_u16_512(AVXUtils::load_u16_512(d + i), AVXUtils::load_u16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32_512(d + i, AVXUtils::mul_u32_512(AVXUtils::load_u32_512(d + i), AVXUtils::load_u32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64_512(d + i, AVXUtils::mul_u64_512(AVXUtils::load_u64_512(d + i), AVXUtils::load_u64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::mul_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::mul_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::mul_i8(AVXUtils::load_i8(d + i), AVXUtils::load_i8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::mul_i16(AVXUtils::load_i16(d + i), AVXUtils::load_i16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::mul_i32(AVXUtils::load_i32(d + i), AVXUtils::load_i32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::mul_i64(AVXUtils::load_i64(d + i), AVXUtils::load_i64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8(d + i, AVXUtils::mul_u8(AVXUtils::load_u8(d + i), AVXUtils::load_u8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16(d + i, AVXUtils::mul_u16(AVXUtils::load_u16(d + i), AVXUtils::load_u16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32(d + i, AVXUtils::mul_u32(AVXUtils::load_u32(d + i), AVXUtils::load_u32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64(d + i, AVXUtils::mul_u64(AVXUtils::load_u64(d + i), AVXUtils::load_u64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] *= s[i];
			}
#endif
		}

		/**
		* @brief Performs element-wise division of two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (must be floating-point: float or double).
		* @param dest Reference to the destination vector, which is modified in place with the quotient.
		* @param src Const reference to the source vector to divide dest by.
		* @throws std::runtime_error If the sizes of dest and src do not match or if T is not a floating-point type.
		* @note Optimized with AVX-512 or AVX2; integers cannot be used here
		*/
		template <typename T>
		static void divide(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			if constexpr (!std::is_floating_point_v<T>)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Division is only supported for floating-point types", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::div_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] /= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::div_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] /= s[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::div_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] /= s[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::div_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] /= s[i];
			}
#endif
		}

		/**
		* @brief Performs element-wise minimum operation on two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param dest Reference to the destination vector, which is modified in place with the minimum values.
		* @param src Const reference to the source vector to compare with dest.
		* @throws std::runtime_error If the sizes of dest and src do not match.
		* @note Employs AVX-512 or AVX2 for vectorized min operations. Because small numbers deserve love too.
		*/
		template <typename T>
		static void min(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::min_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::min_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::min_i8_512(AVXUtils::load_i8_512(d + i), AVXUtils::load_i8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::min_i16_512(AVXUtils::load_i16_512(d + i), AVXUtils::load_i16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::min_i32_512(AVXUtils::load_i32_512(d + i), AVXUtils::load_i32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::min_i64_512(AVXUtils::load_i64_512(d + i), AVXUtils::load_i64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8_512(d + i, AVXUtils::min_u8_512(AVXUtils::load_u8_512(d + i), AVXUtils::load_u8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16_512(d + i, AVXUtils::min_u16_512(AVXUtils::load_u16_512(d + i), AVXUtils::load_u16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32_512(d + i, AVXUtils::min_u32_512(AVXUtils::load_u32_512(d + i), AVXUtils::load_u32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64_512(d + i, AVXUtils::min_u64_512(AVXUtils::load_u64_512(d + i), AVXUtils::load_u64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::min_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::min_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::min_i8(AVXUtils::load_i8(d + i), AVXUtils::load_i8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::min_i16(AVXUtils::load_i16(d + i), AVXUtils::load_i16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::min_i32(AVXUtils::load_i32(d + i), AVXUtils::load_i32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::min_i64(AVXUtils::load_i64(d + i), AVXUtils::load_i64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8(d + i, AVXUtils::min_u8(AVXUtils::load_u8(d + i), AVXUtils::load_u8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16(d + i, AVXUtils::min_u16(AVXUtils::load_u16(d + i), AVXUtils::load_u16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32(d + i, AVXUtils::min_u32(AVXUtils::load_u32(d + i), AVXUtils::load_u32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64(d + i, AVXUtils::min_u64(AVXUtils::load_u64(d + i), AVXUtils::load_u64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::min(d[i], s[i]);
			}
#endif
		}

		/**
		* @brief Performs element-wise maximum operation on two aligned vectors, storing the result in the destination vector.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param dest Reference to the destination vector, which is modified in place with the maximum values.
		* @param src Const reference to the source vector to compare with dest.
		* @throws std::runtime_error If the sizes of dest and src do not match.
		* @note Uses AVX-512 or AVX2 intrinsics for speed. Go big or go home, right?
		*/
		template <typename T>
		static void max(AlignedVector<T>& dest, const AlignedVector<T>& src) {
			if (dest.get_size() != src.get_size())
				throw instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* s = src.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::max_f32_512(AVXUtils::load_f32_512(d + i), AVXUtils::load_f32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::max_f64_512(AVXUtils::load_f64_512(d + i), AVXUtils::load_f64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::max_i8_512(AVXUtils::load_i8_512(d + i), AVXUtils::load_i8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::max_i16_512(AVXUtils::load_i16_512(d + i), AVXUtils::load_i16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::max_i32_512(AVXUtils::load_i32_512(d + i), AVXUtils::load_i32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::max_i64_512(AVXUtils::load_i64_512(d + i), AVXUtils::load_i64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8_512(d + i, AVXUtils::max_u8_512(AVXUtils::load_u8_512(d + i), AVXUtils::load_u8_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16_512(d + i, AVXUtils::max_u16_512(AVXUtils::load_u16_512(d + i), AVXUtils::load_u16_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32_512(d + i, AVXUtils::max_u32_512(AVXUtils::load_u32_512(d + i), AVXUtils::load_u32_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64_512(d + i, AVXUtils::max_u64_512(AVXUtils::load_u64_512(d + i), AVXUtils::load_u64_512(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::max_f32(AVXUtils::load_f32(d + i), AVXUtils::load_f32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::max_f64(AVXUtils::load_f64(d + i), AVXUtils::load_f64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::max_i8(AVXUtils::load_i8(d + i), AVXUtils::load_i8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::max_i16(AVXUtils::load_i16(d + i), AVXUtils::load_i16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::max_i32(AVXUtils::load_i32(d + i), AVXUtils::load_i32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::max_i64(AVXUtils::load_i64(d + i), AVXUtils::load_i64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u8(d + i, AVXUtils::max_u8(AVXUtils::load_u8(d + i), AVXUtils::load_u8(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u16(d + i, AVXUtils::max_u16(AVXUtils::load_u16(d + i), AVXUtils::load_u16(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u32(d + i, AVXUtils::max_u32(AVXUtils::load_u32(d + i), AVXUtils::load_u32(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_u64(d + i, AVXUtils::max_u64(AVXUtils::load_u64(d + i), AVXUtils::load_u64(s + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::max(d[i], s[i]);
			}
#endif
		}

		/**
        * @brief Computes the element-wise absolute value of an aligned vector, storing the result in place.
        * @tparam T The data type of the vector elements (must be signed: float, double, int8_t, etc.).
        * @param dest Reference to the vector, which is modified in place with absolute values.
        * @throws std::runtime_error If T is an unsigned type, unsigned abs is a no-go.
        * @note Optimized with AVX-512 or AVX2; scalar fallback for leftovers. Negatives? Not on our watch!
        */
		template <typename T>
		static void abs(AlignedVector<T>& dest) {
			if constexpr (std::is_unsigned_v<T>)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Cannot take the absolute value of an unsigned type", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::abs_f32_512(AVXUtils::load_f32_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::abs_f64_512(AVXUtils::load_f64_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8_512(d + i, AVXUtils::abs_i8_512(AVXUtils::load_i8_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16_512(d + i, AVXUtils::abs_i16_512(AVXUtils::load_i16_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32_512(d + i, AVXUtils::abs_i32_512(AVXUtils::load_i32_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64_512(d + i, AVXUtils::abs_i64_512(AVXUtils::load_i64_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::abs_f32(AVXUtils::load_f32(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::abs_f64(AVXUtils::load_f64(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i8(d + i, AVXUtils::abs_i8(AVXUtils::load_i8(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i16(d + i, AVXUtils::abs_i16(AVXUtils::load_i16(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i32(d + i, AVXUtils::abs_i32(AVXUtils::load_i32(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_i64(d + i, AVXUtils::abs_i64(AVXUtils::load_i64(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::abs(d[i]);
			}
#endif
		}

		/**
		* @brief Computes the element-wise square root of an aligned vector, storing the result in place.
		* @tparam T The data type of the vector elements (must be floating-point: float or double).
		* @param dest Reference to the vector, which is modified in place with square root values.
		* @throws std::runtime_error If T is not a floating-point type, integers cannot handle this kind of radical.
		* @note Uses AVX-512 or AVX2 for vectorized square roots. Math just got a little more grounded.
		*/
		template <typename T>
		static void sqrt(AlignedVector<T>& dest) {
			if constexpr (!std::is_floating_point_v<T>)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Cannot take the square root of a non-floating-point type", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32_512(d + i, AVXUtils::sqrt_f32_512(AVXUtils::load_f32_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::sqrt(d[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64_512(d + i, AVXUtils::sqrt_f64_512(AVXUtils::load_f64_512(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::sqrt(d[i]);
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f32(d + i, AVXUtils::sqrt_f32(AVXUtils::load_f32(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::sqrt(d[i]);
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes)
					AVXUtils::store_f64(d + i, AVXUtils::sqrt_f64(AVXUtils::load_f64(d + i)));
				for (size_t i = avx_end; i < n; ++i) d[i] = std::sqrt(d[i]);
			}
#endif
		}

		/**
		* @brief Computes the dot product of two aligned vectors, returning a scalar result.
		* @tparam T The data type of the vector elements (e.g., float, double, int32_t, etc.).
		* @param a Const reference to the first input vector.
		* @param b Const reference to the second input vector.
		* @return T The scalar result of the dot product (sum of element-wise products).
		* @throws std::runtime_error If the sizes of a and b do not match.
		* @note Optimized with AVX-512 or AVX2, including fused multiply-add where available. A scalar worth celebrating!
		*/
		template <typename T>
		static T dot_product(const AlignedVector<T>& a, const AlignedVector<T>& b) {
			if (a.get_size() != b.get_size())
				throw instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = a.get_size();
			const T* ap = a.begin();
			const T* bp = b.begin();
			T result = 0;
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				__m512 sum = _mm512_setzero_ps();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::fmadd_f32_512(AVXUtils::load_f32_512(ap + i), AVXUtils::load_f32_512(bp + i), sum);
				}
				result = _mm512_reduce_add_ps(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				__m512d sum = _mm512_setzero_pd();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::fmadd_f64_512(AVXUtils::load_f64_512(ap + i), AVXUtils::load_f64_512(bp + i), sum);
				}
				result = _mm512_reduce_add_pd(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i8_512(sum, AVXUtils::mul_i8_512(AVXUtils::load_i8_512(ap + i), AVXUtils::load_i8_512(bp + i)));
				}
				alignas(64) int8_t temp[64];
				AVXUtils::store_i8_512(temp, sum);
				for (int i = 0; i < 64; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i16_512(sum, AVXUtils::mul_i16_512(AVXUtils::load_i16_512(ap + i), AVXUtils::load_i16_512(bp + i)));
				}
				alignas(64) int16_t temp[32];
				AVXUtils::store_i16_512(temp, sum);
				for (int i = 0; i < 32; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i32_512(sum, AVXUtils::mul_i32_512(AVXUtils::load_i32_512(ap + i), AVXUtils::load_i32_512(bp + i)));
				}
				result = _mm512_reduce_add_epi32(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i64_512(sum, AVXUtils::mul_i64_512(AVXUtils::load_i64_512(ap + i), AVXUtils::load_i64_512(bp + i)));
				}
				result = _mm512_reduce_add_epi64(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u8_512(sum, AVXUtils::mul_u8_512(AVXUtils::load_u8_512(ap + i), AVXUtils::load_u8_512(bp + i)));
				}
				alignas(64) uint8_t temp[64];
				AVXUtils::store_u8_512(temp, sum);
				for (int i = 0; i < 64; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u16_512(sum, AVXUtils::mul_u16_512(AVXUtils::load_u16_512(ap + i), AVXUtils::load_u16_512(bp + i)));
				}
				alignas(64) uint16_t temp[32];
				AVXUtils::store_u16_512(temp, sum);
				for (int i = 0; i < 32; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u32_512(sum, AVXUtils::mul_u32_512(AVXUtils::load_u32_512(ap + i), AVXUtils::load_u32_512(bp + i)));
				}
				result = _mm512_reduce_add_epu32(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				__m512i sum = _mm512_setzero_si512();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u64_512(sum, AVXUtils::mul_u64_512(AVXUtils::load_u64_512(ap + i), AVXUtils::load_u64_512(bp + i)));
				}
				result = _mm512_reduce_add_epu64(sum);
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				__m256 sum = _mm256_setzero_ps();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::fmadd_f32(AVXUtils::load_f32(ap + i), AVXUtils::load_f32(bp + i), sum);
				}
				alignas(32) float temp[8];
				AVXUtils::store_f32(temp, sum);
				for (int i = 0; i < 8; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				__m256d sum = _mm256_setzero_pd();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::fmadd_f64(AVXUtils::load_f64(ap + i), AVXUtils::load_f64(bp + i), sum);
				}
				alignas(32) double temp[4];
				AVXUtils::store_f64(temp, sum);
				for (int i = 0; i < 4; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i8(sum, AVXUtils::mul_i8(AVXUtils::load_i8(ap + i), AVXUtils::load_i8(bp + i)));
				}
				alignas(32) int8_t temp[32];
				AVXUtils::store_i8(temp, sum);
				for (int i = 0; i < 32; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i16(sum, AVXUtils::mul_i16(AVXUtils::load_i16(ap + i), AVXUtils::load_i16(bp + i)));
				}
				alignas(32) int16_t temp[16];
				AVXUtils::store_i16(temp, sum);
				for (int i = 0; i < 16; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i32(sum, AVXUtils::mul_i32(AVXUtils::load_i32(ap + i), AVXUtils::load_i32(bp + i)));
				}
				alignas(32) int32_t temp[8];
				AVXUtils::store_i32(temp, sum);
				for (int i = 0; i < 8; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_i64(sum, AVXUtils::mul_i64(AVXUtils::load_i64(ap + i), AVXUtils::load_i64(bp + i)));
				}
				alignas(32) int64_t temp[4];
				AVXUtils::store_i64(temp, sum);
				for (int i = 0; i < 4; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u8(sum, AVXUtils::mul_u8(AVXUtils::load_u8(ap + i), AVXUtils::load_u8(bp + i)));
				}
				alignas(32) uint8_t temp[32];
				AVXUtils::store_u8(temp, sum);
				for (int i = 0; i < 32; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u16(sum, AVXUtils::mul_u16(AVXUtils::load_u16(ap + i), AVXUtils::load_u16(bp + i)));
				}
				alignas(32) uint16_t temp[16];
				AVXUtils::store_u16(temp, sum);
				for (int i = 0; i < 16; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u32(sum, AVXUtils::mul_u32(AVXUtils::load_u32(ap + i), AVXUtils::load_u32(bp + i)));
				}
				alignas(32) uint32_t temp[8];
				AVXUtils::store_u32(temp, sum);
				for (int i = 0; i < 8; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				__m256i sum = _mm256_setzero_si256();
				for (size_t i = 0; i < avx_end; i += lanes) {
					sum = AVXUtils::add_u64(sum, AVXUtils::mul_u64(AVXUtils::load_u64(ap + i), AVXUtils::load_u64(bp + i)));
				}
				alignas(32) uint64_t temp[4];
				AVXUtils::store_u64(temp, sum);
				for (int i = 0; i < 4; ++i) result += temp[i];
				for (size_t i = avx_end; i < n; ++i) result += ap[i] * bp[i];
			}
#endif
			return result;
		}

		/**
		* @brief Performs a fused multiply-add operation (dest = a * b + dest) on aligned vectors, storing the result in dest.
		* @tparam T The data type of the vector elements (must be floating-point: float or double).
		* @param dest Reference to the destination vector, which is modified in place with the result.
		* @param a Const reference to the first input vector (multiplicand).
		* @param b Const reference to the second input vector (multiplier).
		* @throws std::runtime_error If the sizes of dest, a, and b do not match or if T is not a floating-point type.
		* @note Leverages AVX-512 or AVX2 fused multiply-add instructions. Three vectors, one destiny!
		*/
		template <typename T>
		static void fmadd(AlignedVector<T>& dest, const AlignedVector<T>& a, const AlignedVector<T>& b) {
			if (dest.get_size() != a.get_size() || dest.get_size() != b.get_size())
				throw instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Vectors must have the same size", instrumentation::E_LogComponent::CORE, LOCATION);
			if constexpr (!std::is_floating_point_v<T>)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "Intrinsics",
					"Cannot perform a fused multiply-add on a non-floating-point type", instrumentation::E_LogComponent::CORE, LOCATION);
			size_t n = dest.get_size();
			T* d = dest.begin();
			const T* ap = a.begin();
			const T* bp = b.begin();
#ifdef __AVX512F__
			constexpr size_t lanes = sizeof(__m512) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes) {
					AVXUtils::store_f32_512(d + i, AVXUtils::fmadd_f32_512(AVXUtils::load_f32_512(ap + i), AVXUtils::load_f32_512(bp + i), AVXUtils::load_f32_512(d + i)));
				}
				for (size_t i = avx_end; i < n; ++i) d[i] = ap[i] * bp[i] + d[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes) {
					AVXUtils::store_f64_512(d + i, AVXUtils::fmadd_f64_512(AVXUtils::load_f64_512(ap + i), AVXUtils::load_f64_512(bp + i), AVXUtils::load_f64_512(d + i)));
				}
				for (size_t i = avx_end; i < n; ++i) d[i] = ap[i] * bp[i] + d[i];
			}
#else
			constexpr size_t lanes = sizeof(__m256) / sizeof(T);
			size_t avx_end = n & ~(lanes - 1);
			if constexpr (std::is_same_v<T, float>) {
				for (size_t i = 0; i < avx_end; i += lanes) {
					AVXUtils::store_f32(d + i, AVXUtils::fmadd_f32(AVXUtils::load_f32(ap + i), AVXUtils::load_f32(bp + i), AVXUtils::load_f32(d + i)));
				}
				for (size_t i = avx_end; i < n; ++i) d[i] = ap[i] * bp[i] + d[i];
			}
			else if constexpr (std::is_same_v<T, double>) {
				for (size_t i = 0; i < avx_end; i += lanes) {
					AVXUtils::store_f64(d + i, AVXUtils::fmadd_f64(AVXUtils::load_f64(ap + i), AVXUtils::load_f64(bp + i), AVXUtils::load_f64(d + i)));
				}
				for (size_t i = avx_end; i < n; ++i) d[i] = ap[i] * bp[i] + d[i];
			}
#endif
		}
	};
};
