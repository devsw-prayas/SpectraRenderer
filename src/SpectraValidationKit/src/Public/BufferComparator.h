#pragma once

#include <immintrin.h>

namespace Spectra::Validation {

	/**
	 * @brief High-performance comparison engine for large data buffers.
	 * Reports detailed divergence data to Stratum through a Validator.
	 */
	struct SVK_API BufferComparator final {

		/**
		 * @brief Compares two buffers byte-by-byte.
		 * @return true if buffers match exactly.
		 */
		static bool compareBitwise(
			const Validator& ro_Val,
			const char*      p_Label,
			const void*      p_Ref,
			const void*      p_Test,
			size_t           v_Bytes) noexcept {

			if (std::memcmp(p_Ref, p_Test, v_Bytes) == 0) {
				return true;
			}

			const uint8_t* uRef  = static_cast<const uint8_t*>(p_Ref);
			const uint8_t* uTest = static_cast<const uint8_t*>(p_Test);

			size_t i = 0;
			size_t firstFail = 0;
			bool found = false;

			// AVX2 Loop: 32 bytes per iteration
			const size_t vectorizedEnd = v_Bytes & ~static_cast<size_t>(31);
			for (; i < vectorizedEnd; i += 32) {
				__m256i vRef  = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(uRef + i));
				__m256i vTest = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(uTest + i));

				__m256i vCmp  = _mm256_cmpeq_epi8(vRef, vTest);
				uint32_t mask = _mm256_movemask_epi8(vCmp);

				// movemask returns 1 for equal bytes. If mask != 0xFFFFFFFF, we have a mismatch.
				if (mask != 0xFFFFFFFF) {
					// Find the first zero bit (first mismatch)
					// _tzcnt_u32 on ~mask finds the lowest set bit in the inverted mask
#if STRATUM_COMPILER_MSVC
					unsigned long bitIndex = 0;
					_BitScanForward(&bitIndex, ~mask);
					firstFail = i + bitIndex;
#else
					firstFail = i + __builtin_ctz(~mask);
#endif
					found = true;
					break;
				}
			}

			// Scalar tail or fallback if no AVX2 failure found in blocks
			if (!found) {
				for (; i < v_Bytes; ++i) {
					if (uRef[i] != uTest[i]) {
						firstFail = i;
						found = true;
						break;
					}
				}
			}

			if (found) {
				std::stringstream ss;
				ss << "[" << p_Label << "] Bitwise divergence at offset " << firstFail 
				   << ". Expected: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)uRef[firstFail]
				   << ", Found: 0x" << (int)uTest[firstFail];

				ro_Val.verify(false, ss.str().c_str());
				return false;
			}

			return true;
		}

		/**
		 * @brief Compares two floating point buffers with a tolerance (epsilon).
		 * Optimized for float via AVX2.
		 */
		template<typename T>
		static bool compareNumerical(
			const Validator& ro_Val,
			const char*      p_Label,
			const T*         p_Ref,
			const T*         p_Test,
			size_t           v_Count,
			T                v_Epsilon) noexcept {

			if constexpr (std::is_same_v<T, float>) {
				return compareNumericalFloat(ro_Val, p_Label, reinterpret_cast<const float*>(p_Ref), 
											 reinterpret_cast<const float*>(p_Test), v_Count, static_cast<float>(v_Epsilon));
			}

			// Default scalar implementation for other types (double, etc.)
			size_t failCount = 0;
			size_t firstFail = 0;
			T maxError = 0;

			for (size_t i = 0; i < v_Count; ++i) {
				T diff = std::abs(p_Ref[i] - p_Test[i]);
				if (diff > v_Epsilon) {
					if (failCount == 0) firstFail = i;
					if (diff > maxError) maxError = diff;
					failCount++;
				}
			}

			if (failCount == 0) return true;

			return reportNumericalFailure(ro_Val, p_Label, p_Ref[firstFail], p_Test[firstFail], 
										 firstFail, failCount, v_Count, maxError);
		}

	private:
		static bool compareNumericalFloat(
			const Validator& ro_Val,
			const char*      p_Label,
			const float*     p_Ref,
			const float*     p_Test,
			size_t           v_Count,
			float            v_Epsilon) noexcept {

			size_t i = 0;
			size_t failCount = 0;
			size_t firstFail = 0;
			float  maxError  = 0.0f;
			bool   found     = false;

			const __m256 vEps = _mm256_set1_ps(v_Epsilon);
			const __m256 vAbsMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF));
			__m256 vMaxErr = _mm256_setzero_ps();

			const size_t vectorizedEnd = v_Count & ~static_cast<size_t>(7);
			for (; i < vectorizedEnd; i += 8) {
				__m256 vRef  = _mm256_loadu_ps(p_Ref + i);
				__m256 vTest = _mm256_loadu_ps(p_Test + i);

				// abs(ref - test)
				__m256 vDiff = _mm256_sub_ps(vRef, vTest);
				vDiff = _mm256_and_ps(vDiff, vAbsMask);

				// Update global max error
				vMaxErr = _mm256_max_ps(vMaxErr, vDiff);

				// Compare against epsilon
				__m256 vMask = _mm256_cmp_ps(vDiff, vEps, _CMP_GT_OQ);
				uint32_t mask = _mm256_movemask_ps(vMask);

				if (mask != 0) {
					if (!found) {
						// Identify first failing index in this SIMD block
#if STRATUM_COMPILER_MSVC
						unsigned long bitIndex = 0;
						_BitScanForward(&bitIndex, mask);
						firstFail = i + bitIndex;
#else
						firstFail = i + __builtin_ctz(mask);
#endif
						found = true;
					}
					// Increment fail count by number of set bits
#if STRATUM_COMPILER_MSVC
					failCount += __popcnt(mask);
#else
					failCount += __builtin_popcount(mask);
#endif
				}
			}

			// Finalize Max Error from SIMD register
			float errs[8];
			_mm256_storeu_ps(errs, vMaxErr);
			for (int j = 0; j < 8; ++j) if (errs[j] > maxError) maxError = errs[j];

			// Scalar tail
			for (; i < v_Count; ++i) {
				float diff = std::abs(p_Ref[i] - p_Test[i]);
				if (diff > maxError) maxError = diff;
				if (diff > v_Epsilon) {
					if (!found) {
						firstFail = i;
						found = true;
					}
					failCount++;
				}
			}

			if (!found) return true;

			return reportNumericalFailure(ro_Val, p_Label, p_Ref[firstFail], p_Test[firstFail], 
										 firstFail, failCount, v_Count, maxError);
		}

		template<typename T>
		static bool reportNumericalFailure(
			const Validator& ro_Val,
			const char*      p_Label,
			T                v_RefVal,
			T                v_TestVal,
			size_t           v_FirstIdx,
			size_t           v_FailCount,
			size_t           v_TotalCount,
			T                v_MaxError) noexcept {

			std::stringstream ss;
			ss << "[" << p_Label << "] Numerical divergence. " << v_FailCount << "/" << v_TotalCount 
			   << " elements failed. First fail at [" << v_FirstIdx << "]: Ref=" << v_RefVal 
			   << ", Test=" << v_TestVal << " (Diff=" << std::abs(v_RefVal - v_TestVal) 
			   << "). Max Error: " << v_MaxError;

			ro_Val.verify(false, ss.str().c_str());
			return false;
		}
	};

} // namespace Spectra::Validation
