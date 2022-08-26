//* jobs
//      multithreaded job system based on coroutines
//      uses worker threads where jobs are submitted
#pragma once

#include "std_types.h"

#include "coroutines.h"
#include "atomic_queue.h"
#include "fresa_math.h"
#include "fresa_time.h"

#include "log.h"
#include "system.h"
#include "fresa_assert.h"

#include <atomic>
#include <thread>
#include <condition_variable>

namespace fresa::jobs
{
    //* job system
    struct JobSystem;

    //* job specific coroutine elements
    struct JobPromiseBase;
    template <typename T> struct JobPromise;
    template <> struct JobPromise<void>;
    template <typename T> struct JobFuture;

    //---

    //* final await
    //      called when the job is finished, makes it suspend one last time
    //      control is returned to the parent coroutine
    template <typename P>
    struct FinalAwaitable : std_::suspend_always {
        //: constructor
        FinalAwaitable() noexcept {
            static_assert(coroutines::concepts::Promise<P>, "P must be a promise");
        };

        //: await suspend, if there is a parent and this is the last children, resume it
        void await_suspend(std_::coroutine_handle<P> h) noexcept {
            auto& promise = h.promise();
            if (promise.parent != nullptr) {
                ui32 n = promise.parent->children.fetch_sub(1);
                if (n == 1)
                    promise.parent->resume();
            }
        }
    };

    //* coroutine await
    //      called when using 'co_await some_coroutine()'
    template <typename C, typename P>
    struct CoroutineAwaitable {
        //: constructor
        CoroutineAwaitable(C&& c) noexcept : children(std::forward<C&&>(c)) {
            static_assert(coroutines::concepts::Future<C, typename C::promise_type>, "C must be a future");
            static_assert(coroutines::concepts::Promise<P>, "P must be a promise");
        };

        //: coroutine called on co_await
        C&& children;

        //: await ready, always suspend
        bool await_ready() noexcept { return false; }

        //: await suspend, schedule children and suspend
        bool await_suspend(std_::coroutine_handle<P> h) noexcept {
            auto parent = &h.promise();
            parent->children.fetch_add(1);
            schedule(children, parent);
            return true;
        }

        //: await resume (called after the children resumes the parent), returns child value
        auto await_resume() noexcept {
            if constexpr (std::is_void_v<typename C::promise_type::value_type>) {
                return;
            } else {
                return children.get();
            }
        }
    };

    //---

    //* job promise base
    struct JobPromiseBase : coroutines::PromiseBase {
        //: constructor
        JobPromiseBase(std_::coroutine_handle<> h) noexcept : PromiseBase(h) {};

        //: parent
        JobPromiseBase* parent = nullptr;
        std::atomic<ui32> children = 0;

        //: multithreading information
        int thread_index = -1;
    };

    //* job promise
    template<typename T>
    struct JobPromise : JobPromiseBase {
        using value_type = T;

        //: constructor
        JobPromise() noexcept : JobPromiseBase(std_::coroutine_handle<JobPromise<T>>::from_promise(*this)) {};

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

        //: co_await
        template <typename C>
        CoroutineAwaitable<C, JobPromise<T>> await_transform(C&& c) noexcept { return {std::forward<C>(c)}; };
    };

    //* job promise (return void)
    template<>
    struct JobPromise<void> : JobPromiseBase {
        using value_type = void;

        //: constructor
        JobPromise() noexcept : JobPromiseBase(std_::coroutine_handle<JobPromise<void>>::from_promise(*this)) {};

        //: return object
        JobFuture<void> get_return_object() noexcept;

        //: final suspend
        FinalAwaitable<JobPromise<void>> final_suspend() noexcept { return {}; };

        //: yield value
        FinalAwaitable<JobPromise<void>> yield_value() noexcept { return {}; }

        //: return void
        void return_void() noexcept { }

        //: co_await
        template <typename C>
        CoroutineAwaitable<C, JobPromise<void>> await_transform(C&& c) noexcept { return {std::forward<C>(c)}; };
    };

    //* job future
    template <typename T>
    struct JobFuture : coroutines::Future<JobPromise<T>, T> {
        //: constructor
        JobFuture(std_::coroutine_handle<JobPromise<T>> h) noexcept : coroutines::Future<JobPromise<T>, T>(h) {};

        //: destructor
        ~JobFuture() noexcept {
            if (this->handle) {
                this->handle.destroy();
            }
        }

        //: ready - returns true if the promise value is set
        [[nodiscard]] bool ready() noexcept {
            static_assert(not std::is_void_v<T>, "ready() requires a coroutine with a return value, not void");
            return this->handle.promise().value.has_value();
        }
        
        //: get - returns the promise value
        [[nodiscard]] T get() noexcept {
            static_assert(not std::is_void_v<T>, "get() requires a coroutine with a return value, not void");
            return this->handle.promise().value.value();
        }

        //: done - checks if the coroutine finished execution
        bool done() noexcept {
            return this->handle.done();
        }
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
    static_assert(coroutines::concepts::Promise<JobPromise<int>>, "JobPromise<int> is not a promise");
    static_assert(coroutines::concepts::Promise<JobPromise<void>>, "JobPromise<void> is not a promise");

