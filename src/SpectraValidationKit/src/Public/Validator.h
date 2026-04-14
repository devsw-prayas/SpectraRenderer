#pragma once

#include "SVK.h"
#include <Logger.h>
#include <Orchestrator.h>

#include <string>
#include <string_view>

namespace Spectra::Validation {

	/**
	 * @brief Generic Validator object that manages a Stratum logger and provides
	 * diagnostic services across the engine.
	 */
	class SVK_API Validator final {
	public:
		/**
		 * @brief Constructs a Validator and registers the provided logger with the Stratum Orchestrator.
		 * @param p_Name Unique name for this validation pass/context.
		 * @param po_Logger Pointer to a Stratum ILogger (user-managed lifetime).
		 */
		explicit Validator(const char* p_Name, Stratum::Logging::ILogger* po_Logger) noexcept;

		~Validator() = default;

		Validator(const Validator&) = delete;
		Validator& operator=(const Validator&) = delete;
		Validator(Validator&&) = delete;
		Validator& operator=(Validator&&) = delete;

		/**
		 * @brief Validates a condition. Emits a Stratum ExceptionEntry on failure.
		 * Triggers a debug break in DEBUG builds if the condition fails.
		 */
		bool verify(
			bool v_Condition,
			const char* p_ErrorMessage,
			const std::source_location& ro_Location = std::source_location::current()) const noexcept;

		/**
		 * @brief Checks a condition. Emits a Stratum LogEntry (Warning) on failure.
		 * Does not trigger a debug break.
		 */
		bool expect(
			bool v_Condition,
			const char* p_WarningMessage,
			const std::source_location& ro_Location = std::source_location::current()) const noexcept;

		/**
		 * @brief Accessor for the validator's name (and its registered logger tag).
		 */
		STRATUM_NODISCARD std::string_view name() const noexcept { return m_name; }

	private:
		char m_name[Stratum::LOGGER_NAME_MAX];
	};

} // namespace Spectra::Validation
