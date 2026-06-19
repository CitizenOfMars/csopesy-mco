#pragma once
#include "Process.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <string>

static constexpr int NUM_CORES = 4;

class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    // Start/stop the scheduler and worker threads
    void start();
    void stop();
    bool isRunning() const { return running.load(); }

    // Add a new process to the ready queue
    void addProcess(std::shared_ptr<Process> process);

    // Snapshot for screen -ls display
    struct ProcessSnapshot {
        std::string name;
        std::string createdAt;
        int core;           // -1 if finished
        int current;
        int total;
        bool finished;
    };
    std::vector<ProcessSnapshot> getSnapshot() const;

    // Find a process by name (returns nullptr if not found)
    std::shared_ptr<Process> findProcess(const std::string& name) const;

private:
    // One worker loop per core
    void workerLoop(int coreId);

    std::atomic<bool> running{false};

    // Ready queue protected by readyMutex
    mutable std::mutex readyMutex;
    std::condition_variable readyCV;
    std::queue<std::shared_ptr<Process>> readyQueue;

    // Per-core: current process being executed (nullptr = idle)
    mutable std::mutex coreMutex;
    std::vector<std::shared_ptr<Process>> coreProcess; // size NUM_CORES

    // All processes ever added (for lookup and snapshot)
    mutable std::mutex allMutex;
    std::vector<std::shared_ptr<Process>> allProcesses;

    std::vector<std::thread> workerThreads;
};
