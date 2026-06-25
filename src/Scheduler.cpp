#include "Scheduler.h"
#include <chrono>
#include <iostream>

Scheduler::Scheduler() : config(ConfigParser::getDefaults()), coreProcess(config.numCpu, nullptr) {}

Scheduler::~Scheduler() {
    stop();
}

bool Scheduler::loadConfig(const std::string& filePath) {
    if (running.load()) {
        std::cerr << "Cannot load config while scheduler is running.\n";
        return false;
    }

    config = ConfigParser::loadConfig(filePath);

    // Resize core process vector to match new CPU count
    coreProcess.clear();
    coreProcess.resize(config.numCpu, nullptr);

    return true;
}

void Scheduler::start() {
    if (running.load()) return;
    running.store(true);

    // Spawn one worker thread per core (using config value)
    for (int i = 0; i < static_cast<int>(config.numCpu); ++i) {
        workerThreads.emplace_back(&Scheduler::workerLoop, this, i);
    }
}

void Scheduler::stop() {
    if (!running.load()) return;
    running.store(false);

    // Wake all waiting workers so they can exit
    readyCV.notify_all();

    for (auto& t : workerThreads) {
        if (t.joinable()) t.join();
    }
    workerThreads.clear();
}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(allMutex);
        allProcesses.push_back(process);
    }
    {
        std::lock_guard<std::mutex> lock(readyMutex);
        readyQueue.push(process);
    }
    readyCV.notify_one();
}

// Each worker thread picks one process from the ready queue and executes it
void Scheduler::workerLoop(int coreId) {
    while (running.load()) {
        std::shared_ptr<Process> proc;

        // Wait for a process to become available
        {
            std::unique_lock<std::mutex> lock(readyMutex);
            readyCV.wait(lock, [this] {
                return !readyQueue.empty() || !running.load();
            });

            if (!running.load() && readyQueue.empty()) return;
            if (readyQueue.empty()) continue;

            proc = readyQueue.front();
            readyQueue.pop();
        }

        // Initialize log file with header (only the very first time the process is run)
        if (proc->currentInstruction.load() == 0) {
            proc->initLogFile();
        }

        // Mark as running on this core
        proc->state.store(ProcessState::RUNNING);
        proc->coreId.store(coreId);

        {
            std::lock_guard<std::mutex> lock(coreMutex);
            coreProcess[coreId] = proc;
        }

        // Execute instructions (or process sleep ticks)
        while (proc->currentInstruction.load() < proc->totalInstructions || proc->sleepTicksRemaining.load() > 0) {
            if (!running.load()) {
                // Scheduler stopped; put process back so it can resume later
                {
                    std::lock_guard<std::mutex> lock(readyMutex);
                    proc->state.store(ProcessState::WAITING);
                    proc->coreId.store(-1);
                    readyQueue.push(proc);
                }
                break;
            }
            
            bool yield = proc->executeOneInstruction(coreId);

            // Small yield to avoid starving other host OS threads
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (yield) {
                // Process yielded CPU (e.g., SLEEP) -> Relinquish
                proc->state.store(ProcessState::WAITING);
                proc->coreId.store(-1);
                {
                    std::lock_guard<std::mutex> lock(readyMutex);
                    readyQueue.push(proc);
                }
                readyCV.notify_one(); // Notify in case another worker is currently idle
                break; // Break the execution loop to fetch a new process
            }
        }

        // Clear core slot
        {
            std::lock_guard<std::mutex> lock(coreMutex);
            coreProcess[coreId] = nullptr;
        }
    }
}

std::vector<Scheduler::ProcessSnapshot> Scheduler::getSnapshot() const {
    std::vector<ProcessSnapshot> result;

    std::lock_guard<std::mutex> lock(allMutex);
    for (const auto& p : allProcesses) {
        ProcessSnapshot snap;
        snap.name      = p->name;
        snap.createdAt = p->createdAt;
        snap.current   = p->currentInstruction.load();
        snap.total     = p->totalInstructions;
        snap.finished  = (p->state.load() == ProcessState::FINISHED);
        snap.core      = p->coreId.load();
        result.push_back(snap);
    }
    return result;
}

std::shared_ptr<Process> Scheduler::findProcess(const std::string& name) const {
    std::lock_guard<std::mutex> lock(allMutex);
    for (const auto& p : allProcesses) {
        if (p->name == name) return p;
    }
    return nullptr;
}