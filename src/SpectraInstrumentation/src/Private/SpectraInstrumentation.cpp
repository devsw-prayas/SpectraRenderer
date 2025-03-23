#include "SpectraInstrumentation.h"

#include <iostream>
#include <chrono>
#include <format>  // For std::format

void SPEC_INSTRUMENTATION SpectraInstrumentationInit() {
}

namespace spectra {
    namespace instrumentation {
        // LogEntry implementations
        LogEntry::LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
            const std::string& subComp, const std::string& msg, const std::vector<std::string>& args)
            : timestamp(std::move(ts)), level(lvl), libraryName(lib), component(comp),
            subComponent(subComp), message(msg), formattedArgs(args) {
        }

        std::string LogEntry::toString() const {
            std::string formatted = std::format("[{}] [{}] {}::{}::{}: {}",
                timestamp, levelToString(level), libraryName, component, subComponent, message);

            if (!formattedArgs.empty()) {
                formatted += " (details: ";
                for (size_t i = 0; i < formattedArgs.size(); ++i) {
                    formatted += formattedArgs[i];
                    if (i < formattedArgs.size() - 1) {
                        formatted += ", ";
                    }
                }
                formatted += ")";
            }

            return formatted;
        }

        std::string LogEntry::levelToString(E_LogLevel level) {
            switch (level) {
            case E_LogLevel::DEBUG: return "DEBUG";
            case E_LogLevel::INFO: return "INFO";
            case E_LogLevel::WARNING: return "WARNING";
            case E_LogLevel::ERROR: return "ERROR";
            }
            return "UNKNOWN";
        }

        // LogHistory implementations
        void LogHistory::addLog(const LogEntry& entry) {
            std::lock_guard<std::mutex> lock(historyMutex);
            history.push_back(entry);
            if (history.size() > MAX_HISTORY_SIZE) {
                history.pop_front();
            }
        }

        std::vector<std::string> LogHistory::getHistory() const {
            std::lock_guard<std::mutex> lock(historyMutex);
            std::vector<std::string> result;
            for (const auto& entry : history) {
                result.push_back(entry.toString());
            }
            return result;
        }

        std::string LogHistory::getHistoryAsString() const {
            std::lock_guard<std::mutex> lock(historyMutex);
            std::string result = "Log History (most recent last):\n";
            for (const auto& entry : history) {
                result += "  " + entry.toString() + "\n";
            }
            return result;
        }

        // LoggedRuntimeError implementations
        LoggedRuntimeError::LoggedRuntimeError(const std::string& message, const LogHistory& logHistory)
            : std::runtime_error(message), history(logHistory.getHistory()) {}

        const std::vector<std::string>& LoggedRuntimeError::getLogHistory() const {
            return history;
        }

        std::string LoggedRuntimeError::getFullMessage() const {
            std::string result = what();
            result += "\nLog History (most recent last):\n";
            for (const auto& log : history) {
                result += "  " + log + "\n";
            }
            return result;
        }

        // BaseLogger implementations
        Instrumentation::BaseLogger::BaseLogger(const std::string& libName)
            : libraryName(libName), enabled(true), minLevel(E_LogLevel::INFO),
            outputDestinations(E_LogOutput::CONSOLE | E_LogOutput::FILE) {
            // Initialize log counts
            for (int i = static_cast<int>(E_LogLevel::DEBUG); i <= static_cast<int>(E_LogLevel::ERROR); ++i) {
                logCounts[static_cast<E_LogLevel>(i)] = 0;
            }
        }

        Instrumentation::BaseLogger::~BaseLogger() {}

        std::string Instrumentation::BaseLogger::getTimestamp() {
            try {
                auto now = std::chrono::system_clock::now();
                auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
                return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
            }
            catch (const std::exception& e) {
                return "[ERROR: Failed to get timestamp: " + std::string(e.what()) + "]";
            }
        }

        bool Instrumentation::BaseLogger::isValidLevel(E_LogLevel level) {
            const int l = static_cast<int>(level);
            return l >= static_cast<int>(E_LogLevel::DEBUG) && l <= static_cast<int>(E_LogLevel::ERROR);
        }

        void Instrumentation::BaseLogger::logInternal(E_LogLevel level, const std::string& component, const std::string& subComponent,
            const std::string& message, const std::vector<std::any>& args) {
            if (!isValidLevel(level)) {
                if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
                    std::cerr << "[WARNING] " << libraryName << ": Invalid log level " << static_cast<int>(level) << ", ignoring log\n";
                }
                return;
            }

            if (!enabled || static_cast<int>(level) < static_cast<int>(minLevel)) return;

            logCounts[level]++;

            // Unpack and format the std::any arguments
            std::vector<std::string> formattedArgs = unpackAndFormatArgs(args);

            // Create a LogEntry object
            LogEntry entry(getTimestamp(), level, libraryName, component, subComponent, message, formattedArgs);

            // Store the log entry in history
            logHistory.addLog(entry);

            // Add the log entry to the central buffer in Instrumentation
            addToBuffer(entry);

