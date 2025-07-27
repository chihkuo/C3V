#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <thread>
#include <atomic>
#include <functional>
#include <iostream>

template<typename InputQueue, typename OutputQueue, typename Func>
void worker_loop(InputQueue& input, OutputQueue* output, std::atomic<bool>& running, Func process_func) {
    using TaskDataPtr = typename InputQueue::value_type;
    while (running) {
        TaskDataPtr task;
        if (!input.pop(task)) break;

        process_func(task);

        if (output && task) {
            output->push(task);
        }
    }
}

class WorkerThread {
public:
    void start(std::function<void()> func);
    void stop();
    bool isRunning() const;

private:
    std::thread thread_;
    std::atomic<bool> running_ = false;
};

#endif
