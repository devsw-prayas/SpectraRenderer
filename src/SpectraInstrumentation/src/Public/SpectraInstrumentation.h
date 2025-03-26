#pragma once

#ifndef SPEC_INSTRUMENTATION
#define SPEC_INSTRUMENTATION __declspec(dllexport)
/** @brief Defines export macro for DLL visibility on Windows. */
#endif

#ifndef UINT_8
#define UINT_8(value) static_cast<uint8_t>(value)
/** @brief Casts a value to an 8-bit unsigned integer. */
#endif

#ifndef LOCATION
#define LOCATION "Line Number: " + std::to_string(__LINE__), "Function call: " + std::string(__FUNCSIG__), "File: " + std::string(__FILE__)
/** @brief Attempts to define a macro for embedding file, function, and line information as formatted strings.
	@note This macro is invalid due to runtime operations in a preprocessor context; see implementation notes for correction. */
#endif

#include <string>
#include <fstream>
#include <unordered_map>
#include <any>
#include <vector>
#include <mutex>
#include <deque>

	/** @namespace spectra::instrumentation
		@brief Namespace for logging and instrumentation utilities within the Spectra framework. */
namespace spectra::instrumentation {
	/** @enum E_LogOutput
		@brief Specifies possible output destinations for log messages. */
	enum class SPEC_INSTRUMENTATION E_LogOutput : uint8_t {
		NONE = 0,           /**< No output destination. */
		CONSOLE = 1 << 0,   /**< Output to console. */
		FILE = 1 << 1       /**< Output to file. */
	};

