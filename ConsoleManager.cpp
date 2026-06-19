#include "ConsoleManager.h"
#include "UI_Utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

// ─── helpers ────────────────────────────────────────────────────────────────

namespace {
    // Split a line into tokens (whitespace-delimited)
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::istringstream ss(line);
        std::string tok;
        while (ss >> tok) tokens.push_back(tok);
        return tokens;
    }

    // Left-pad a string to a fixed width
    std::string padRight(const std::string& s, size_t width) {
        if (s.size() >= width) return s;
        return s + std::string(width - s.size(), ' ');
    }
}

// ─── ctor ────────────────────────────────────────────────────────────────────

ConsoleManager::ConsoleManager() {}

// ─── main loop ───────────────────────────────────────────────────────────────

void ConsoleManager::startMainLoop() {
    UI_Utils::printHeader();

    std::string inputLine;
    while (true) {
        std::cout << "Enter a command: ";
        if (!std::getline(std::cin, inputLine)) break;

        if (processCommand(inputLine)) break;
    }
}

// ─── command dispatch ────────────────────────────────────────────────────────

bool ConsoleManager::processCommand(const std::string& rawLine) {
    auto tokens = tokenize(rawLine);
    if (tokens.empty()) return false;

    const std::string& cmd = tokens[0];

    // ── exit ────────────────────────────────────────────────────────────────
    if (cmd == "exit") {
        scheduler.stop();
        std::cout << "Exiting emulator.\n";
        return true;
    }

    // ── initialize ──────────────────────────────────────────────────────────
    if (cmd == "initialize") {
        if (tokens.size() != 1) {
            std::cout << "Usage: initialize\n";
            return false;
        }
        cmdInitialize();
        return false;
    }

    // ── screen ──────────────────────────────────────────────────────────────
    if (cmd == "screen") {
        if (tokens.size() == 2 && tokens[1] == "-ls") {
            cmdScreen("-ls", "");
        } else if (tokens.size() == 3 && (tokens[1] == "-s" || tokens[1] == "-r")) {
            cmdScreen(tokens[1], tokens[2]);
        } else {
            std::cout << "Usage:\n"
                      << "  screen -ls          list all processes\n"
                      << "  screen -s <name>    create and view a new process\n"
                      << "  screen -r <name>    re-attach to an existing process\n";
        }
        return false;
    }

    // ── scheduler-start / scheduler-stop ────────────────────────────────────
    if (cmd == "scheduler-start") {
        if (!initialized.load()) {
            std::cout << "Please run 'initialize' first.\n";
        } else {
            cmdSchedulerStart();
        }
        return false;
    }

    if (cmd == "scheduler-stop") {
        cmdSchedulerStop();
        return false;
    }

    // ── report-util ─────────────────────────────────────────────────────────
    if (cmd == "report-util") {
        if (!initialized.load()) {
            std::cout << "Please run 'initialize' first.\n";
        } else {
            cmdReportUtil();
        }
        return false;
    }

    // ── clear ───────────────────────────────────────────────────────────────
    if (cmd == "clear") {
        UI_Utils::clearScreen();
        UI_Utils::printHeader();
        return false;
    }

    std::cout << "Command not recognized: '" << cmd << "'\n";
    return false;
}

// ─── initialize ──────────────────────────────────────────────────────────────

void ConsoleManager::cmdInitialize() {
    if (initialized.load()) {
        std::cout << "Already initialized.\n";
        return;
    }
    initialized.store(true);

    // Start the scheduler (worker threads spin up)
    scheduler.start();

    // Test case requirement: create 10 processes, each with 100 print commands
    const int NUM_TEST_PROCESSES = 10;
    const int INSTRUCTIONS_EACH  = 100;

    std::cout << "Initializing OS Emulator...\n";
    std::cout << "Creating " << NUM_TEST_PROCESSES
              << " test processes with " << INSTRUCTIONS_EACH
              << " print commands each.\n";

    for (int i = 1; i <= NUM_TEST_PROCESSES; ++i) {
        std::ostringstream oss;
        oss << "process" << std::setfill('0') << std::setw(2) << i;
        auto proc = std::make_shared<Process>(oss.str(), INSTRUCTIONS_EACH);
        scheduler.addProcess(proc);
    }
    processCounter = NUM_TEST_PROCESSES;

    std::cout << "Scheduler started with " << NUM_CORES << " CPU cores.\n";
    std::cout << "Type 'screen -ls' to view process status.\n";
}

// ─── screen dispatcher ───────────────────────────────────────────────────────

void ConsoleManager::cmdScreen(const std::string& flag, const std::string& arg) {
    if (flag == "-ls") {
        cmdScreenList();
    } else if (flag == "-s") {
        if (!initialized.load()) {
            std::cout << "Please run 'initialize' first.\n";
            return;
        }
        cmdScreenNew(arg);
    } else if (flag == "-r") {
        cmdScreenResume(arg);
    }
}

// ─── screen -ls ──────────────────────────────────────────────────────────────

