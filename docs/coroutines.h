//! temporary archive of coroutine implementation

//* coroutines
//      implementation of c++20 coroutine support

#include "std_types.h"

namespace fresa::coroutines
{
    namespace concepts
    {
        //* awaitable concept
        template <typename T>
        concept TAwaitSuspend = std::same_as<T, bool> || std::same_as<T, void> || std::same_as<T, std_::coroutine_handle<>>;
        template <typename T, typename P>
        concept TAwaitResume = std::same_as<T, void> || std::same_as<T, P*>;
        template <typename A, typename P>
        concept TAwaitable = requires(A a) {
            { a.await_ready() } -> std::same_as<bool>;
            { a.await_suspend(std::declval<std_::coroutine_handle<P>>()) } -> TAwaitSuspend;
            { a.await_resume() } -> TAwaitResume<P>;
        };

        //* promise concept
        template <typename P, typename T>
        concept TPromise = requires(P p) {
            { p.initial_suspend() } -> TAwaitable<P>;
            { p.final_suspend() } -> TAwaitable<P>;
            { p.unhandled_exception() } -> std::same_as<void>;
            p.get_return_object();
        };

        //* future concept
        template <typename F, typename P>
        concept TFuture = requires(F f) {
            typename F::promise_type;
            std::same_as<typename F::promise_type, P>;
            std::same_as<decltype(f.handle), std_::coroutine_handle<P>>;
        };
    }

    //* future forward declaration
    template <template <typename> typename P, typename T>
    struct Future;

    //---

    //* awaitables
    //      control the flow of the coroutine execution
    //
    //: await_ready
    //      true: the coroutine is ready to be resumed
    //      false: the coroutine is suspended (goes to await_suspend)
    //: await_suspend
    //      void / true: the coroutine remains suspended, control goes to the caller
    //      false: the coroutine is resumed
    //      handle for another coroutine: that coroutine is resumed by calling handle.resume()
    //: await_resume
    //      called whether the coroutine is suspended or resumed
    //      it is the result of the co_await expression
    //
    //: trivial awaitables
    //      suspend_never
    //          await_ready true, never suspends
    //          await_suspend void, does nothing
    //          await_resume void, returns nothing
    //      suspend_always
    //          await_ready false, always suspends
    //          await_suspend void, does nothing
    //          await_resume void, returns nothing

    //* awaitable with promise
    template <template <typename> typename P, typename T> requires concepts::TPromise<P<T>, T>
    struct AwaitablePromise {
        P<T>* p;
        //: await_ready (false), always suspends
        bool await_ready() { return false; }
        //: await_suspend (false), saves the coroutine handle promise and then the coroutine is resumed
        bool await_suspend(std_::coroutine_handle<P<T>> h) { p = &h.promise(); return false; }
        //: await_resume (pointer), makes co_await return the promise pointer
        P<T>* await_resume() { return p; }
    };

    //---

    //* promises
    //      implementations of promise_type of c++20 coroutines
    //
    //: get_return_object
    //      returns a future object with a propper handle with the promise type
    //
    //: initial_suspend
    //      returns an awaitable that controls how the coroutine behaves when it is created
    //: final_suspend
    //      returns an awaitable that controls how the coroutine behaves before it is destroyed
    //: unhandled_exception
    //      called when an exception is thrown in the coroutine, blank by default
    //
    //: returns
    //      called when the coroutine returns using co_return or execution finishes (end of function)
    //      note: only one of return_void and return_value can be used in a promise
    //      return_void
    //          for when co_return evaluates to void or no co_return is used
    //      return_value
    //          for when co_return evaluates to a value
    //
    //: yield_value
    //      called when the coroutine yields using co_yield, the value is passed as a parameter

    //* promise with void return
    template <typename T>
    struct Promise {
        //: returns a new future with a coroutine handle referencing this promise
        Future<Promise, T> get_return_object() { return { .handle = std_::coroutine_handle<Promise<T>>::from_promise(*this)}; }

        //: inital_suspend, always suspends, must be resumed with handle.resume()
        std_::suspend_always initial_suspend() { return {}; }
        //: final_suspend, always suspends
        std_::suspend_always final_suspend() noexcept { return {}; }
        //: unhandled_exception, blank by default
        void unhandled_exception() {}

        //: return_void
        void return_void() {}

        //: yield_value, always suspends, saves the yielded value in the promise
        T value;
        std_::suspend_always yield_value(T v) {
            value = v;
            return {};
        }
    };

    //---

    //* future
    //      return type of coroutine functions that use co_await, co_return or co_yield
    //      must have a promise_type defined, in this case given by the promise type
    //      we add a coroutine handle so it is easier to manage the coroutine
    template <template <typename> typename P, typename T> 
    struct Future {
        using promise_type = P<T>;
        using handle_type = std_::coroutine_handle<promise_type>;
        handle_type handle;
    };

    //---

    //* assertions

    //: promises
    static_assert(concepts::TPromise<Promise<int>, int>, "Promise<int> is not a promise");

    //: futures
    static_assert(concepts::TFuture<Future<Promise, int>, Promise<int>>, "Future<Promise, int> is not a future");

    //: awaitables
    static_assert(concepts::TAwaitable<AwaitablePromise<Promise, int>, Promise<int>>, "AwaitablePromise<Promise, int> is not an awaitable");
    static_assert(concepts::TAwaitable<std_::suspend_always, void>, "std_::suspend_always is not an awaitable");
    static_assert(concepts::TAwaitable<std_::suspend_never, void>, "std_::suspend_never is not an awaitable");
}