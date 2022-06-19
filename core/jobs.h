//* jobs
//      ...

#include "fresa_types.h"
#include <atomic>
#include <thread>

namespace fresa::jobs
{
    struct JobSystem;

    //* job coroutines implementation
    namespace coroutines
    {
        using namespace fresa::coroutines;

        //* job specific coroutine elements
        struct JobPromiseBase;
        template <typename T> struct JobPromise;
        template <> struct JobPromise<void>;

        template <typename T>
        using JobFuture = Future<JobPromise<T>, T>;

        //---

        //* final await
        //      called when the job is finished, makes it suspend one last time
        //      - control is returned to the parent coroutine
        template <typename P>
        struct FinalAwaitable : std_::suspend_always {
            //: constructor
            FinalAwaitable() noexcept { static_assert(concepts::TPromise<P>, "P must be a promise"); };

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
        struct JobPromiseBase : PromiseBase {
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
        static_assert(concepts::TPromise<JobPromise<int>>, "JobPromise<int> is not a promise");
        static_assert(concepts::TPromise<JobPromise<void>>, "JobPromise<void> is not a promise");
        static_assert(concepts::TFuture<JobFuture<int>, JobPromise<int>>, "JobFuture<int> is not a future");
    }

    struct JobSystem {
        std::atomic<ui32> thread_count; //: number of threads in the job system's pool
    };

    //- job queue

    //- fiber pool

    //- worker threads

}