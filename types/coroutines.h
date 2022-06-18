//* coroutines
//      implementation of c++20 coroutine support

#include "std_types.h"
#include <optional>

namespace fresa::coroutines
{
    //* coroutine promises
    struct PromiseBase;
    template<typename T> struct Promise;
    template<> struct Promise<void>;

    //* coroutine futures
    struct FutureBase;
    template<typename T> struct Future;
    template<> struct Future<void>;

    //* coroutine concept
    template <typename T>
    concept Coroutine =  std::is_base_of_v<FutureBase, T>;

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

    //* base promise type
    //      base class without templates for promise types
    //      implements:
    //          initial_suspend (always)
    //          unhandled_exception (default)
    //      also adds:
    //          handle, a coroutine handle referencing this promise (without a type)
    //          resume, uses the handle to continue execution of the coroutine
    struct PromiseBase {
        //: constructor
        PromiseBase(std_::coroutine_handle<> h) : handle(h) {};

        //: coroutine handle (untyped)
        std_::coroutine_handle<> handle;

        //: coroutine promise_type default implementation
        std_::suspend_always initial_suspend() noexcept { return {}; }
        void unhandled_exception() noexcept {}

        //: resume execution
        bool resume() noexcept {
            if (handle and not handle.done())
                handle.resume();
            return true;
        }
    };

    //* templated promise type
    //      inherits behaviour from the base promise type, and extends it adding typing
    //      this is the implementation for all types except void (using return_value)
    //      implements:
    //          get_return_object, returns a future object initialized with a handle of this promise type
    //          - final_suspend ...
    //          return_value, stores the value from co_return
    //          yield_value, stores the value from co_yield
    template<typename T>
    struct Promise : PromiseBase {
        //: constructor
        //      calls the base constructor with this handle, it is automatically saved there as typeless handle
        Promise() : PromiseBase(Future<T>::handle_type::from_promise(*this)) {};

        //: promise value
        std::optional<T> value;

        //: return object
        //      creates a future object and adds this handle (still typed) to it
        Future<T> get_return_object() noexcept { return Future<T>(Future<T>::handle_type::from_promise(*this)); }

        //- final suspend
        std_::suspend_always final_suspend() noexcept { return {}; }

        //: return value
        void return_value(T v) noexcept {
            value = v;
        }

        //: yield value
        std_::suspend_always yield_value(T v) noexcept {
            value = v;
            return {};
        }
    };

    //---

    //* future
    //      return type of coroutine functions that use co_await, co_return or co_yield, must have a promise_type defined
    //      we also add a coroutine handle so we can manage the state of the coroutine

    //* base future type
    //      base class without templates for future types
    //      has a pointer to the base promise type, which allows access to the untyped handle
    struct FutureBase {
        //: constructor
        FutureBase(PromiseBase* p) : promise(p) {};

        //: promise pointer
        PromiseBase* promise = nullptr;
    };

    //* templated future type
    //      inherits behaviour from the base future type, and extends it adding typing
    //      defines promise_type, required by the coroutine specification, as the relevant typed promise
    //      also stores a typed handled to the coroutine, which is created using promise::get_return_object
    template <typename T> 
    struct Future : FutureBase {
        //: type aliases
        using promise_type = Promise<T>;
        using handle_type = std_::coroutine_handle<promise_type>;

        //: constructor
        Future(handle_type h) : FutureBase(&h.promise()), handle{h} {}

        //: coroutine handle (typed)
        handle_type handle;

        //: ready - returns true if the promise value is set
        bool ready() noexcept { return handle.promise().value.has_value(); }
        //: get - returns the promise value
        T get() noexcept { return handle.promise().value.value(); }
    };
}