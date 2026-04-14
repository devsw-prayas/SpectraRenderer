#pragma once

#include "Validator.h"
#include "BufferComparator.h"
#include <Hades/Fixture.h>

namespace Spectra::Validation {

	/**
	 * @brief A specialized Hades Fixture base that provides integrated
	 * validation services. 
	 */
	template<typename D, typename A>
	class ValidationFixture : public Hades::Runtime::IFixture<D, A> {
		using base_ = Hades::Runtime::IFixture<D, A>;

	protected:
		Validator& m_Validator;

	public:
		explicit ValidationFixture(A& ro_Adapter, Validator& ro_Validator) noexcept
			: base_(ro_Adapter), m_Validator(ro_Validator) {
		}

		/**
		 * @brief Performs a bitwise comparison of a result buffer against a reference.
		 * Automatically reports discrepancies to the fixture's registered Validator.
		 */
		bool validateBitwise(const char* p_Label, const void* p_Ref, const void* p_Test, size_t v_Bytes) const noexcept {
			return BufferComparator::compareBitwise(m_Validator, p_Label, p_Ref, p_Test, v_Bytes);
		}

		/**
		 * @brief Performs a numerical comparison of a result buffer against a reference.
		 * Automatically reports discrepancies to the fixture's registered Validator.
		 */
		template<typename T>
		bool validateNumerical(const char* p_Label, const T* p_Ref, const T* p_Test, size_t v_Count, T v_Epsilon) const noexcept {
			return BufferComparator::compareNumerical(m_Validator, p_Label, p_Ref, p_Test, v_Count, v_Epsilon);
		}

		/**
		 * @brief Quick verify hook.
		 */
		bool verify(bool v_Condition, const char* p_ErrorMessage) const noexcept {
			return m_Validator.verify(v_Condition, p_ErrorMessage);
		}
	};

} // namespace Spectra::Validation
