#pragma once
#include "Sinks.h"
#include "Records.h"
#include <fstream>
#include <ctime>

namespace TestRunner {

    // FileSinkRouter
    //
    // Concrete ISinkRouter that writes all entry types to a UTF-8 text file.
    // One router per file; the file is opened at construction and closed at destruction.
    // Thread-safety: flush() is called single-threaded from the Stratum orchestrator,
    // so no locking is needed here.
    class FileSinkRouter final : public Stratum::Logging::ISinkRouter {
    public:
        explicit FileSinkRouter(const char* p_Path) noexcept
            : m_file(p_Path, std::ios::out | std::ios::trunc) {
            if (m_file.is_open()) {
                std::time_t now = std::time(nullptr);
                char timeBuf[64]{};
                std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
                m_file << "=== KerbecsValidation run @ " << timeBuf << " ===\n\n";
                m_file.flush();
            }
        }

        ~FileSinkRouter() override {
            if (m_file.is_open()) {
                m_file << "\n=== end of log ===\n";
                m_file.close();
            }
        }

        FileSinkRouter(const FileSinkRouter&)            = delete;
        FileSinkRouter& operator=(const FileSinkRouter&) = delete;
        FileSinkRouter(FileSinkRouter&&)                 = delete;
        FileSinkRouter& operator=(FileSinkRouter&&)      = delete;

        void write(const Stratum::Records::LogEntry& ro_Entry) noexcept override {
            if (!m_file.is_open()) return;
            const char* level = levelTag(ro_Entry.getLevel());
            m_file << "[LOG  ][" << level << "] "
                   << ro_Entry.getComponent() << ": "
                   << ro_Entry.getMessage()
                   << " (" << ro_Entry.getLocation().file_name()
                   << ':' << ro_Entry.getLocation().line() << ")\n";
        }

        void write(const Stratum::Records::ExceptionEntry& ro_Entry) noexcept override {
            if (!m_file.is_open()) return;
            m_file << "[FAIL]       "
                   << ro_Entry.getComponent() << ": "
                   << ro_Entry.getErrorMessage()
                   << " (" << ro_Entry.getLocation().file_name()
                   << ':' << ro_Entry.getLocation().line() << ")\n";
        }

        void write(const Stratum::Records::TracerEntry& ro_Entry) noexcept override {
            if (!m_file.is_open()) return;
            m_file << "[TRACE]      "
                   << ro_Entry.getComponent() << ": "
                   << ro_Entry.getLabel()
                   << " duration=" << ro_Entry.getDuration(Stratum::Records::TracerPrecision::Microseconds)
                   << "us\n";
        }

        bool isOpen() const noexcept { return m_file.is_open(); }

    private:
        static const char* levelTag(Stratum::Records::LogLevel v_Level) noexcept {
            switch (v_Level) {
            case Stratum::Records::LogLevel::Info:    return "INFO ";
            case Stratum::Records::LogLevel::Warning: return "WARN ";
            case Stratum::Records::LogLevel::Debug:   return "DEBUG";
            case Stratum::Records::LogLevel::Error:   return "ERROR";
            case Stratum::Records::LogLevel::Crash:   return "CRASH";
            default:                                  return "?    ";
            }
        }

        std::ofstream m_file;
    };

} // namespace TestRunner
