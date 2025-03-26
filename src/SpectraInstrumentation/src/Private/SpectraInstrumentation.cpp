#include "SpectraInstrumentation.h"
#include <iostream>
#include <chrono>
#include <format>

/** @brief Placeholder function for initializing Spectra instrumentation.
	@note Currently empty; intended for future initialization logic outside the namespace. */
void SPEC_INSTRUMENTATION SpectraInstrumentationInit() {}

/** @namespace spectra::instrumentation
	@brief Namespace for logging and instrumentation utilities within the Spectra framework. */
namespace spectra::instrumentation {
	/** @brief Initializes the logging system by starting flush threads for all loggers.
		@details Launches asynchronous flush threads for mathLogger and benchmarkLogger. */
	void Instrumentation::init() {
		mathLogger.flushThread = std::thread(&BaseLogger::flushWorker, &mathLogger);
		benchmarkLogger.flushThread = std::thread(&BaseLogger::flushWorker, &benchmarkLogger);
	}

	/** @brief Shuts down the logging system, stopping flush threads and cleaning up resources.
		@details Signals flush threads to stop, notifies them to process remaining logs, and joins them. */
	void Instrumentation::shutdown() {
		{
			std::lock_guard<std::mutex> lock(mathLogger.flushMutex);
			mathLogger.keepRunning = false;
			mathLogger.flushCV.notify_one();
		}
		mathLogger.flushThread.join();

		{
			std::lock_guard<std::mutex> lock(benchmarkLogger.flushMutex);
			benchmarkLogger.keepRunning = false;
			benchmarkLogger.flushCV.notify_one();
		}
		benchmarkLogger.flushThread.join();
	}

	/** @brief Retrieves the ANSI color code corresponding to a log level.
		@param level Log severity level.
		@return ANSI color code string for the specified level. */
	static std::string getColor(E_LogLevel level) {
		switch (level) {
		case E_LogLevel::DEBUG: return Instrumentation::ANSI_COLOR_BLUE;
		case E_LogLevel::INFO: return Instrumentation::ANSI_COLOR_GREEN;
		case E_LogLevel::WARNING: return Instrumentation::ANSI_COLOR_YELLOW;
		case E_LogLevel::ERROR: return Instrumentation::ANSI_COLOR_RED;
		default: return Instrumentation::ANSI_COLOR_RESET;
		}
	}

