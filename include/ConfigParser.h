#pragma once
#include <cstdint>
#include <string>

struct Config {
    // Number of CPU cores
    uint32_t numCpu;

    // Scheduler algorithm: "fcfs" or "rr"
    std::string scheduler;

    // Time slice for round-robin (in CPU cycles)
    uint32_t quantumCycles;

    // Frequency of process generation in scheduler-start (in CPU cycles)
    uint32_t batchProcessFreq;

    // Min/max instructions per process
    uint32_t minIns;
    uint32_t maxIns;

    // Delay before executing next instruction (in CPU cycles, busy-wait)
    uint32_t delaysPerExec;
};

class ConfigParser {
public:
    // Load configuration from config.txt file
    static Config loadConfig(const std::string& filePath = "config.txt");

    // Save configuration to file
    static void saveConfig(const std::string& filePath, const Config& cfg);

    // Get default configuration
    static Config getDefaults();
};
