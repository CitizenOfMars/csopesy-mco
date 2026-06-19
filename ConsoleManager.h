#pragma once
#include "Scheduler.h"
#include <string>
#include <memory>
#include <atomic>

class ConsoleManager {
public:
    ConsoleManager();
    void startMainLoop();

private:
    // Command dispatch — returns true when the emulator should exit
    bool processCommand(const std::string& rawLine);

    // Sub-command handlers
    void cmdInitialize();
    void cmdScreen(const std::string& flag, const std::string& arg);
    void cmdScreenList();
    void cmdScreenNew(const std::string& name);
    void cmdScreenResume(const std::string& name);
    void cmdSchedulerStart();
    void cmdSchedulerStop();
    void cmdReportUtil();

    // Pretty-print the process list (used by screen -ls)
    void printProcessList() const;

    // Print the "attached" view for a single process
    void printProcessView(const std::shared_ptr<Process>& proc) const;

    Scheduler scheduler;
    std::atomic<bool> initialized{false};

    // Counter used when auto-generating process names
    int processCounter{0};
};