    static_assert(coroutines::concepts::Future<JobFuture<int>, JobPromise<int>>, "JobFuture<int> is not a future");
    static_assert(coroutines::concepts::Future<JobFuture<void>, JobPromise<void>>, "JobFuture<void> is not a future");

    static_assert(coroutines::concepts::Awaitable<FinalAwaitable<JobPromise<int>>, JobPromise<int>>, "FinalAwaitable is not an awaitable");
    static_assert(coroutines::concepts::Awaitable<CoroutineAwaitable<JobFuture<int>, JobPromise<int>>, JobPromise<int>>, "CoroutineAwaitable is not an awaitable");
    
    //---

    //* job system
    struct JobSystem {
        //: system registration
        inline static System<JobSystem, system::SYSTEM_PRIORITY_FIRST> system;

        //: global parameters
        static inline std::atomic<bool> running = false;                                // flag to stop the job system
        static inline std::atomic<bool> is_stopped = true;                              // set to true when all threads are stopped
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
            if (not is_stopped) return;
            running = true;
            is_stopped = false;

            //: get thread count
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0)
                thread_count = 1;

            //: create threads and queues
            for (ui32 i = 0; i < thread_count; i++) {
                thread_pool.push_back(std::jthread(JobSystem::thread_run, i));

                global_queues.emplace_back(AtomicQueue<JobPromiseBase*>());
                local_queues.emplace_back(AtomicQueue<JobPromiseBase*>());

                thread_cv.emplace_back(std::make_unique<std::condition_variable>());
                thread_mutex.emplace_back(std::make_unique<std::mutex>());
            }
        }

        //: schedule job
        static void schedule(JobPromiseBase* job) noexcept {
            if (job == nullptr) { log::error("invalid job to schedule"); return; }

            thread_local static ui32 t_id = random<ui32>(0, thread_count-1);

            //: no thread specified, schedule on a random thread's global queue
            if (job->thread_index < 0 or job->thread_index >= thread_count) {
                t_id = (++t_id) % thread_count;
                global_queues[t_id].push(job);
                thread_cv[t_id]->notify_all();
                return;
            }

            //: if a thread is specified, schedule on the local queue
            local_queues[job->thread_index].push(job);
            thread_cv[job->thread_index]->notify_all();
        }

        //: run function for each thread
        static void thread_run(ui32 index) noexcept {
            //: save thread local index
            thread_index = index;
            
            //: counter for the number of threads initialized
            //      it is written like this to allow for system recreation (stop and init again)
            static std::atomic<ui32> thread_counter = 0;
            if (thread_counter == 0) thread_counter = thread_count.load();

            //: wait for all threads to be available
            thread_counter--;
            while (thread_counter.load() > 0) {}
            detail::log<"JOB SYSTEM", LOG_JOBS, fmt::color::light_green>("worker thread {} ready", thread_index);

            //: random index for job stealing
            ui32 steal_next = random<ui32>(0, thread_count - 1);

            //: number of empty loops before sleeping
            constexpr ui32 max_empty_loops = 256;
            thread_local static ui32 empty_loops = 0;

            //: mutex lock to use with condition variables to wake the thread if there is a job
            std::unique_lock<std::mutex> lock(*thread_mutex[thread_index]);

            while (running) {
                //: get job
                current_job = local_queues[thread_index].pop();
                if (not current_job)
                    current_job = global_queues[thread_index].pop();

                //: if there is no job, try to steal one from another thread
                ui32 n = thread_count - 1;
                while (not current_job and --n > 0) {
                    steal_next = (steal_next + 1) % thread_count;
                    current_job = global_queues[steal_next].pop();
                }

                //: there is a job
                if (current_job) {
                    fresa_assert(current_job.value() != nullptr, "the job you are trying to add is null, this should not happen");

                    //: run the job
                    detail::log<"JOB RUNNING", LOG_JOBS, fmt::color::gold>("thread {} is running job {}", thread_index, current_job.value()->handle.address());
                    current_job.value()->resume();

                    //: reset empty loops
                    empty_loops = 0;
                }
                //: if there is no job still, sleep
                else if (empty_loops++ > max_empty_loops) {
                    thread_cv.at(thread_index)->wait_for(lock, 1ms); //? maybe this is too long of a wait
                    empty_loops = max_empty_loops / 2;
                }
            }

            //: clean queues
            global_queues[thread_index].clear();
            local_queues[thread_index].clear();

            //: check if it is the last thread alive
            uint32_t threads_left = thread_count.fetch_sub(1);
            if (threads_left == 1)
                is_stopped = true;
        }

        //: stop the job system
        static void stop() noexcept {
            running = false;
            while (not is_stopped)
                std::this_thread::sleep_for(0.1ms);

            //: clear lists
            thread_pool.clear();
            global_queues.clear();
            local_queues.clear();
            thread_cv.clear();
            thread_mutex.clear();
        }
    };

    //---

    //* schedule jobs
    template <typename T>
    void schedule(const JobFuture<T>& job, JobPromiseBase* parent = nullptr, int thread_index = -1) noexcept {
        auto& promise = job.handle.promise();
        promise.thread_index = thread_index;
        JobSystem::schedule(&promise);
        promise.parent = parent;
    }

    //* wait for a job to complete
    template <typename T> requires (not std::is_void<T>())
    void waitFor(JobFuture<T>& job) noexcept {
        while (not job.ready());
    }
}