void ConsoleManager::cmdScreenList() {
    printProcessList();
}

void ConsoleManager::printProcessList() const {
    auto snapshot = scheduler.getSnapshot();

    std::cout << "\n" << std::string(60, '-') << "\n";
    std::cout << "Running processes:\n";

    bool anyRunning = false;
    for (const auto& s : snapshot) {
        if (!s.finished) {
            anyRunning = true;
            std::string coreStr = (s.core >= 0)
                ? "Core: " + std::to_string(s.core)
                : "Waiting ";
            std::cout << padRight(s.name, 12)
                      << " " << padRight(s.createdAt, 24)
                      << " " << padRight(coreStr, 8)
                      << "   " << s.current << " / " << s.total
                      << "\n";
        }
    }
    if (!anyRunning) std::cout << "  (none)\n";

    std::cout << "\nFinished processes:\n";
    bool anyFinished = false;
    for (const auto& s : snapshot) {
        if (s.finished) {
            anyFinished = true;
            std::cout << padRight(s.name, 12)
                      << " " << padRight(s.createdAt, 24)
                      << " Finished"
                      << "   " << s.total << " / " << s.total
                      << "\n";
        }
    }
    if (!anyFinished) std::cout << "  (none)\n";

    std::cout << std::string(60, '-') << "\n\n";
}

// ─── screen -s ───────────────────────────────────────────────────────────────

void ConsoleManager::cmdScreenNew(const std::string& name) {
    // Reject duplicate names
    if (scheduler.findProcess(name) != nullptr) {
        std::cout << "A process named '" << name << "' already exists. "
                  << "Use 'screen -r " << name << "' to attach.\n";
        return;
    }

    // Default to 100 instructions for manually created processes
    auto proc = std::make_shared<Process>(name, 100);
    scheduler.addProcess(proc);
    ++processCounter;

    std::cout << "\nCreated process '" << name << "' with 100 print commands.\n";
    printProcessView(proc);
}

// ─── screen -r ───────────────────────────────────────────────────────────────

void ConsoleManager::cmdScreenResume(const std::string& name) {
    auto proc = scheduler.findProcess(name);
    if (!proc) {
        std::cout << "No process named '" << name << "' found.\n";
        return;
    }
    printProcessView(proc);
}

// ─── process detail view ─────────────────────────────────────────────────────

void ConsoleManager::printProcessView(const std::shared_ptr<Process>& proc) const {
    ProcessState st = proc->state.load();
    std::cout << "\n" << std::string(40, '-') << "\n";
    std::cout << "Process: " << proc->name << "\n";
    std::cout << "Created: " << proc->createdAt << "\n";

    if (st == ProcessState::FINISHED) {
        std::cout << "Status : Finished\n";
        std::cout << "Instructions: " << proc->totalInstructions
                  << " / " << proc->totalInstructions << "\n";
    } else {
        int cur  = proc->currentInstruction.load();
        int core = proc->coreId.load();
        std::cout << "Status : " << (st == ProcessState::RUNNING ? "Running" : "Waiting") << "\n";
        if (core >= 0)
            std::cout << "Core   : " << core << "\n";
        std::cout << "Instructions: " << cur
                  << " / " << proc->totalInstructions << "\n";
    }
    std::cout << std::string(40, '-') << "\n\n";
}

// ─── scheduler-start ─────────────────────────────────────────────────────────

void ConsoleManager::cmdSchedulerStart() {
    if (scheduler.isRunning()) {
        std::cout << "Scheduler is already running.\n";
    } else {
        scheduler.start();
        std::cout << "Scheduler started.\n";
    }
}

// ─── scheduler-stop ──────────────────────────────────────────────────────────

void ConsoleManager::cmdSchedulerStop() {
    if (!scheduler.isRunning()) {
        std::cout << "Scheduler is not running.\n";
    } else {
        scheduler.stop();
        std::cout << "Scheduler stopped.\n";
    }
}

// ─── report-util ─────────────────────────────────────────────────────────────

void ConsoleManager::cmdReportUtil() {
    auto snapshot = scheduler.getSnapshot();

    int total    = static_cast<int>(snapshot.size());
    int finished = 0;
    int running  = 0;

    for (const auto& s : snapshot) {
        if (s.finished)       ++finished;
        else if (s.core >= 0) ++running;
    }

    int idle = NUM_CORES - running;

    double utilPct = (total == 0) ? 0.0
                                  : (running * 100.0 / NUM_CORES);

    std::cout << "\n--- CPU Utilization Report ---\n";
    std::cout << "CPU cores      : " << NUM_CORES  << "\n";
    std::cout << "Cores in use   : " << running    << "\n";
    std::cout << "Cores idle     : " << idle       << "\n";
    std::cout << std::fixed << std::setprecision(1)
              << "CPU utilization: " << utilPct    << "%\n";
    std::cout << "Total processes: " << total      << "\n";
    std::cout << "Finished       : " << finished   << "\n";
    std::cout << "Running/Waiting: " << (total - finished) << "\n";
    std::cout << "------------------------------\n\n";
}
