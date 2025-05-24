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

namespace spectra::instrumentation {

	enum class SPEC_INSTRUMENTATION E_LogOutput : uint8_t {
		NONE = 0,           /**< No output destination. */
		CONSOLE = 1 << 0,   /**< Output to console. */
		FILE = 1 << 1       /**< Output to file. */
	};

	inline E_LogOutput operator|(E_LogOutput lhs, E_LogOutput rhs) {
		return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	inline E_LogOutput operator&(E_LogOutput lhs, E_LogOutput rhs) {
		return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	enum class SPEC_INSTRUMENTATION E_LogLevel : uint8_t {
		DEBUG_ = 0,   
		INFO_ = 1,    
		WARNING_ = 2, 
		ERROR_ = 3    
	};

	enum class SPEC_INSTRUMENTATION E_LogComponent : uint8_t {
		MATH,      /**< Mathematical operations component. */
		BENCHMARK,  /**< Performance benchmarking component. */
		CORE /**< Core framework component. */
	};

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

		
		LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
			const std::string& subComp, const std::string& msg, const std::vector<std::string>& args, bool isColored);

		[[nodiscard]] std::string toString() const;

		[[nodiscard]] std::string toColoredString() const;

	private:
		static std::string levelToString(E_LogLevel level, bool isColored);
	};

	
	class SPEC_INSTRUMENTATION LogHistory {
	private:
		static constexpr size_t MAX_HISTORY_SIZE = 100; /**< Maximum number of log entries to retain. */
		std::deque<LogEntry> history;                   /**< Deque storing log entries. */
		mutable std::mutex historyMutex;                /**< Mutex for thread-safe access to history. */

	public:
		void addLog(const LogEntry& entry);

		std::vector<std::string> getHistory() const;

		std::string getHistoryAsString() const;
	};

	class SPEC_INSTRUMENTATION LoggedRuntimeError : public std::runtime_error {
	private:
		std::vector<std::string> history; /**< Log history associated with the error. */

	public:
		LoggedRuntimeError(const std::string& message, const LogHistory& logHistory);

		[[nodiscard]] const std::vector<std::string>& getLogHistory() const;

		[[nodiscard]] std::string getFullMessage() const;
	};

	class SPEC_INSTRUMENTATION ILogger {
	public:
		ILogger() = default;
		virtual ~ILogger() = default;

	protected:
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
		class SPEC_INSTRUMENTATION BaseLogger final : public ILogger {
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
		static BaseLogger coreLogger; 	/**< Logger instance for core Spectra operations. */

	public:
		static BaseLogger& getLogger(E_LogComponent component);

		template<typename... Args>
		static void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
			const std::string& message, E_LogComponent buffer, Args&&... args) {
			getLogger(buffer).log(level, component, subComponent, message, std::forward<Args>(args)...);
		}

		static void setEnabled(bool enable, E_LogComponent component) {
			getLogger(component).setEnabled(enable);
		}

		static bool isEnabled(E_LogComponent component) {
			return getLogger(component).isEnabled();
		}

		static void setMinLevel(E_LogLevel level, E_LogComponent component) {
			getLogger(component).setMinLevel(level);
		}

		static E_LogLevel getMinLevel(E_LogComponent component) {
			return getLogger(component).getMinLevel();
		}

		static void setOutputDestinations(E_LogOutput destinations, E_LogComponent component) {
			getLogger(component).setOutputDestinations(destinations);
		}

		static E_LogOutput getOutputDestinations(E_LogComponent component) {
			return getLogger(component).getOutputDestinations();
		}

		static int getLogCount(E_LogLevel level, E_LogComponent component) {
			return getLogger(component).getLogCount(level);
		}

		static int getTotalLogCount(E_LogComponent component) {
			return getLogger(component).getTotalLogCount();
		}
		
		static void synchronousFlush(E_LogComponent component) {
			getLogger(component).synchronousFlush();
		}

		static void setOutputLocation(std::string& location, E_LogComponent component) {
			getLogger(component).setOutputLocation(location);
		}

		static void enableColoredConsole(bool value, E_LogComponent component) {
			getLogger(component).enableColoredConsole(value);
		}
	};

	// Static member definitions
	inline Instrumentation::BaseLogger Instrumentation::mathLogger("spectra::math", "math_log.txt");
	inline Instrumentation::BaseLogger Instrumentation::benchmarkLogger("spectra::benchmark", "benchmark_log.txt");
	inline Instrumentation::BaseLogger Instrumentation::coreLogger("spectra::core", "core_log.txt");

	inline const std::string Instrumentation::ANSI_COLOR_RED = "\033[31m";
	inline const std::string Instrumentation::ANSI_COLOR_YELLOW = "\033[33m";
	inline const std::string Instrumentation::ANSI_COLOR_BLUE = "\033[34m";
	inline const std::string Instrumentation::ANSI_COLOR_RESET = "\033[0m";
	inline const std::string Instrumentation::ANSI_COLOR_GREEN = "\033[32m";
}