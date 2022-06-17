//* coroutines
//      ...

#include "std_types.h"

namespace fresa::coroutines
{
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
    template <typename P> 
    struct AwaitablePromise {
        P* p;
        //: await_ready (false), always suspends
        bool await_ready() { return false; }
        //: await_suspend (false), saves the coroutine handle promise and then the coroutine is resumed
        bool await_suspend(std_::coroutine_handle<P> h) { p = &h.promise(); return false; }
        //: await_resume (pointer), makes co_await return the promise pointer
        P* await_resume() { return p; }
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
}