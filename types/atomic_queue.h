//* atomic_queue
//      thread safe queue
#pragma once

#include <queue>
#include <atomic>
#include <optional>
#include <mutex>

namespace fresa
{
    //: simple implementation of a spin lock for thread syncronization, can be used inside std::lock_guard
    struct SpinLock {
        std::atomic_flag flag;
        void lock() noexcept { while( flag.test_and_set(std::memory_order_acquire) ); }
        void unlock() noexcept { flag.clear(std::memory_order_release); }
        bool try_lock() noexcept { return !flag.test_and_set(std::memory_order_acquire); }
    };

    //: adaptation of std::queue to be thread safe
    template <typename T>
    struct AtomicQueue {
        std::queue<T> queue;
        SpinLock lock;

        AtomicQueue() noexcept {}; //: constructor
        AtomicQueue(const AtomicQueue<T>&) = delete; //: no copy
        AtomicQueue& operator=(const AtomicQueue<T>&) = delete;
        AtomicQueue(AtomicQueue<T>&&) noexcept {}; //: move
        AtomicQueue& operator=(AtomicQueue<T>&&) {};

        //: add to the end of the queue
        void push(const T& value) {
            std::lock_guard<SpinLock> guard(lock);
            queue.push(value);
        }

        //: get an item from the front of the queue and remove it
        //  if the queue is empty, return an empty optional
        std::optional<T> pop() {
            std::lock_guard<SpinLock> guard(lock);
            if (queue.empty())
                return std::nullopt;
            std::optional<T> t = queue.front();
            queue.pop();
            return t;
        }

        //: returns the number of elements remaining in the queue
        std::size_t size() {
            std::lock_guard<SpinLock> guard(lock);
            return queue.size();
        }

        //: clear the queue
        void clear() {
            std::lock_guard<SpinLock> guard(lock);
            queue = {};
        }
    };
}