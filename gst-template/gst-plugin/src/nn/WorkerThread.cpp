#include "WorkerThread.h"

void WorkerThread::start(std::function<void()> func) {
    running_ = true;
    if (thread_.joinable()) return;

    thread_ = std::thread(std::move(func));
}

void WorkerThread::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool WorkerThread::isRunning() const {
    return running_;
}