	/** @brief Bitwise OR operator for combining E_LogOutput values.
		@param lhs Left-hand side operand.
		@param rhs Right-hand side operand.
		@return Combined E_LogOutput value. */
	inline E_LogOutput operator|(E_LogOutput lhs, E_LogOutput rhs) {
		return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	/** @brief Bitwise AND operator for checking E_LogOutput flags.
		@param lhs Left-hand side operand.
		@param rhs Right-hand side operand.
		@return Resulting E_LogOutput value after applying AND operation. */
	inline E_LogOutput operator&(E_LogOutput lhs, E_LogOutput rhs) {
		return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	/** @enum E_LogLevel
		@brief Defines severity levels for log messages. */
	enum class SPEC_INSTRUMENTATION E_LogLevel : uint8_t {
		DEBUG = 0,   /**< Debug-level message for detailed diagnostics. */
		INFO = 1,    /**< Informational message for general status updates. */
		WARNING = 2, /**< Warning message indicating potential issues. */
		ERROR = 3    /**< Error message indicating a failure or critical issue. */
	};

	/** @enum E_LogComponent
		@brief Identifies logging components within the Spectra framework. */
	enum class SPEC_INSTRUMENTATION E_LogComponent : uint8_t {
		MATH,      /**< Mathematical operations component. */
		BENCHMARK  /**< Performance benchmarking component. */
	};

	/** @class LogEntry
		@brief Represents a single log entry with associated metadata. */
	class SPEC_INSTRUMENTATION LogEntry {
	public:
		std::string timestamp;            /**< Timestamp of the log entry. */
		E_LogLevel level;                 /**< Severity level of the log entry. */
		std::string libraryName;          /**< Name of the library generating the log. */
		std::string component;            /**< Component within the library. */
		std::string subComponent;         /**< Sub-component within the component. */
		std::string message;              /**< Log message content. */
		std::vector<std::string> formattedArgs; /**< Formatted arguments associated with the message. */
		bool isColored;                   /**< Flag indicating if the log should be colored in console output. */

		/** @brief Constructs a log entry with the specified parameters.
			@param ts Timestamp of the log entry.
			@param lvl Log severity level.
			@param lib Library name.
			@param comp Component name.
			@param subComp Sub-component name.
			@param msg Log message.
			@param args Formatted arguments.
			@param isColored Whether to apply color formatting. */
		LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
			const std::string& subComp, const std::string& msg, const std::vector<std::string>& args, bool isColored);

		/** @brief Converts the log entry to a plain string representation.
			@return String representation of the log entry. */
		[[nodiscard]] std::string toString() const;

		/** @brief Converts the log entry to a colored string representation for console output.
			@return Colored string representation of the log entry. */
		[[nodiscard]] std::string toColoredString() const;

	private:
		/** @brief Converts a log level to its string representation, optionally with color.
			@param level Log severity level.
			@param isColored Whether to apply color codes.
			@return String representation of the log level. */
		static std::string levelToString(E_LogLevel level, bool isColored);
	};

	/** @class LogHistory
		@brief Manages a history of log entries with a fixed maximum size. */
	class SPEC_INSTRUMENTATION LogHistory {
	private:
		static constexpr size_t MAX_HISTORY_SIZE = 100; /**< Maximum number of log entries to retain. */
		std::deque<LogEntry> history;                   /**< Deque storing log entries. */
		mutable std::mutex historyMutex;                /**< Mutex for thread-safe access to history. */

	public:
		/** @brief Adds a log entry to the history, removing the oldest if exceeding capacity.
			@param entry Log entry to add. */
		void addLog(const LogEntry& entry);

		/** @brief Retrieves the log history as a vector of strings.
			@return Vector of log entry strings. */
		std::vector<std::string> getHistory() const;

		/** @brief Retrieves the log history as a single formatted string.
			@return Formatted string of the log history. */
		std::string getHistoryAsString() const;
	};

	/** @class LoggedRuntimeError
		@brief Exception class that includes log history with the error message. */
	class SPEC_INSTRUMENTATION LoggedRuntimeError : public std::runtime_error {
	private:
		std::vector<std::string> history; /**< Log history associated with the error. */

	public:
		/** @brief Constructs an error with a message and associated log history.
			@param message Error message.
			@param logHistory Log history to include. */
		LoggedRuntimeError(const std::string& message, const LogHistory& logHistory);

		/** @brief Retrieves the log history associated with the error.
			@return Vector of log entry strings. */
		[[nodiscard]] const std::vector<std::string>& getLogHistory() const;

		/** @brief Retrieves the full error message including log history.
			@return Full error message string. */
		[[nodiscard]] std::string getFullMessage() const;
	};

	/** @class I_Logger
		@brief Abstract interface for logging functionality. */
	class SPEC_INSTRUMENTATION I_Logger {
	public:
		I_Logger() = default;
		virtual ~I_Logger() = default;

	protected:
		/** @brief Internal method to process a log entry.
			@param level Log severity level.
			@param component Component name.
			@param subComponent Sub-component name.
			@param message Log message.
			@param args Arguments to format into the log. */
		virtual void logInternal(E_LogLevel level, const std::string& component, const std::string& subComponent,
			const std::string& message, const std::vector<std::any>& args) = 0;

	public:
		virtual void setEnabled(bool enable) = 0;
		[[nodiscard]] virtual bool isEnabled() const = 0;
		virtual void setMinLevel(E_LogLevel level) = 0;
		[[nodiscard]] virtual E_LogLevel getMinLevel() const = 0;
		virtual void setOutputDestinations(E_LogOutput destinations) = 0;
		[[nodiscard]] virtual E_LogOutput getOutputDestinations() const = 0;
		[[nodiscard]] virtual int getLogCount(E_LogLevel level) const = 0;
		[[nodiscard]] virtual int getTotalLogCount() const = 0;
		virtual void synchronousFlush() = 0;
		virtual void setOutputLocation(std::string& location) = 0;
	};

	/** @class Instrumentation
		@brief Main class for managing logging functionality within the Spectra framework. */
	class SPEC_INSTRUMENTATION Instrumentation final {
	public:
		/** @brief Initializes the logging system, starting flush threads for all loggers. */
		static void init();

		/** @brief Shuts down the logging system, stopping flush threads and cleaning up resources. */
		static void shutdown();

		static const std::string ANSI_COLOR_RED;    /**< ANSI escape code for red text. */
		static const std::string ANSI_COLOR_YELLOW; /**< ANSI escape code for yellow text. */
		static const std::string ANSI_COLOR_BLUE;   /**< ANSI escape code for blue text. */
		static const std::string ANSI_COLOR_RESET;  /**< ANSI escape code to reset text color. */
		static const std::string ANSI_COLOR_GREEN;  /**< ANSI escape code for green text. */

		/** @class BaseLogger
			@brief Concrete implementation of the I_Logger interface for logging operations. */
		class SPEC_INSTRUMENTATION BaseLogger final : public I_Logger {
			std::string libraryName;                     /**< Name of the library this logger serves. */
			bool enabled;                                /**< Flag indicating if logging is enabled. */
			E_LogLevel minLevel;                         /**< Minimum log level to process. */
			E_LogOutput outputDestinations;              /**< Destinations for log output. */
			std::string fileName;                        /**< File path for log output. */
			std::ofstream fileStream;                    /**< File stream for persistent logging. */
			std::vector<LogEntry> logBuffer;             /**< Buffer for pending log entries. */
			std::mutex bufferMutex;                      /**< Mutex for thread-safe buffer access. */
			std::unordered_map<E_LogLevel, int> logCounts; /**< Counts of logs per level. */
			LogHistory logHistory;                       /**< History of log entries. */
			bool coloredConsole;                         /**< Flag for enabling colored console output. */

			// Async inclusions for non-blocking logging
			std::thread flushThread;                     /**< Thread for asynchronous log flushing. */
			bool keepRunning;                            /**< Flag to control flush thread lifecycle. */
			std::condition_variable flushCV;             /**< Condition variable for flush synchronization. */
			std::mutex flushMutex;                       /**< Mutex for flush thread synchronization. */

			/** @brief Generates a timestamp for log entries.
				@return Current timestamp as a string. */
			static std::string getTimestamp();

			/** @brief Formats a vector of arguments into strings.
				@param args Arguments to format.
				@return Vector of formatted argument strings. */
			static std::vector<std::string> unpackAndFormatArgs(const std::vector<std::any>& args);

			/** @brief Checks if a log level is valid.
				@param level Log level to validate.
				@return True if valid, false otherwise. */
			static bool isValidLevel(E_LogLevel level);

		public:

			BaseLogger() = default;

			/** @brief Constructs a logger instance for a specific library and file.
				@param libName Name of the library.
				@param name File name for log output. */
			BaseLogger(std::string libName, std::string name);

			/** @brief Destructor, ensures proper cleanup of resources. */
			~BaseLogger() override;

		private:
			/** @brief Processes a log entry internally.
				@param level Log severity level.
				@param component Component name.
				@param subComponent Sub-component name.
				@param message Log message.
				@param args Arguments to format into the log. */
			void logInternal(E_LogLevel level, const std::string& component, const std::string& subComponent,
				const std::string& message, const std::vector<std::any>& args) override;

			/** @brief Worker function for asynchronous log flushing. */
			void flushWorker();

		public:
			/** @brief Logs a message with variadic arguments.
				@tparam Args Variadic argument types.
				@param level Log severity level.
				@param component Component name.
				@param subComponent Sub-component name.
				@param message Log message.
				@param args Arguments to include in the log. */
			template<typename... Args>
			void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
				const std::string& message, Args&&... args) {
				std::vector<std::any> packedArgs;
				(packedArgs.emplace_back(std::forward<Args>(args)), ...);
				logInternal(level, component, subComponent, message, packedArgs);
			}

			/** @brief Enables or disables logging.
				@param enable True to enable, false to disable. */
			void setEnabled(bool enable) override;

			/** @brief Checks if logging is enabled.
				@return True if enabled, false otherwise. */
			[[nodiscard]] bool isEnabled() const override;

			/** @brief Sets the minimum log level to process.
				@param level Minimum log level. */
			void setMinLevel(E_LogLevel level) override;

			/** @brief Retrieves the minimum log level.
				@return Current minimum log level. */
			[[nodiscard]] E_LogLevel getMinLevel() const override;

			/** @brief Sets the output destinations for logs.
				@param destinations Output destinations. */
			void setOutputDestinations(E_LogOutput destinations) override;

			/** @brief Retrieves the current output destinations.
				@return Current output destinations. */
			[[nodiscard]] E_LogOutput getOutputDestinations() const override;

			/** @brief Retrieves the count of logs for a specific level.
				@param level Log level to query.
				@return Number of logs at the specified level. */
			[[nodiscard]] int getLogCount(E_LogLevel level) const override;

			/** @brief Retrieves the total count of logs.
				@return Total number of logs processed. */
			[[nodiscard]] int getTotalLogCount() const override;

			/** @brief Flushes the log buffer synchronously.
				@note Blocks until the buffer is cleared. */
			void synchronousFlush() override;

			/** @brief Sets the file location for log output.
				@param location New file path for logs. */
			void setOutputLocation(std::string& location) override;

			/** @brief Enables or disables colored console output.
				@param value True to enable, false to disable. */
			void enableColoredConsole(bool value);

			friend class Instrumentation;
		};

	private:
		static BaseLogger mathLogger;      /**< Logger instance for mathematical operations. */
		static BaseLogger benchmarkLogger; /**< Logger instance for benchmarking. */

	public:
		/** @brief Retrieves the logger instance for a given component.
			@param component Component to retrieve logger for.
			@return Reference to the corresponding BaseLogger instance.
			@throws std::invalid_argument If the component is unknown. */
		static BaseLogger& getLogger(E_LogComponent component);

	public:
		/** @brief Logs a message with variadic arguments to a specified component.
			@tparam Args Variadic argument types.
			@param level Log severity level.
			@param component Component name.
			@param subComponent Sub-component name.
			@param message Log message.
			@param buffer Component to log to (MATH or BENCHMARK).
			@param args Arguments to include in the log. */
		template<typename... Args>
		static void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
			const std::string& message, E_LogComponent buffer, Args&&... args) {
			getLogger(buffer).log(level, component, subComponent, message, std::forward<Args>(args)...);
		}