	/** @brief Constructs a log entry with the specified parameters.
		@param ts Timestamp of the log entry.
		@param lvl Log severity level.
		@param lib Library name.
		@param comp Component name.
		@param subComp Sub-component name.
		@param msg Log message.
		@param args Formatted arguments.
		@param isColored Whether to apply color formatting. */
	LogEntry::LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
		const std::string& subComp, const std::string& msg, const std::vector<std::string>& args, bool isColored)
		: timestamp(std::move(ts)), level(lvl), libraryName(lib), component(comp),
		subComponent(subComp), message(msg), formattedArgs(args), isColored(isColored) {
	}

	Instrumentation::BaseLogger& Instrumentation::getLogger(E_LogComponent component) {
		switch (component) {
		case E_LogComponent::BENCHMARK: return benchmarkLogger;
		case E_LogComponent::MATH: return mathLogger;
		}
	}

	/** @brief Converts the log entry to a plain string representation.
		@return String representation of the log entry without color codes. */
	std::string LogEntry::toString() const {
		std::string formatted = std::format("[{}] [{}] {}::{}::{}: {}",
			timestamp, levelToString(level, false), libraryName, component, subComponent, message);
		if (!formattedArgs.empty()) {
			formatted += " (details: ";
			for (size_t i = 0; i < formattedArgs.size(); ++i) {
				formatted += formattedArgs[i];
				if (i < formattedArgs.size() - 1) formatted += ", ";
			}
			formatted += ")";
		}
		return formatted;
	}

	/** @brief Converts the log entry to a colored string representation for console output.
		@return String representation with ANSI color codes applied to level, component, sub-component, and message.
		@note Only applies if isColored is true; otherwise, falls back to toString(). */
	std::string LogEntry::toColoredString() const {
		if (!isColored) return toString();
		std::string formatted = std::format("[{}] [{}] {}::{}::{}: {}",
			timestamp,
			levelToString(level, true),
			libraryName,
			getColor(level) + component + Instrumentation::ANSI_COLOR_RESET,
			getColor(level) + subComponent + Instrumentation::ANSI_COLOR_RESET,
			getColor(level) + message + Instrumentation::ANSI_COLOR_RESET);

		if (!formattedArgs.empty()) {
			formatted += (getColor(level) + " (details: ");
			for (size_t i = 0; i < formattedArgs.size(); ++i) {
				formatted += formattedArgs[i];
				if (i < formattedArgs.size() - 1) formatted += ", ";
			}
			formatted += (")" + Instrumentation::ANSI_COLOR_RESET);
		}
		return formatted;
	}

	/** @brief Converts a log level to its string representation, optionally with color.
		@param level Log severity level.
		@param isColored Whether to apply ANSI color codes.
		@return String representation of the log level, colored if specified. */
	std::string LogEntry::levelToString(E_LogLevel level, bool isColored) {
		if (isColored) {
			switch (level) {
			case E_LogLevel::DEBUG: return Instrumentation::ANSI_COLOR_BLUE + "DEBUG" + Instrumentation::ANSI_COLOR_RESET;
			case E_LogLevel::INFO: return Instrumentation::ANSI_COLOR_GREEN + "INFO" + Instrumentation::ANSI_COLOR_RESET;
			case E_LogLevel::WARNING: return Instrumentation::ANSI_COLOR_YELLOW + "WARNING" + Instrumentation::ANSI_COLOR_RESET;
			case E_LogLevel::ERROR: return Instrumentation::ANSI_COLOR_RED + "ERROR" + Instrumentation::ANSI_COLOR_RESET;
			default: return "UNKNOWN";
			}
		}
		else {
			switch (level) {
			case E_LogLevel::DEBUG: return "DEBUG";
			case E_LogLevel::INFO: return "INFO";
			case E_LogLevel::WARNING: return "WARNING";
			case E_LogLevel::ERROR: return "ERROR";
			default: return "UNKNOWN";
			}
		}
	}

	/** @brief Adds a log entry to the history, maintaining a maximum size.
		@param entry Log entry to add.
		@details Thread-safe; removes oldest entry if exceeding MAX_HISTORY_SIZE. */
	void LogHistory::addLog(const LogEntry& entry) {
		{
			std::lock_guard<std::mutex> lock(historyMutex);
			history.push_back(entry);
		}
		if (history.size() > MAX_HISTORY_SIZE) history.pop_front();
	}

	/** @brief Retrieves the log history as a vector of strings.
		@return Vector containing string representations of log entries.
		@details Thread-safe; uses toString() for each entry. */
	std::vector<std::string> LogHistory::getHistory() const {
		std::vector<std::string> result;
		{
			std::lock_guard<std::mutex> lock(historyMutex);
			for (const auto& entry : history) result.push_back(entry.toString());
		}
		return result;
	}

	/** @brief Retrieves the log history as a single formatted string.
		@return Formatted string of the log history with entries prefixed by indentation.
		@details Thread-safe; uses toString() for each entry. */
	std::string LogHistory::getHistoryAsString() const {
		std::string result = "Log History (most recent last):\n";
		{
			std::lock_guard<std::mutex> lock(historyMutex);
			for (const auto& entry : history) result += "  " + entry.toString() + "\n";
		}
		return result;
	}

	/** @brief Constructs a runtime error with a message and associated log history.
		@param message Error message.
		@param logHistory Log history to include with the error. */
	LoggedRuntimeError::LoggedRuntimeError(const std::string& message, const LogHistory& logHistory)
		: std::runtime_error(message), history(logHistory.getHistory()) {}

	/** @brief Retrieves the log history associated with the error.
		@return Constant reference to the vector of log entry strings. */
	const std::vector<std::string>& LoggedRuntimeError::getLogHistory() const { return history; }

	/** @brief Retrieves the full error message including log history.
		@return String containing the error message followed by formatted log history. */
	std::string LoggedRuntimeError::getFullMessage() const {
		std::string result = what();
		result += "\nLog History (most recent last):\n";
		for (const auto& log : history) result += "  " + log + "\n";
		return result;
	}

	/** @brief Constructs a BaseLogger instance for a specific library and file.
		@param libName Name of the library this logger serves.
		@param name File path for log output.
		@details Initializes default settings and opens file stream if FILE output is enabled. */
	Instrumentation::BaseLogger::BaseLogger(std::string libName, std::string name)
		: libraryName(std::move(libName)), enabled(true), minLevel(E_LogLevel::INFO), keepRunning(true),
		outputDestinations(E_LogOutput::CONSOLE | E_LogOutput::FILE), fileName(std::move(name)), coloredConsole(false) {
		for (int i = static_cast<int>(E_LogLevel::DEBUG); i <= static_cast<int>(E_LogLevel::ERROR); ++i) {
			logCounts[static_cast<E_LogLevel>(i)] = 0;
		}
		if (UINT_8(outputDestinations & E_LogOutput::FILE)) {
			fileStream.open(fileName, std::ios::app);
			if (!fileStream.is_open()) {
				std::cerr << "[WARNING] " << libraryName << ": Failed to open " << fileName << ", file logging off!\n";
				outputDestinations = E_LogOutput::CONSOLE;
			}
		}
	}

	/** @brief Destructor for BaseLogger, ensuring proper resource cleanup.
		@details Closes the file stream if open. */
	Instrumentation::BaseLogger::~BaseLogger() {
		if (fileStream.is_open()) fileStream.close();
	}

	/** @brief Generates a timestamp for log entries.
		@return Current timestamp as a string in the format "YYYY-MM-DD HH:MM:SS".
		@exception std::exception Returns an error string if timestamp generation fails. */
	std::string Instrumentation::BaseLogger::getTimestamp() {
		try {
			auto now = std::chrono::system_clock::now();
			auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
			return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
		}
		catch (const std::exception& e) {
			return "[ERROR: Timestamp fail: " + std::string(e.what()) + "]";
		}
	}

	/** @brief Checks if a log level is valid.
		@param level Log level to validate.
		@return True if the level is within the defined range, false otherwise. */
	bool Instrumentation::BaseLogger::isValidLevel(E_LogLevel level) {
		const int l = static_cast<int>(level);
		return l >= static_cast<int>(E_LogLevel::DEBUG) && l <= static_cast<int>(E_LogLevel::ERROR);
	}

	/** @brief Enables or disables colored console output.
		@param value True to enable colored output, false to disable. */
	void Instrumentation::BaseLogger::enableColoredConsole(bool value) {
		coloredConsole = value;
	}

	/** @brief Processes a log entry internally.
		@param level Log severity level.
		@param component Component name.
		@param subComponent Sub-component name.
		@param message Log message.
		@param args Arguments to format into the log.
		@details Validates level, buffers the entry, and throws an error for ERROR level logs. */
	void Instrumentation::BaseLogger::logInternal(E_LogLevel level, const std::string& component,
		const std::string& subComponent, const std::string& message,
		const std::vector<std::any>& args) {
		if (!isValidLevel(level)) {
			if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
				std::cerr << "[WARNING] " << libraryName << ": Invalid log level " << static_cast<int>(level) << ", ignored!\n";
			}
			return;
		}

		if (!enabled || static_cast<int>(level) < static_cast<int>(minLevel)) return;

		logCounts[level]++;
		std::vector<std::string> formattedArgs = unpackAndFormatArgs(args);
		LogEntry entry(getTimestamp(), level, libraryName, component, subComponent, message, formattedArgs, coloredConsole);
		logHistory.addLog(entry);
		{
			std::lock_guard<std::mutex> lock(bufferMutex);
			logBuffer.push_back(entry);
		}

		{
			std::lock_guard<std::mutex> lock(flushMutex);
			flushCV.notify_one();
		}

		if (level == E_LogLevel::ERROR) {
			throw LoggedRuntimeError(entry.toString(), logHistory);
		}
	}

	/** @brief Formats a vector of arguments into strings.
		@param args Arguments to format.
		@return Vector of formatted argument strings.
		@details Handles common types (int, string, char*, void*) with fallback for unknown types. */
	std::vector<std::string> Instrumentation::BaseLogger::unpackAndFormatArgs(const std::vector<std::any>& args) {
		std::vector<std::string> formattedArgs;
		for (const auto& arg : args) {
			try {
				if (arg.type() == typeid(int)) {
					formattedArgs.emplace_back(std::format("{}", std::any_cast<int>(arg)));
				}
				else if (arg.type() == typeid(std::string)) {
					formattedArgs.emplace_back(std::any_cast<std::string>(arg));
				}
				else if (arg.type() == typeid(const char*)) {
					formattedArgs.emplace_back(std::any_cast<const char*>(arg));
				}
				else if (arg.type() == typeid(void*)) {
					auto ptr = std::any_cast<void*>(arg);
					formattedArgs.emplace_back(ptr ? std::format("0x{:x}", reinterpret_cast<uintptr_t>(ptr)) : "null");
				}
				else {
					formattedArgs.emplace_back("[unknown type]");
				}
			}
			catch (const std::bad_any_cast&) {
				formattedArgs.emplace_back("[bad cast]");
			}
		}
		return formattedArgs;
	}

	/** @brief Enables or disables logging.
		@param enable True to enable logging, false to disable. */
	void Instrumentation::BaseLogger::setEnabled(bool enable) { enabled = enable; }

	/** @brief Checks if logging is enabled.
		@return True if logging is enabled, false otherwise. */
	bool Instrumentation::BaseLogger::isEnabled() const { return enabled; }

	/** @brief Sets the minimum log level to process.
		@param level Minimum log level to set.
		@details Validates the level; logs a warning if invalid. */
	void Instrumentation::BaseLogger::setMinLevel(E_LogLevel level) {
		if (isValidLevel(level)) minLevel = level;
		else if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
			std::cerr << "[WARNING] " << libraryName << ": Invalid log level " << static_cast<int>(level) << ", keeping previous!\n";
		}
	}

	/** @brief Retrieves the current minimum log level.
		@return Current minimum log level. */
	E_LogLevel Instrumentation::BaseLogger::getMinLevel() const { return minLevel; }

	/** @brief Sets the output destinations for logs.
		@param destinations New output destinations to set.
		@details Manages file stream state based on FILE flag changes. */
	void Instrumentation::BaseLogger::setOutputDestinations(E_LogOutput destinations) {
		{
			std::lock_guard<std::mutex> lock(bufferMutex);
			if (UINT_8(outputDestinations & E_LogOutput::FILE) && !UINT_8(destinations & E_LogOutput::FILE) && fileStream.is_open()) {
				fileStream.close();
			}
			if (!UINT_8(outputDestinations & E_LogOutput::FILE) && UINT_8(destinations & E_LogOutput::FILE)) {
				fileStream.open(fileName, std::ios::app);
				if (!fileStream.is_open()) {
					std::cerr << "[WARNING] " << libraryName << ": Failed to open " << fileName << ", no file logs!\n";
					outputDestinations = E_LogOutput::CONSOLE;
					return;
				}
			}
		}
		outputDestinations = destinations;
	}

	/** @brief Retrieves the current output destinations.
		@return Current output destinations. */
	E_LogOutput Instrumentation::BaseLogger::getOutputDestinations() const { return outputDestinations; }

	/** @brief Retrieves the count of logs for a specific level.
		@param level Log level to query.
		@return Number of logs recorded at the specified level, or 0 if invalid. */
	int Instrumentation::BaseLogger::getLogCount(E_LogLevel level) const {
		if (!isValidLevel(level)) return 0;
		auto it = logCounts.find(level);
		return it != logCounts.end() ? it->second : 0;
	}

	/** @brief Retrieves the total count of logs across all levels.
		@return Total number of logs processed. */
	int Instrumentation::BaseLogger::getTotalLogCount() const {
		int total = 0;
		for (const auto& pair : logCounts) total += pair.second;
		return total;
	}

	/** @brief Flushes the log buffer synchronously.
		@details Outputs buffered logs to console and/or file, then clears the buffer.
		@note Blocks until the operation completes. */
	void Instrumentation::BaseLogger::synchronousFlush() {
		std::lock_guard<std::mutex> lock(bufferMutex);
		if (logBuffer.empty()) return;

		if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
			for (const auto& entry : logBuffer) {
				std::cerr << "[TEMP] " << entry.toColoredString() << "\n";
			}
			std::cerr.flush();
		}
		if (UINT_8(outputDestinations & E_LogOutput::FILE) && fileStream.is_open()) {
			for (const auto& entry : logBuffer) {
				fileStream << "[PERM] " << entry.toString() << "\n";
			}
			fileStream.flush();
		}
	}

	/** @brief Worker function for asynchronous log flushing.
		@details Runs in a separate thread, flushing the log buffer when signaled or on shutdown. */
	void Instrumentation::BaseLogger::flushWorker() {
		while (true) {
			std::unique_lock<std::mutex> lock(flushMutex);
			flushCV.wait(lock, [this] { return !logBuffer.empty() || !keepRunning; });
			if (!keepRunning && logBuffer.empty()) return;
			{
				std::lock_guard<std::mutex> bufferLock(bufferMutex);
				if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
					for (const auto& entry : logBuffer) {
						std::cerr << "[TEMP] " << entry.toColoredString() << "\n";
					}
					std::cerr.flush();
				}
				if (UINT_8(outputDestinations & E_LogOutput::FILE) && fileStream.is_open()) {
					for (const auto& entry : logBuffer) {
						fileStream << "[PERM] " << entry.toString() << "\n";
					}
					fileStream.flush();
				}
				logBuffer.clear();
			}
		}
	}

	/** @brief Sets the file location for log output.
		@param location New file path for logs.
		@details Updates the file stream, reopening if FILE output is enabled.
		@note Thread-safe; closes existing stream before switching. */
	void Instrumentation::BaseLogger::setOutputLocation(std::string& location) {
		std::lock_guard<std::mutex> lock(bufferMutex);
		if (fileStream.is_open()) fileStream.close();
		fileName = location;
		if (UINT_8(outputDestinations & E_LogOutput::FILE)) {
			fileStream.open(fileName, std::ios::app);
			if (!fileStream.is_open()) {
				std::cerr << "[WARNING] " << libraryName << ": Failed to open " << fileName << ", file logging off!\n";
				outputDestinations = E_LogOutput::CONSOLE;
			}
		}
	}
}