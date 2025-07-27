#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

enum class OverflowPolicy {
    DropNewest,
    DropOldest
};

template<typename T>
class SafeQueue {
public:
    SafeQueue(size_t max_size = 0, OverflowPolicy policy = OverflowPolicy::DropOldest)
        : max_size_(max_size), overflow_policy_(policy) {}

    using value_type = T;

    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (shutdown_) return;

        if (max_size_ > 0 && queue_.size() >= max_size_) {
            if (overflow_policy_ == OverflowPolicy::DropNewest) {
                return;
            } else if (overflow_policy_ == OverflowPolicy::DropOldest) {
                queue_.pop();
            }
        }

        queue_.push(value);
        cv_.notify_one();
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]() { return shutdown_ || !queue_.empty(); });

        if (shutdown_ && queue_.empty()) return false;

        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty() || shutdown_) return false;

        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool wait_and_pop(T& value, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!cv_.wait_for(lock, timeout, [this] { return shutdown_ || !queue_.empty(); })) {
            return false; // Timeout
        }
        
        if (shutdown_ && queue_.empty()) return false;
        value = queue_.front();
        queue_.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    bool is_shutdown() const {
        return shutdown_;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = false;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx_);
        std::queue<T>().swap(queue_);
    }

    void setName(const char* name) {
        std::lock_guard<std::mutex> lock(mtx_);
        name_ = name;
    }

    std::string getName() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return name_;
    }

    void setMaxSize(size_t max_size) {
        std::lock_guard<std::mutex> lock(mtx_);
        max_size_ = max_size;
    }

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    std::atomic<bool> shutdown_ = false;
    std::string name_;

    size_t max_size_ = 0;
    OverflowPolicy overflow_policy_ = OverflowPolicy::DropOldest;
};
