#include "Validator.h"

namespace Spectra::Validation {

	Validator::Validator(const char* p_Name, Stratum::Logging::ILogger* po_Logger) noexcept {
		std::strncpy(m_name, p_Name, Stratum::LOGGER_NAME_MAX - 1);
		m_name[Stratum::LOGGER_NAME_MAX - 1] = '\0';
		
		// Register the logger with the global orchestrator under this name
		Stratum::Logging::Orchestrator::getInstance().registerLogger(m_name, po_Logger);
	}

	bool Validator::verify(
		bool v_Condition,
		const char* p_ErrorMessage,
		const std::source_location& ro_Location) const noexcept {
		
		if (STRATUM_LIKELY(v_Condition)) {
			return true;
		}

		// Prepare exception entry
		Stratum::Records::ExceptionEntry entry(SVK_COMPONENT, p_ErrorMessage, ro_Location);
		
		// Dispatch to orchestrator using our registered logger name
		Stratum::Logging::Orchestrator::getInstance().exception(m_name, m_name, entry);

		// Trigger debug break if possible
		STRATUM_ASSERT(v_Condition);

		return false;
	}

	bool Validator::expect(
		bool v_Condition,
		const char* p_WarningMessage,
		const std::source_location& ro_Location) const noexcept {

		if (STRATUM_LIKELY(v_Condition)) {
			return true;
		}

		// Prepare log entry with Warning level
		Stratum::Records::LogEntry entry(Stratum::Records::LogLevel::Warning, SVK_COMPONENT, p_WarningMessage, ro_Location);

		// Dispatch to orchestrator
		Stratum::Logging::Orchestrator::getInstance().log(
			Stratum::Records::LogLevel::Warning,
			m_name,
			SVK_COMPONENT,
			m_name,
			p_WarningMessage,
			ro_Location);

		return false;
	}

} // namespace Spectra::Validation
