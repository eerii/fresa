//* coroutines
//      ...

#include "std_types.h"

namespace fresa::coroutines
{
    namespace concepts
    {
        //* promise concept
        template <typename P>
        concept TPromise = requires(P p) {
            p.get_return_object();
        };
    }

    //* awaitables
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

    //* trivial awaitables
    //: suspend_never
    //      await_ready true, never suspends
    //      await_suspend void, does nothing
    //      await_resume void, returns nothing
    //: suspend_always
    //      await_ready false, always suspends
    //      await_suspend void, does nothing
    //      await_resume void, returns nothing

    //* awaitable with promise
    template <concepts::TPromise P>
    struct AwaitablePromise {
        P* p;
        //: await_ready (false), always suspends
        bool await_ready() { return false; }
        //: await_suspend (false), saves the coroutine handle promise and then the coroutine is resumed
        bool await_suspend(std_::coroutine_handle<P> h) { p = &h.promise(); return false; }
        //: await_resume (pointer), makes co_await return the promise pointer
        P* await_resume() { return p; }
    };

    //* future forward declaration
    template <typename T>
    struct Future;

    //* promise
    //      implementation of promise_type
    template <typename T>
    struct Promise {
        //: returns a new future with a coroutine handle referencing this promise
        Future<T> get_return_object() { return { .handle = Future<T>::handle_type::from_promise(*this)}; }
    };

    //* future
    template <typename T>
    struct Future {
        using promise_type = Promise<T>;
        using handle_type = std_::coroutine_handle<promise_type>;
        handle_type handle;
    };
}