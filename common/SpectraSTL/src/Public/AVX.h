#pragma once
#include <cstdint>

#include "SpectraSTL.h"
#include <immintrin.h>

#ifndef FUNC
#define FUNC static inline
#endif


namespace spectra::stl {
    struct SPECTRA_STL AVXUtils {
        // ================= AVX2 Floating-Point Operations (256-bit) =================
        // Single-precision (f32)
        FUNC __m256 load_f32(const float* ptr) { return _mm256_load_ps(ptr); }
        FUNC void store_f32(float* ptr, __m256 vec) { _mm256_store_ps(ptr, vec); }
        FUNC __m256 add_f32(__m256 a, __m256 b) { return _mm256_add_ps(a, b); }
        FUNC __m256 sub_f32(__m256 a, __m256 b) { return _mm256_sub_ps(a, b); }
        FUNC __m256 mul_f32(__m256 a, __m256 b) { return _mm256_mul_ps(a, b); }
        FUNC __m256 div_f32(__m256 a, __m256 b) { return _mm256_div_ps(a, b); }
        FUNC __m256 min_f32(__m256 a, __m256 b) { return _mm256_min_ps(a, b); }
        FUNC __m256 max_f32(__m256 a, __m256 b) { return _mm256_max_ps(a, b); }
        FUNC __m256 abs_f32(__m256 a) { return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), a); }
        FUNC __m256 sqrt_f32(__m256 a) { return _mm256_sqrt_ps(a); }
        FUNC __m256 rsqrt_f32(__m256 a) { return _mm256_rsqrt_ps(a); }
        FUNC __m256 fmadd_f32(__m256 a, __m256 b, __m256 c) { return _mm256_fmadd_ps(a, b, c); }
        FUNC __m256 fmsub_f32(__m256 a, __m256 b, __m256 c) { return _mm256_fmsub_ps(a, b, c); }
        FUNC __m256 fnmadd_f32(__m256 a, __m256 b, __m256 c) { return _mm256_fnmadd_ps(a, b, c); }

        // Double-precision (f64)
        FUNC __m256d load_f64(const double* ptr) { return _mm256_load_pd(ptr); }
        FUNC void store_f64(double* ptr, __m256d vec) { _mm256_store_pd(ptr, vec); }
        FUNC __m256d add_f64(__m256d a, __m256d b) { return _mm256_add_pd(a, b); }
        FUNC __m256d sub_f64(__m256d a, __m256d b) { return _mm256_sub_pd(a, b); }
        FUNC __m256d mul_f64(__m256d a, __m256d b) { return _mm256_mul_pd(a, b); }
        FUNC __m256d div_f64(__m256d a, __m256d b) { return _mm256_div_pd(a, b); }
        FUNC __m256d min_f64(__m256d a, __m256d b) { return _mm256_min_pd(a, b); }
        FUNC __m256d max_f64(__m256d a, __m256d b) { return _mm256_max_pd(a, b); }
        FUNC __m256d abs_f64(__m256d a) { return _mm256_andnot_pd(_mm256_set1_pd(-0.0), a); }
        FUNC __m256d sqrt_f64(__m256d a) { return _mm256_sqrt_pd(a); }
        FUNC __m256d fmadd_f64(__m256d a, __m256d b, __m256d c) { return _mm256_fmadd_pd(a, b, c); }
        FUNC __m256d fmsub_f64(__m256d a, __m256d b, __m256d c) { return _mm256_fmsub_pd(a, b, c); }
        FUNC __m256d fnmadd_f64(__m256d a, __m256d b, __m256d c) { return _mm256_fnmadd_pd(a, b, c); }

        // ================= AVX2 Integer Operations (256-bit) =================
        // 8-bit (i8/u8)
        FUNC __m256i load_i8(const int8_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_i8(int8_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i load_u8(const uint8_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_u8(uint8_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i add_i8(__m256i a, __m256i b) { return _mm256_add_epi8(a, b); }
        FUNC __m256i add_u8(__m256i a, __m256i b) { return _mm256_add_epi8(a, b); }
        FUNC __m256i sub_i8(__m256i a, __m256i b) { return _mm256_sub_epi8(a, b); }
        FUNC __m256i sub_u8(__m256i a, __m256i b) { return _mm256_sub_epi8(a, b); }
        FUNC __m256i mul_i8(__m256i a, __m256i b) {
            __m256i even = _mm256_mullo_epi16(a, b);
            __m256i odd = _mm256_mullo_epi16(_mm256_srli_epi16(a, 8), _mm256_srli_epi16(b, 8));
            return _mm256_or_si256(_mm256_and_si256(even, _mm256_set1_epi16(0xFF)), _mm256_slli_epi16(odd, 8));
        }
        FUNC __m256i mul_u8(__m256i a, __m256i b) { return mul_i8(a, b); }
        FUNC __m256i min_i8(__m256i a, __m256i b) { return _mm256_min_epi8(a, b); }
        FUNC __m256i max_i8(__m256i a, __m256i b) { return _mm256_max_epi8(a, b); }
        FUNC __m256i min_u8(__m256i a, __m256i b) { return _mm256_min_epu8(a, b); }
        FUNC __m256i max_u8(__m256i a, __m256i b) { return _mm256_max_epu8(a, b); }
        FUNC __m256i abs_i8(__m256i a) { return _mm256_abs_epi8(a); }

        // 16-bit (i16/u16)
        FUNC __m256i load_i16(const int16_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_i16(int16_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i load_u16(const uint16_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_u16(uint16_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i add_i16(__m256i a, __m256i b) { return _mm256_add_epi16(a, b); }
        FUNC __m256i add_u16(__m256i a, __m256i b) { return _mm256_add_epi16(a, b); }
        FUNC __m256i sub_i16(__m256i a, __m256i b) { return _mm256_sub_epi16(a, b); }
        FUNC __m256i sub_u16(__m256i a, __m256i b) { return _mm256_sub_epi16(a, b); }
        FUNC __m256i mul_i16(__m256i a, __m256i b) { return _mm256_mullo_epi16(a, b); }
        FUNC __m256i mul_u16(__m256i a, __m256i b) { return _mm256_mullo_epi16(a, b); }
        FUNC __m256i min_i16(__m256i a, __m256i b) { return _mm256_min_epi16(a, b); }
        FUNC __m256i max_i16(__m256i a, __m256i b) { return _mm256_max_epi16(a, b); }
        FUNC __m256i min_u16(__m256i a, __m256i b) { return _mm256_min_epu16(a, b); }
        FUNC __m256i max_u16(__m256i a, __m256i b) { return _mm256_max_epu16(a, b); }
        FUNC __m256i abs_i16(__m256i a) { return _mm256_abs_epi16(a); }

        // 32-bit (i32/u32)
        FUNC __m256i load_i32(const int32_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_i32(int32_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i load_u32(const uint32_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_u32(uint32_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i add_i32(__m256i a, __m256i b) { return _mm256_add_epi32(a, b); }
        FUNC __m256i add_u32(__m256i a, __m256i b) { return _mm256_add_epi32(a, b); }
        FUNC __m256i sub_i32(__m256i a, __m256i b) { return _mm256_sub_epi32(a, b); }
        FUNC __m256i sub_u32(__m256i a, __m256i b) { return _mm256_sub_epi32(a, b); }
        FUNC __m256i mul_i32(__m256i a, __m256i b) { return _mm256_mullo_epi32(a, b); }
        FUNC __m256i mul_u32(__m256i a, __m256i b) { return _mm256_mullo_epi32(a, b); }
        FUNC __m256i min_i32(__m256i a, __m256i b) { return _mm256_min_epi32(a, b); }
        FUNC __m256i max_i32(__m256i a, __m256i b) { return _mm256_max_epi32(a, b); }
        FUNC __m256i min_u32(__m256i a, __m256i b) { return _mm256_min_epu32(a, b); }
        FUNC __m256i max_u32(__m256i a, __m256i b) { return _mm256_max_epu32(a, b); }
        FUNC __m256i abs_i32(__m256i a) { return _mm256_abs_epi32(a); }

        // 64-bit (i64/u64)
        FUNC __m256i load_i64(const int64_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_i64(int64_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i load_u64(const uint64_t* ptr) { return _mm256_load_si256((__m256i*)ptr); }
        FUNC void store_u64(uint64_t* ptr, __m256i vec) { _mm256_store_si256((__m256i*)ptr, vec); }
        FUNC __m256i add_i64(__m256i a, __m256i b) { return _mm256_add_epi64(a, b); }
        FUNC __m256i add_u64(__m256i a, __m256i b) { return _mm256_add_epi64(a, b); }
        FUNC __m256i sub_i64(__m256i a, __m256i b) { return _mm256_sub_epi64(a, b); }
        FUNC __m256i sub_u64(__m256i a, __m256i b) { return _mm256_sub_epi64(a, b); }
        FUNC __m256i mul_i64(__m256i a, __m256i b) {
            __m256i a_lo = _mm256_and_si256(a, _mm256_set1_epi64x(0xFFFFFFFF));
            __m256i b_lo = _mm256_and_si256(b, _mm256_set1_epi64x(0xFFFFFFFF));
            __m256i a_hi = _mm256_srli_epi64(a, 32);
            __m256i b_hi = _mm256_srli_epi64(b, 32);
            __m256i lo = _mm256_mul_epu32(a_lo, b_lo);
            __m256i hi = _mm256_mul_epu32(a_hi, b_hi);
            __m256i mid1 = _mm256_mul_epu32(a_lo, b_hi);
            __m256i mid2 = _mm256_mul_epu32(a_hi, b_lo);
            __m256i mid = _mm256_add_epi64(mid1, mid2);
            return _mm256_add_epi64(lo, _mm256_slli_epi64(_mm256_add_epi64(mid, hi), 32));
        }
        FUNC __m256i mul_u64(__m256i a, __m256i b) { return mul_i64(a, b); }
        FUNC __m256i min_i64(__m256i a, __m256i b) { return _mm256_min_epi64(a, b); }
        FUNC __m256i max_i64(__m256i a, __m256i b) { return _mm256_max_epi64(a, b); }
        FUNC __m256i min_u64(__m256i a, __m256i b) { return _mm256_min_epu64(a, b); }
        FUNC __m256i max_u64(__m256i a, __m256i b) { return _mm256_max_epu64(a, b); }
        FUNC __m256i abs_i64(__m256i a) { return _mm256_abs_epi64(a); }

#ifdef __AVX512F__
        // ================= AVX-512 Floating-Point Operations (512-bit) =================
        // Single-precision (f32)
        FUNC __m512 load_f32_512(const float* ptr) { return _mm512_load_ps(ptr); }
        FUNC void store_f32_512(float* ptr, __m512 vec) { _mm512_store_ps(ptr, vec); }
        FUNC __m512 add_f32_512(__m512 a, __m512 b) { return _mm512_add_ps(a, b); }
        FUNC __m512 sub_f32_512(__m512 a, __m512 b) { return _mm512_sub_ps(a, b); }
        FUNC __m512 mul_f32_512(__m512 a, __m512 b) { return _mm512_mul_ps(a, b); }
        FUNC __m512 div_f32_512(__m512 a, __m512 b) { return _mm512_div_ps(a, b); }
        FUNC __m512 min_f32_512(__m512 a, __m512 b) { return _mm512_min_ps(a, b); }
        FUNC __m512 max_f32_512(__m512 a, __m512 b) { return _mm512_max_ps(a, b); }
        FUNC __m512 abs_f32_512(__m512 a) { return _mm512_abs_ps(a); }
        FUNC __m512 sqrt_f32_512(__m512 a) { return _mm512_sqrt_ps(a); }
        FUNC __m512 fmadd_f32_512(__m512 a, __m512 b, __m512 c) { return _mm512_fmadd_ps(a, b, c); }
        FUNC __m512 fmsub_f32_512(__m512 a, __m512 b, __m512 c) { return _mm512_fmsub_ps(a, b, c); }
        FUNC __m512 fnmadd_f32_512(__m512 a, __m512 b, __m512 c) { return _mm512_fnmadd_ps(a, b, c); }

        // Double-precision (f64)
        FUNC __m512d load_f64_512(const double* ptr) { return _mm512_load_pd(ptr); }
        FUNC void store_f64_512(double* ptr, __m512d vec) { _mm512_store_pd(ptr, vec); }
        FUNC __m512d add_f64_512(__m512d a, __m512d b) { return _mm512_add_pd(a, b); }
        FUNC __m512d sub_f64_512(__m512d a, __m512d b) { return _mm512_sub_pd(a, b); }
        FUNC __m512d mul_f64_512(__m512d a, __m512d b) { return _mm512_mul_pd(a, b); }
        FUNC __m512d div_f64_512(__m512d a, __m512d b) { return _mm512_div_pd(a, b); }
        FUNC __m512d min_f64_512(__m512d a, __m512d b) { return _mm512_min_pd(a, b); }
        FUNC __m512d max_f64_512(__m512d a, __m512d b) { return _mm512_max_pd(a, b); }
        FUNC __m512d abs_f64_512(__m512d a) { return _mm512_abs_pd(a); }
        FUNC __m512d sqrt_f64_512(__m512d a) { return _mm512_sqrt_pd(a); }
        FUNC __m512d fmadd_f64_512(__m512d a, __m512d b, __m512d c) { return _mm512_fmadd_pd(a, b, c); }
        FUNC __m512d fmsub_f64_512(__m512d a, __m512d b, __m512d c) { return _mm512_fmsub_pd(a, b, c); }
        FUNC __m512d fnmadd_f64_512(__m512d a, __m512d b, __m512d c) { return _mm512_fnmadd_pd(a, b, c); }

        // ================= AVX-512 Integer Operations (512-bit) =================
        // 8-bit (i8/u8)
        FUNC __m512i load_i8_512(const int8_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_i8_512(int8_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i load_u8_512(const uint8_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_u8_512(uint8_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i add_i8_512(__m512i a, __m512i b) { return _mm512_add_epi8(a, b); }
        FUNC __m512i add_u8_512(__m512i a, __m512i b) { return _mm512_add_epi8(a, b); }
        FUNC __m512i sub_i8_512(__m512i a, __m512i b) { return _mm512_sub_epi8(a, b); }
        FUNC __m512i sub_u8_512(__m512i a, __m512i b) { return _mm512_sub_epi8(a, b); }
        FUNC __m512i mul_i8_512(__m512i a, __m512i b) {
            __m512i even = _mm512_mullo_epi16(a, b);
            __m512i odd = _mm512_mullo_epi16(_mm512_srli_epi16(a, 8), _mm512_srli_epi16(b, 8));
            return _mm512_or_si512(_mm512_and_si512(even, _mm512_set1_epi16(0xFF)), _mm512_slli_epi16(odd, 8));
        }
        FUNC __m512i mul_u8_512(__m512i a, __m512i b) { return mul_i8_512(a, b); }
        FUNC __m512i min_i8_512(__m512i a, __m512i b) { return _mm512_min_epi8(a, b); }
        FUNC __m512i max_i8_512(__m512i a, __m512i b) { return _mm512_max_epi8(a, b); }
        FUNC __m512i min_u8_512(__m512i a, __m512i b) { return _mm512_min_epu8(a, b); }
        FUNC __m512i max_u8_512(__m512i a, __m512i b) { return _mm512_max_epu8(a, b); }
        FUNC __m512i abs_i8_512(__m512i a) { return _mm512_abs_epi8(a); }

        // 16-bit (i16/u16)
        FUNC __m512i load_i16_512(const int16_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_i16_512(int16_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i load_u16_512(const uint16_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_u16_512(uint16_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i add_i16_512(__m512i a, __m512i b) { return _mm512_add_epi16(a, b); }
        FUNC __m512i add_u16_512(__m512i a, __m512i b) { return _mm512_add_epi16(a, b); }
        FUNC __m512i sub_i16_512(__m512i a, __m512i b) { return _mm512_sub_epi16(a, b); }
        FUNC __m512i sub_u16_512(__m512i a, __m512i b) { return _mm512_sub_epi16(a, b); }
        FUNC __m512i mul_i16_512(__m512i a, __m512i b) { return _mm512_mullo_epi16(a, b); }
        FUNC __m512i mul_u16_512(__m512i a, __m512i b) { return _mm512_mullo_epi16(a, b); }
        FUNC __m512i min_i16_512(__m512i a, __m512i b) { return _mm512_min_epi16(a, b); }
        FUNC __m512i max_i16_512(__m512i a, __m512i b) { return _mm512_max_epi16(a, b); }
        FUNC __m512i min_u16_512(__m512i a, __m512i b) { return _mm512_min_epu16(a, b); }
        FUNC __m512i max_u16_512(__m512i a, __m512i b) { return _mm512_max_epu16(a, b); }
        FUNC __m512i abs_i16_512(__m512i a) { return _mm512_abs_epi16(a); }

        // 32-bit (i32/u32)
        FUNC __m512i load_i32_512(const int32_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_i32_512(int32_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i load_u32_512(const uint32_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_u32_512(uint32_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i add_i32_512(__m512i a, __m512i b) { return _mm512_add_epi32(a, b); }
        FUNC __m512i add_u32_512(__m512i a, __m512i b) { return _mm512_add_epi32(a, b); }
        FUNC __m512i sub_i32_512(__m512i a, __m512i b) { return _mm512_sub_epi32(a, b); }
        FUNC __m512i sub_u32_512(__m512i a, __m512i b) { return _mm512_sub_epi32(a, b); }
        FUNC __m512i mul_i32_512(__m512i a, __m512i b) { return _mm512_mullo_epi32(a, b); }
        FUNC __m512i mul_u32_512(__m512i a, __m512i b) { return _mm512_mullo_epi32(a, b); }
        FUNC __m512i min_i32_512(__m512i a, __m512i b) { return _mm512_min_epi32(a, b); }
        FUNC __m512i max_i32_512(__m512i a, __m512i b) { return _mm512_max_epi32(a, b); }
        FUNC __m512i min_u32_512(__m512i a, __m512i b) { return _mm512_min_epu32(a, b); }
        FUNC __m512i max_u32_512(__m512i a, __m512i b) { return _mm512_max_epu32(a, b); }
        FUNC __m512i abs_i32_512(__m512i a) { return _mm512_abs_epi32(a); }

        // 64-bit (i64/u64)
        FUNC __m512i load_i64_512(const int64_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_i64_512(int64_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i load_u64_512(const uint64_t* ptr) { return _mm512_load_si512((__m512i*)ptr); }
        FUNC void store_u64_512(uint64_t* ptr, __m512i vec) { _mm512_store_si512((__m512i*)ptr, vec); }
        FUNC __m512i add_i64_512(__m512i a, __m512i b) { return _mm512_add_epi64(a, b); }
        FUNC __m512i add_u64_512(__m512i a, __m512i b) { return _mm512_add_epi64(a, b); }
        FUNC __m512i sub_i64_512(__m512i a, __m512i b) { return _mm512_sub_epi64(a, b); }
        FUNC __m512i sub_u64_512(__m512i a, __m512i b) { return _mm512_sub_epi64(a, b); }
        FUNC __m512i mul_i64_512(__m512i a, __m512i b) { return _mm512_mullo_epi64(a, b); }
        FUNC __m512i mul_u64_512(__m512i a, __m512i b) { return _mm512_mullo_epi64(a, b); }
        FUNC __m512i min_i64_512(__m512i a, __m512i b) { return _mm512_min_epi64(a, b); }
        FUNC __m512i max_i64_512(__m512i a, __m512i b) { return _mm512_max_epi64(a, b); }
        FUNC __m512i min_u64_512(__m512i a, __m512i b) { return _mm512_min_epu64(a, b); }
        FUNC __m512i max_u64_512(__m512i a, __m512i b) { return _mm512_max_epu64(a, b); }
        FUNC __m512i abs_i64_512(__m512i a) { return _mm512_abs_epi64(a); }
#endif
    };
}