//* jobs
//      ...
#pragma once

#include "fresa_types.h"
#include "atomic_queue.h"
#include "fresa_time.h"
#include <atomic>
#include <thread>

//!
#include "log.h"

namespace fresa::jobs
{
    struct JobSystem;

    //* job specific coroutine elements
    struct JobPromiseBase;
    template <typename T> struct JobPromise;
    template <> struct JobPromise<void>;

    template <typename T>
    using JobFuture = coroutines::Future<JobPromise<T>, T>;

    //---

    //* final await
    //      called when the job is finished, makes it suspend one last time
    //      - control is returned to the parent coroutine
    template <typename P>
    struct FinalAwaitable : std_::suspend_always {
        //: constructor
        FinalAwaitable() noexcept { static_assert(coroutines::concepts::TPromise<P>, "P must be a promise"); };

        //: await_suspend
        void await_suspend(std_::coroutine_handle<P> h) noexcept {
            auto& promise = h.promise();
            if (promise.parent != nullptr) {
                ui32 n = promise.parent->children.fetch_sub(1);
                if (n == 1)
                    promise.parent->resume(); //- schedule parent coroutine using job system
            }
        }
    };

    //---

    //* job promise base
    struct JobPromiseBase : coroutines::PromiseBase {
        //: constructor
        JobPromiseBase(std_::coroutine_handle<> h) noexcept : PromiseBase(h) {};

        //: parent and children
        JobPromiseBase* parent = nullptr;
        std::atomic<ui32> children = 0;

        //: multithreading information
        ui32 thread_index;
    };

    //* job promise
    template<typename T>
    struct JobPromise : JobPromiseBase {
        //: constructor
        JobPromise() noexcept : JobPromiseBase(JobFuture<T>::handle_type::from_promise(*this)) {};

        //: promise value
        std::optional<T> value;

        //: return object
        JobFuture<T> get_return_object() noexcept;

        //: final suspend
        FinalAwaitable<JobPromise<T>> final_suspend() noexcept { return {}; };

        //: yield value
        FinalAwaitable<JobPromise<T>> yield_value(T v) noexcept { value = v; return {}; }

        //: return value
        void return_value(T v) noexcept { value = v; }
    };

    //* job promise (return void)
    template<>
    struct JobPromise<void> : JobPromiseBase {
        //: constructor
        JobPromise() noexcept : JobPromiseBase(JobFuture<void>::handle_type::from_promise(*this)) {};

        //: return object
        JobFuture<void> get_return_object() noexcept;

        //: final suspend
        FinalAwaitable<JobPromise<void>> final_suspend() noexcept { return {}; };

        //: yield value
        FinalAwaitable<JobPromise<void>> yield_value() noexcept { return {}; }

        //: return void
        void return_void() noexcept { }
    };

    //* get return objects
    template <typename T>
    inline JobFuture<T> JobPromise<T>::get_return_object() noexcept {
        return JobFuture<T>(JobFuture<T>::handle_type::from_promise(*this));
    }
    inline JobFuture<void> JobPromise<void>::get_return_object() noexcept {
        return JobFuture<void>(JobFuture<void>::handle_type::from_promise(*this));
    }

    //---

    //* concept checks
    static_assert(coroutines::concepts::TPromise<JobPromise<int>>, "JobPromise<int> is not a promise");
    static_assert(coroutines::concepts::TPromise<JobPromise<void>>, "JobPromise<void> is not a promise");
    static_assert(coroutines::concepts::TFuture<JobFuture<int>, JobPromise<int>>, "JobFuture<int> is not a future");
    
    //---

    //* job system
    struct JobSystem {
        //: global parameters
        static inline std::atomic<ui32> thread_count = 0;       // number of threads in the job system's pool
        static inline std::vector<std::jthread> thread_pool;    // thread pool
        static inline std::atomic<bool> running = false;        // flag to stop the job system

        //: thread local parameters
        static inline thread_local ui32 thread_index = 0;                       // thread index in the pool
        static inline thread_local JobPromiseBase* current_job = nullptr;       // current job

        //: queues
        static inline std::vector<AtomicQueue<JobPromiseBase*>> global_queues;  // global queues
        static inline std::vector<AtomicQueue<JobPromiseBase*>> local_queues;   // local queues

        //: initialize job system
        static void init() noexcept {
            //: check if the job system is already running
            if (running) return;
            running = true;

            //: get thread count
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0)
                thread_count = 1;

            //: create threads
            thread_pool.reserve(thread_count);
            for (ui32 i = 0; i < thread_count; i++)
                thread_pool.push_back(std::jthread(JobSystem::thread_run, i));
        }

        //: run function for each thread
        static void thread_run(ui32 index) noexcept {
            //: save thread local index
            thread_index = index;
            
            //: wait for all threads to be available
            static std::atomic<ui32> thread_counter = thread_count.load();
            thread_counter--;
            while (thread_counter.load() > 0) {}

            detail::log<"JOB SYSTEM", LOG_JOBS, fmt::color::light_green>("worker thread {} ready", thread_index);

            while (running) {
                std::this_thread::sleep_for(100ms);
            }
        }

        //: stop the job system
        static void stop() noexcept {
            running = false;
        }
    };

    //- job queue
}