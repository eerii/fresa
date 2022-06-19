//* jobs
//      ...

#include "fresa_types.h"

namespace fresa::jobs
{
    namespace coroutines
    {
        using namespace fresa::coroutines;

        template <typename T> struct JobPromise;
        template <> struct JobPromise<void>;

        template <typename T>
        using JobFuture = Future<JobPromise<T>, T>;

        template<typename T>
        struct JobPromise : PromiseBase {
            //: constructor
            JobPromise() noexcept : PromiseBase(JobFuture<T>::handle_type::from_promise(*this)) {};

            //: promise value
            std::optional<T> value;

            //: return object
            JobFuture<T> get_return_object() noexcept;

            //: final suspend
            std_::suspend_always final_suspend() noexcept { return {}; };

            //: yield value
            std_::suspend_always yield_value(T v) noexcept { value = v; return {}; }

            //: return value
            void return_value(T v) noexcept { value = v; }
        };

        template<>
        struct JobPromise<void> : PromiseBase {
            //: constructor
            JobPromise() noexcept : PromiseBase(JobFuture<void>::handle_type::from_promise(*this)) {};

            //: return object
            JobFuture<void> get_return_object() noexcept;

            //: final suspend
            std_::suspend_always final_suspend() noexcept { return {}; };

            //: yield value
            std_::suspend_always yield_value() noexcept { return {}; }

            //: return void
            void return_void() noexcept { }
        };

        template <typename T>
        inline JobFuture<T> JobPromise<T>::get_return_object() noexcept {
            return JobFuture<T>(JobFuture<T>::handle_type::from_promise(*this));
        }
        inline JobFuture<void> JobPromise<void>::get_return_object() noexcept {
            return JobFuture<void>(JobFuture<void>::handle_type::from_promise(*this));
        }

        static_assert(concepts::TPromise<JobPromise<int>, int>, "JobPromise<int> is not a promise");
        static_assert(concepts::TPromise<JobPromise<void>, void>, "JobPromise<void> is not a promise");
        static_assert(concepts::TFuture<JobFuture<int>, JobPromise<int>>, "JobFuture<int> is not a future");
    }

    //- job queue

    //- fiber pool

    //- worker threads

}