#pragma once

#ifndef SPEC_INSTRUMENTATION
#define SPEC_INSTRUMENTATION __declspec(dllexport)
#include <iostream>
#endif

#ifndef UINT_8
#define UINT_8(value) static_cast<uint8_t>(value)
#endif

void SPEC_INSTRUMENTATION SpectraInstrumentationInit();

#include <string>
#include <fstream>
#include <unordered_map>
#include <any>
#include <vector>
#include <mutex>
#include <deque>  // For circular buffer

namespace spectra {
    namespace instrumentation {
        // Bitmask for output destinations
        enum class SPEC_INSTRUMENTATION E_LogOutput : uint8_t {
            NONE = 0,
            CONSOLE = 1 << 0,  // Temporary logs
            FILE = 1 << 1,     // Permanent logs
        };

        // Allow combining E_LogOutput values
        inline E_LogOutput operator|(E_LogOutput lhs, E_LogOutput rhs) {
            return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
        }

        inline E_LogOutput operator&(E_LogOutput lhs, E_LogOutput rhs) {
            return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
        }

        // Reordered for correct severity: DEBUG < INFO < WARNING < ERROR
        enum class SPEC_INSTRUMENTATION E_LogLevel : uint8_t {
            DEBUG = 0,
            INFO = 1,
            WARNING = 2,
            ERROR = 3
        };

        // Struct to encapsulate a log entry
        class SPEC_INSTRUMENTATION LogEntry {
        public:
            std::string timestamp;
            E_LogLevel level;
            std::string libraryName;
            std::string component;
            std::string subComponent;
            std::string message;
            std::vector<std::string> formattedArgs;

            LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
                const std::string& subComp, const std::string& msg, const std::vector<std::string>& args);

            // Convert the log entry to a formatted string
            std::string toString() const;

        private:
            static std::string levelToString(E_LogLevel level);
        };

        // Class to store log history for backtracking
        class SPEC_INSTRUMENTATION LogHistory {
        private:
            static constexpr size_t MAX_HISTORY_SIZE = 100;  // Max number of log messages to store
            std::deque<LogEntry> history;  // Circular buffer of log entries
            mutable std::mutex historyMutex;  // Thread safety for history, mutable to allow locking in const methods

        public:
            void addLog(const LogEntry& entry);

            std::vector<std::string> getHistory() const;

            std::string getHistoryAsString() const;
        };

        // Custom exception class to include log history
        class SPEC_INSTRUMENTATION LoggedRuntimeError : public std::runtime_error {
        private:
            std::vector<std::string> history;  // Snapshot of the log history

        public:
            LoggedRuntimeError(const std::string& message, const LogHistory& logHistory);

            const std::vector<std::string>& getLogHistory() const;

            std::string getFullMessage() const;
        };

        // Abstract base class for loggers
        class SPEC_INSTRUMENTATION I_Logger {
        public:
            I_Logger() = default;
            virtual ~I_Logger() = default;

        protected:
            virtual void logInternal(E_LogLevel level, const std::string& component, const std::string& subComponent,
                const std::string& message, const std::vector<std::any>& args) = 0;

        public:
            virtual void setEnabled(bool enable) = 0;
            virtual bool isEnabled() const = 0;
            virtual void setMinLevel(E_LogLevel level) = 0;
            virtual E_LogLevel getMinLevel() const = 0;
            virtual void setOutputDestinations(E_LogOutput destinations) = 0;
            virtual E_LogOutput getOutputDestinations() const = 0;
            virtual int getLogCount(E_LogLevel level) const = 0;
            virtual int getTotalLogCount() const = 0;
            virtual void flush() = 0;
        };

        // Global instrumentation manager with nested loggers
        class SPEC_INSTRUMENTATION Instrumentation {
        private:
            static std::vector<LogEntry> logBuffer;  // Central buffer for all log entries
            static std::mutex bufferMutex;  // Thread safety for the buffer
            static std::ofstream mathFileStream;  // File stream for MathLogger
            static std::string mathFileName;  // File name for MathLogger

            // Base class for nested loggers
            class SPEC_INSTRUMENTATION BaseLogger : public I_Logger {
            protected:
                std::string libraryName;
                bool enabled;
                E_LogLevel minLevel;
                E_LogOutput outputDestinations;
                std::unordered_map<E_LogLevel, int> logCounts;
                LogHistory logHistory;

                // Helpers (declared here, implemented in .cpp)
                static std::string getTimestamp();
                static std::vector<std::string> unpackAndFormatArgs(const std::vector<std::any>& args);

                // Validate log level
                static bool isValidLevel(E_LogLevel level);

            public:
                BaseLogger(const std::string& libName);
                ~BaseLogger() override;

            private:
                void logInternal(E_LogLevel level, const std::string& component, const std::string& subComponent,
                    const std::string& message, const std::vector<std::any>& args) override;

            public:
                template<typename... Args>
                void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
                    const std::string& message, Args&&... args) {
                    // Pack the variadic arguments into a vector of std::any
                    std::vector<std::any> packedArgs;
                    (packedArgs.emplace_back(std::forward<Args>(args)), ...);

                    // Call the virtual log method
                    logInternal(level, component, subComponent, message, packedArgs);
                }

                void setEnabled(bool enable) override;
                bool isEnabled() const override;
                void setMinLevel(E_LogLevel level) override;
                E_LogLevel getMinLevel() const override;
                void setOutputDestinations(E_LogOutput destinations) override;
                E_LogOutput getOutputDestinations() const override;
                int getLogCount(E_LogLevel level) const override;
                int getTotalLogCount() const override;
                void flush() override;
            };

        public:
            // Logger for spectra::core::math library
            class SPEC_INSTRUMENTATION MathLogger final : public BaseLogger {
            public:
                MathLogger();
                static MathLogger& getInstance();
            };

            template<typename ...Args>
			static void logMath(E_LogLevel level, const std::string& component, const std::string& subComponent,
                const std::string& message, Args&&... args) {
                MathLogger::getInstance().log(level, component, subComponent, message, std::forward<Args>(args)...);
            }

            static void setMathEnabled(bool enable);
            static bool isMathEnabled();
            static void setMathMinLevel(E_LogLevel level);
            static E_LogLevel getMathMinLevel();
            static void setMathOutputDestinations(E_LogOutput destinations);
            static E_LogOutput getMathOutputDestinations();
            static int getMathLogCount(E_LogLevel level);
            static int getMathTotalLogCount();
            static void flushMath();

            // Add a log entry to the central buffer
            static void addToBuffer(const LogEntry& entry);

            // Flush the central buffer to the output destinations
            static void flush();
        };
    }
}

// Define static members
inline std::vector<spectra::instrumentation::LogEntry> spectra::instrumentation::Instrumentation::logBuffer;
inline std::mutex spectra::instrumentation::Instrumentation::bufferMutex;
inline std::ofstream spectra::instrumentation::Instrumentation::mathFileStream;
inline std::string spectra::instrumentation::Instrumentation::mathFileName = "math_log.txt";