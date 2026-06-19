#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

enum class ProcessState {
    READY,
    RUNNING,
    FINISHED
};

// Returns a formatted timestamp string: (MM/DD/YYYY HH:MM:SSAM/PM)
inline std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
    int hour = tm_info.tm_hour;
    const char* ampm = (hour >= 12) ? "PM" : "AM";
    if (hour == 0) hour = 12;
    else if (hour > 12) hour -= 12;

    std::ostringstream oss;
    oss << "("
        << std::setfill('0') << std::setw(2) << (tm_info.tm_mon + 1) << "/"
        << std::setw(2) << tm_info.tm_mday << "/"
        << (tm_info.tm_year + 1900) << " "
        << std::setw(2) << hour << ":"
        << std::setw(2) << tm_info.tm_min << ":"
        << std::setw(2) << tm_info.tm_sec << ampm
        << ")";
    return oss.str();
}

struct Process {
    std::string name;
    int totalInstructions;
    std::atomic<int> currentInstruction{0};
    std::atomic<ProcessState> state{ProcessState::READY};
    std::atomic<int> coreId{-1};

    // Creation timestamp (set once when process is created)
    std::string createdAt;

    // Mutex for log file writes
    std::mutex logMutex;

    Process(const std::string& n, int total)
        : name(n), totalInstructions(total) {
        createdAt = getCurrentTimestamp();
    }

    // Non-copyable due to mutex and atomics
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    bool isFinished() const {
        return state.load() == ProcessState::FINISHED;
    }

    // Called by a worker thread each time it executes one print instruction
    void executeOneInstruction(int core) {
        int idx = currentInstruction.fetch_add(1);
        if (idx >= totalInstructions) {
            // Already done — shouldn't happen if scheduler checks first
            return;
        }

        std::string ts = getCurrentTimestamp();

        // Write to log file
        {
            std::lock_guard<std::mutex> lock(logMutex);
            std::ofstream file(name + ".txt", std::ios::app);
            if (file.is_open()) {
                file << ts << " Core:" << core << " \"Hello world from " << name << "!\"\n";
            }
        }

        // If this was the last instruction, mark finished
        if (idx + 1 >= totalInstructions) {
            state.store(ProcessState::FINISHED);
            coreId.store(core);
        }
    }

    // Write the process name / Logs: header to the file (called once when process is first assigned)
    void initLogFile() {
        std::lock_guard<std::mutex> lock(logMutex);
        std::ofstream file(name + ".txt", std::ios::trunc);
        if (file.is_open()) {
            file << "Process name: " << name << "\n";
            file << "Logs:\n\n";
        }
    }
};