		/** @brief Enables or disables logging for a component.
			@param enable True to enable, false to disable.
			@param component Component to configure. */
		static void setEnabled(bool enable, E_LogComponent component) {
			getLogger(component).setEnabled(enable);
		}

		/** @brief Checks if logging is enabled for a component.
			@param component Component to query.
			@return True if enabled, false otherwise. */
		static bool isEnabled(E_LogComponent component) {
			return getLogger(component).isEnabled();
		}

		/** @brief Sets the minimum log level for a component.
			@param level Minimum log level.
			@param component Component to configure. */
		static void setMinLevel(E_LogLevel level, E_LogComponent component) {
			getLogger(component).setMinLevel(level);
		}

		/** @brief Retrieves the minimum log level for a component.
			@param component Component to query.
			@return Current minimum log level. */
		static E_LogLevel getMinLevel(E_LogComponent component) {
			return getLogger(component).getMinLevel();
		}

		/** @brief Sets the output destinations for a component.
			@param destinations Output destinations.
			@param component Component to configure. */
		static void setOutputDestinations(E_LogOutput destinations, E_LogComponent component) {
			getLogger(component).setOutputDestinations(destinations);
		}

		/** @brief Retrieves the output destinations for a component.
			@param component Component to query.
			@return Current output destinations. */
		static E_LogOutput getOutputDestinations(E_LogComponent component) {
			return getLogger(component).getOutputDestinations();
		}

