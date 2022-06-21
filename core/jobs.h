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
        static inline std::atomic<bool> running = false;                                // flag to stop the job system
        static inline std::atomic<ui32> thread_count = 0;                               // number of threads in the job system's pool
        static inline std::vector<std::jthread> thread_pool;                            // thread pool
        static inline std::vector<std::unique_ptr<std::condition_variable>> thread_cv;  // thread condition variables
        static inline std::vector<std::unique_ptr<std::mutex>> thread_mutex;            // thread mutexes

        //: thread local parameters
        static inline thread_local ui32 thread_index = 0;                               // thread index in the pool
        static inline thread_local std::optional<JobPromiseBase*> current_job;          // current job

        //: queues
        static inline std::vector<AtomicQueue<JobPromiseBase*>> global_queues;          // global queues
        static inline std::vector<AtomicQueue<JobPromiseBase*>> local_queues;           // local queues

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
            for (ui32 i = 0; i < thread_count; i++) {
                thread_pool.push_back(std::jthread(JobSystem::thread_run, i));

                global_queues.push_back(AtomicQueue<JobPromiseBase*>());
                local_queues.push_back(AtomicQueue<JobPromiseBase*>());

                thread_cv.push_back(std::make_unique<std::condition_variable>());
                thread_mutex.push_back(std::make_unique<std::mutex>());
            }
        }

        //: schedule job
        static void schedule(JobPromiseBase* job) noexcept {
            if (job == nullptr) { log::error("invalid job to schedule"); return; }

            thread_local static ui32 t_id(rand() % thread_count); //- random utilities in fresa_math.h

            global_queues[t_id].push(job); //- global/local queues, also job can specify thread
            thread_cv[t_id]->notify_all();
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

            //: random index for job stealing
            ui32 steal_next = rand() % thread_count; //- random utilities in fresa_math.h

            //: number of empty loops before sleeping
            constexpr ui32 max_empty_loops = 256;
            thread_local static ui32 empty_loops = 0;

            //: mutex lock to use with condition variables to wake the thread if there is a job
            std::unique_lock<std::mutex> lock(*thread_mutex[thread_index]);

            while (running) {
                //: get job
                current_job = local_queues[thread_index].pop();
                if (not current_job.has_value())
                    current_job = global_queues[thread_index].pop();

                //: if there is no job, try to steal one from another thread
                ui32 n = thread_count - 1;
                while (not current_job.has_value() and --n > 0) {
                    steal_next = (steal_next + 1) % thread_count;
                    current_job = global_queues[steal_next].pop();
                }

                //: there is a job
                if (current_job.has_value()) {
                    if (current_job.value() == nullptr) {
                        log::error("The job you are trying to add is null, this should not happen");
                        continue;
                    }

                    //: run the job
                    log::info("Thread {} is running job {}", thread_index, current_job.value()->handle.address());
                    current_job.value()->resume();

                    //: reset empty loops
                    empty_loops = 0;
                }
                //: if there is no job still, sleep
                else if (empty_loops++ > max_empty_loops) {
                    thread_cv.at(thread_index)->wait_for(lock, 1ms); //! maybe this is too long of a wait
                    empty_loops = max_empty_loops / 2;
                }
            }
        }

        //: stop the job system
        static void stop() noexcept {
            running = false;
        }
    };

    //---

    //* schedule jobs
    template <typename T>
    void schedule(JobFuture<T>&& job) {
        auto& promise = job.handle.promise();
        JobSystem::schedule(&promise);

        //- parent and children
    }
}