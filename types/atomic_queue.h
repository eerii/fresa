//* atomic_queue
//      thread safe queue
#pragma once

#include <queue>
#include <atomic>
#include <optional>

namespace fresa
{
    struct SpinLock {
        std::atomic_flag flag;
        void lock() noexcept { while( flag.test_and_set(std::memory_order_acquire) ); }
        void unlock() noexcept { flag.clear(std::memory_order_release); }
        bool try_lock() noexcept { return !flag.test_and_set(std::memory_order_acquire); }
    };

    template <typename T>
    struct AtomicQueue {
        std::queue<T> queue;
        SpinLock lock;

        AtomicQueue() noexcept {}; //: constructor
        AtomicQueue(const AtomicQueue<T>&) = delete; //: no copy
        AtomicQueue& operator=(const AtomicQueue<T>&) = delete;
        AtomicQueue(AtomicQueue<T>&&) noexcept {}; //: move
        AtomicQueue& operator=(AtomicQueue<T>&&) {};

        void push(const T& value) {
            std::lock_guard<SpinLock> guard(lock);
            queue.push(value);
        }

        std::optional<T> pop() {
            std::lock_guard<SpinLock> guard(lock);
            if (queue.empty())
                return std::nullopt;
            std::optional<T> t = queue.front();
            queue.pop();
            return t;
        }

        std::size_t size() {
            std::lock_guard<SpinLock> guard(lock);
            return queue.size();
        }

        void clear() {
            std::lock_guard<SpinLock> guard(lock);
            queue = {};
        }
    };
}