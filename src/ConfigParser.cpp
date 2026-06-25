#include "ConfigParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>

Config ConfigParser::getDefaults() {
    return Config{
        .numCpu = 4,
        .scheduler = "rr",
        .quantumCycles = 5,
        .batchProcessFreq = 1,
        .minIns = 1000,
        .maxIns = 2000,
        .delaysPerExec = 0
    };
}

Config ConfigParser::loadConfig(const std::string& filePath) {
    Config config = getDefaults();

    // Try to find config.txt in multiple locations
    std::vector<std::string> searchPaths = {
        filePath,                           // Current path as specified
        "../" + filePath,                  // Parent directory
        "../../" + filePath,               // Grandparent directory
        "./config/" + filePath             // config subdirectory
    };

    std::ifstream file;
    std::string foundPath;
    for (const auto& path : searchPaths) {
        file.open(path);
        if (file.is_open()) {
            foundPath = path;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filePath
                  << " in any searched location. Using default configuration.\n";
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;

        // Parse different config parameters
        if (key == "num-cpu") {
            uint32_t val;
            if (iss >> val && val >= 1 && val <= 128) {
                config.numCpu = val;
            } else {
                std::cerr << "Warning: Invalid num-cpu value. Using default (4).\n";
            }
        }
        else if (key == "scheduler") {
            std::string val;
            if (iss >> val) {
                // Remove quotes if present
                if (!val.empty() && val.front() == '"' && val.back() == '"') {
                    val = val.substr(1, val.length() - 2);
                }
                if (val == "fcfs" || val == "rr") {
                    config.scheduler = val;
                } else {
                    std::cerr << "Warning: Invalid scheduler value. Must be 'fcfs' or 'rr'. Using default (rr).\n";
                }
            }
        }
        else if (key == "quantum-cycles") {
            uint32_t val;
            if (iss >> val && val >= 1) {
                config.quantumCycles = val;
            } else {
                std::cerr << "Warning: Invalid quantum-cycles value. Using default (5).\n";
            }
        }
        else if (key == "batch-process-freq") {
            uint32_t val;
            if (iss >> val && val >= 1) {
                config.batchProcessFreq = val;
            } else {
                std::cerr << "Warning: Invalid batch-process-freq value. Using default (1).\n";
            }
        }
        else if (key == "min-ins") {
            uint32_t val;
            if (iss >> val && val >= 1) {
                config.minIns = val;
            } else {
                std::cerr << "Warning: Invalid min-ins value. Using default (1000).\n";
            }
        }
        else if (key == "max-ins") {
            uint32_t val;
            if (iss >> val && val >= 1) {
                config.maxIns = val;
            } else {
                std::cerr << "Warning: Invalid max-ins value. Using default (2000).\n";
            }
        }
        else if (key == "delays-per-exec") {
            uint32_t val;
            if (iss >> val) {
                config.delaysPerExec = val;
            } else {
                std::cerr << "Warning: Invalid delays-per-exec value. Using default (0).\n";
            }
        }
    }

    file.close();

    // Validate that max-ins >= min-ins
    if (config.maxIns < config.minIns) {
        std::cerr << "Warning: max-ins (" << config.maxIns 
                  << ") < min-ins (" << config.minIns 
                  << "). Swapping values.\n";
        std::swap(config.minIns, config.maxIns);
    }

    return config;
}

void ConfigParser::saveConfig(const std::string& filePath, const Config& cfg) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filePath << " for writing.\n";
        return;
    }

    file << "num-cpu " << cfg.numCpu << "\n";
    file << "scheduler \"" << cfg.scheduler << "\"\n";
    file << "quantum-cycles " << cfg.quantumCycles << "\n";
    file << "batch-process-freq " << cfg.batchProcessFreq << "\n";
    file << "min-ins " << cfg.minIns << "\n";
    file << "max-ins " << cfg.maxIns << "\n";
    file << "delays-per-exec " << cfg.delaysPerExec << "\n";

    file.close();
    std::cout << "Configuration saved to " << filePath << "\n";
}
