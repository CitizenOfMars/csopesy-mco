#include "Scheduler.h"
#include <chrono>

Scheduler::Scheduler() : coreProcess(NUM_CORES, nullptr) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (running.load()) return;
    running.store(true);

    // Spawn one worker thread per core
    for (int i = 0; i < NUM_CORES; ++i) {
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

// Each worker thread picks one process from the FCFS queue and runs it to completion
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

        // Mark as running on this core
        proc->state.store(ProcessState::RUNNING);
        proc->coreId.store(coreId);

        {
            std::lock_guard<std::mutex> lock(coreMutex);
            coreProcess[coreId] = proc;
        }

        // Initialize log file with header (only first time)
        proc->initLogFile();

        // Execute all instructions one by one (each is one "print" command)
        while (proc->currentInstruction.load() < proc->totalInstructions) {
            if (!running.load()) {
                // Scheduler stopped; put process back so it can resume later
                {
                    std::lock_guard<std::mutex> lock(readyMutex);
                    readyQueue.push(proc);
                }
                break;
            }
            proc->executeOneInstruction(coreId);

            // Small yield to avoid starving other threads / burning CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
