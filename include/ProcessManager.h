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
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include <iostream>

enum class ProcessState {
    READY,
    RUNNING,
    WAITING, 
    FINISHED
};

// --- Instruction Set Definitions ---
enum class OpCode {
    PRINT,
    DECLARE,
    ADD,
    SUBTRACT,
    SLEEP
};

struct Instruction {
    OpCode op;
    std::string arg1; 
    std::string arg2; 
    std::string arg3; 
};

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

    std::string createdAt;
    std::mutex logMutex;

    // --- Memory and Instruction Storage ---
    std::unordered_map<std::string, uint16_t> memory;
    std::vector<Instruction> instructions;
    std::atomic<int> sleepTicksRemaining{0};

    uint16_t evaluate(const std::string& operand) {
        if (operand.empty()) return 0;
        
        bool isNumber = true;
        for (char c : operand) {
            if (c < '0' || c > '9') {
                isNumber = false;
                break;
            }
        }
        
        if (isNumber) {
            try {
                return static_cast<uint16_t>(std::stoul(operand));
            } catch (...) {
                return 0; 
            }
        }
        
        return memory[operand];
    }

    Process(const std::string& n, const std::vector<Instruction>& instrs)
        : name(n), instructions(instrs) {
        totalInstructions = static_cast<int>(instructions.size());
        createdAt = getCurrentTimestamp();
    }

    Process(const std::string& n, int total)
        : name(n), totalInstructions(total) {
        createdAt = getCurrentTimestamp();
        
        for(int i = 0; i < total; i++) {
            instructions.push_back({OpCode::PRINT, "Hello world from " + name + "!", "", ""});
        }
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    bool isFinished() const {
        return state.load() == ProcessState::FINISHED;
    }

    bool executeOneInstruction(int core) {
        if (sleepTicksRemaining.load() > 0) {
            sleepTicksRemaining--;
            
            if (sleepTicksRemaining.load() == 0 && currentInstruction.load() >= totalInstructions) {
                state.store(ProcessState::FINISHED);
                coreId.store(core);
                return false; 
            }
            return true; 
        }

        int idx = currentInstruction.fetch_add(1);
        if (idx >= totalInstructions) {
            return false;
        }

        const Instruction& inst = instructions[idx];
        std::string ts = getCurrentTimestamp();
        bool yieldCpu = false;

        switch (inst.op) {
            case OpCode::PRINT: {
                std::string output = inst.arg1;
                if (!inst.arg2.empty()) {
                    output += std::to_string(evaluate(inst.arg2));
                }
                
                std::lock_guard<std::mutex> lock(logMutex);
                // UPDATED: Prepend "bin/" to the filename path
                std::string filePath = "bin/" + name + ".txt";
                std::ofstream file(filePath, std::ios::app);
                if (file.is_open()) {
                    file << ts << " Core:" << core << " \"" << output << "\"\n";
                } else {
                    // Fallback just in case the bin folder is missing from the working directory
                    std::ofstream fallback(name + ".txt", std::ios::app);
                    if (fallback.is_open()) fallback << ts << " Core:" << core << " \"" << output << "\"\n";
                }
                break;
            }
            case OpCode::DECLARE: {
                memory[inst.arg1] = evaluate(inst.arg2);
                break;
            }
            case OpCode::ADD: {
                uint32_t val = evaluate(inst.arg2) + evaluate(inst.arg3);
                memory[inst.arg1] = static_cast<uint16_t>(std::min<uint32_t>(val, 65535));
                break;
            }
            case OpCode::SUBTRACT: {
                int32_t val = evaluate(inst.arg2) - evaluate(inst.arg3);
                memory[inst.arg1] = static_cast<uint16_t>(std::max<int32_t>(val, 0));
                break;
            }
            case OpCode::SLEEP: {
                int ticks = evaluate(inst.arg1);
                if (ticks > 0) {
                    sleepTicksRemaining.store(ticks);
                    yieldCpu = true; 
                }
                break;
            }
        }

        if (currentInstruction.load() >= totalInstructions && sleepTicksRemaining.load() == 0) {
            state.store(ProcessState::FINISHED);
            coreId.store(core);
        }

        return yieldCpu;
    }

    void initLogFile() {
        std::lock_guard<std::mutex> lock(logMutex);
        // UPDATED: Prepend "bin/" to the filename path
        std::string filePath = "bin/" + name + ".txt";
        std::ofstream file(filePath, std::ios::trunc);
        if (file.is_open()) {
            file << "Process name: " << name << "\n";
            file << "Logs:\n\n";
        } else {
            // Fallback just in case the bin folder is missing from the working directory
            std::ofstream fallback(name + ".txt", std::ios::trunc);
            if (fallback.is_open()) {
                fallback << "Process name: " << name << "\n";
                fallback << "Logs:\n\n";
            }
        }
    }
};