		/** @brief Retrieves the log count for a level and component.
			@param level Log level to query.
			@param component Component to query.
			@return Number of logs at the specified level. */
		static int getLogCount(E_LogLevel level, E_LogComponent component) {
			return getLogger(component).getLogCount(level);
		}

		/** @brief Retrieves the total log count for a component.
			@param component Component to query.
			@return Total number of logs processed. */
		static int getTotalLogCount(E_LogComponent component) {
			return getLogger(component).getTotalLogCount();
		}

		/** @brief Flushes the log buffer synchronously for a component.
			@param component Component to flush. */
		static void synchronousFlush(E_LogComponent component) {
			getLogger(component).synchronousFlush();
		}

		/** @brief Sets the file location for log output for a component.
			@param location New file path for logs.
			@param component Component to configure. */
		static void setOutputLocation(std::string& location, E_LogComponent component) {
			getLogger(component).setOutputLocation(location);
		}

		/** @brief Enables or disables colored console output for a component.
			@param value True to enable, false to disable.
			@param component Component to configure. */
		static void enableColoredConsole(bool value, E_LogComponent component) {
			getLogger(component).enableColoredConsole(value);
		}
	};

	// Static member definitions
	inline Instrumentation::BaseLogger Instrumentation::mathLogger("spectra::math", "math_log.txt");
	inline Instrumentation::BaseLogger Instrumentation::benchmarkLogger("spectra::benchmark", "benchmark_log.txt");

	inline const std::string Instrumentation::ANSI_COLOR_RED = "\033[31m";
	inline const std::string Instrumentation::ANSI_COLOR_YELLOW = "\033[33m";
	inline const std::string Instrumentation::ANSI_COLOR_BLUE = "\033[34m";
	inline const std::string Instrumentation::ANSI_COLOR_RESET = "\033[0m";
	inline const std::string Instrumentation::ANSI_COLOR_GREEN = "\033[32m";
}