            if (level == E_LogLevel::ERROR) {
                throw LoggedRuntimeError(entry.toString(), logHistory);
            }
        }

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
                        if (ptr) {
                            formattedArgs.emplace_back(std::format("0x{:x}", reinterpret_cast<uintptr_t>(ptr)));
                        }
                        else {
                            formattedArgs.emplace_back("null");
                        }
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

        void Instrumentation::BaseLogger::setEnabled(bool enable) {
            enabled = enable;
        }

        bool Instrumentation::BaseLogger::isEnabled() const {
            return enabled;
        }

        void Instrumentation::BaseLogger::setMinLevel(E_LogLevel level) {
            if (isValidLevel(level)) {
                minLevel = level;
            }
            else {
                if (UINT_8(outputDestinations & E_LogOutput::CONSOLE)) {
                    std::cerr << "[WARNING] " << libraryName << ": Invalid log level " << static_cast<int>(level) << ", keeping previous minLevel\n";
                }
            }
        }

        E_LogLevel Instrumentation::BaseLogger::getMinLevel() const {
            return minLevel;
        }

        void Instrumentation::BaseLogger::setOutputDestinations(E_LogOutput destinations) {
            outputDestinations = destinations;
        }

        E_LogOutput Instrumentation::BaseLogger::getOutputDestinations() const {
            return outputDestinations;
        }

        int Instrumentation::BaseLogger::getLogCount(E_LogLevel level) const {
            if (!isValidLevel(level)) return 0;
            auto it = logCounts.find(level);
            return it != logCounts.end() ? it->second : 0;
        }

        int Instrumentation::BaseLogger::getTotalLogCount() const {
            int total = 0;
            for (const auto& pair : logCounts) {
                total += pair.second;
            }
            return total;
        }

        void Instrumentation::BaseLogger::flush() {
        }

        // MathLogger implementations
        Instrumentation::MathLogger::MathLogger() : BaseLogger("spectra::core::math") {}

        Instrumentation::MathLogger& Instrumentation::MathLogger::getInstance() {
            static MathLogger instance;
            return instance;
        }

        // Instrumentation implementations

        void Instrumentation::setMathEnabled(bool enable) {
            MathLogger::getInstance().setEnabled(enable);
        }

        bool Instrumentation::isMathEnabled() {
            return MathLogger::getInstance().isEnabled();
        }

        void Instrumentation::setMathMinLevel(E_LogLevel level) {
            MathLogger::getInstance().setMinLevel(level);
        }

        E_LogLevel Instrumentation::getMathMinLevel() {
            return MathLogger::getInstance().getMinLevel();
        }

        void Instrumentation::setMathOutputDestinations(E_LogOutput destinations) {
            std::lock_guard<std::mutex> lock(bufferMutex);
            if (UINT_8(MathLogger::getInstance().getOutputDestinations() & E_LogOutput::FILE) && !UINT_8(destinations & E_LogOutput::FILE) && mathFileStream.is_open()) {
                mathFileStream.close();
            }

            if (!UINT_8(MathLogger::getInstance().getOutputDestinations() & E_LogOutput::FILE) && UINT_8(destinations & E_LogOutput::FILE)) {
                mathFileStream.open(mathFileName, std::ios::app);
                if (!mathFileStream.is_open()) {
                    if (UINT_8(destinations & E_LogOutput::CONSOLE)) {
                        std::cerr << "[WARNING] spectra::core::math: Failed to open log file " << mathFileName << ", disabling file output\n";
                    }
                }
            }

            MathLogger::getInstance().setOutputDestinations(destinations);
        }

        E_LogOutput Instrumentation::getMathOutputDestinations() {
            return MathLogger::getInstance().getOutputDestinations();
        }

        int Instrumentation::getMathLogCount(E_LogLevel level) {
            return MathLogger::getInstance().getLogCount(level);
        }

        int Instrumentation::getMathTotalLogCount() {
            return MathLogger::getInstance().getTotalLogCount();
        }

        void Instrumentation::flushMath() {
            MathLogger::getInstance().flush();
        }

        void Instrumentation::addToBuffer(const LogEntry& entry) {
            std::lock_guard<std::mutex> lock(bufferMutex);
            logBuffer.push_back(entry);
        }

        void Instrumentation::flush() {
            std::lock_guard<std::mutex> lock(bufferMutex);
            if (logBuffer.empty()) return;

            // Flush MathLogger
            if (UINT_8(MathLogger::getInstance().getOutputDestinations() & E_LogOutput::CONSOLE)) {
                for (const auto& entry : logBuffer) {
                    std::cerr << "[TEMP] " << entry.toString() << "\n";
                }
                std::cerr.flush();
            }
            if (UINT_8(MathLogger::getInstance().getOutputDestinations() & E_LogOutput::FILE)) {
                if (mathFileStream.is_open()) {
                    for (const auto& entry : logBuffer) {
                        mathFileStream << "[PERM] " << entry.toString() << "\n";
                    }
                    mathFileStream.flush();
                }
            }

            logBuffer.clear();
        }
    }
}