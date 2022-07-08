//* coroutines
//      implementation of c++20 coroutine support
#pragma once

#if __has_include(<coroutine>)
    #include <coroutine>
    namespace std_ = std;
#elif __has_include(<experimental/coroutine>)
    #include <experimental/coroutine>
    namespace std_ = std::experimental;
#else
    #error "coroutines not supported"
#endif

#include <optional>

namespace fresa::coroutines
{
    namespace concepts
    {
        //* awaitable concept
        template <typename T>
        concept AwaitSuspend = std::same_as<T, bool> or std::same_as<T, void> or std::same_as<T, std_::coroutine_handle<>>;
        template <typename A, typename P>
        concept Awaitable = requires(A a) {
            { a.await_ready() } -> std::same_as<bool>;
            { a.await_suspend(std::declval<std_::coroutine_handle<P>>()) } -> AwaitSuspend;
            a.await_resume();
        };

        //* future concept
        template <typename F, typename P>
        concept Future = requires(F f) {
            typename F::promise_type;
            std::same_as<typename F::promise_type, P>;
            std::same_as<decltype(f.handle), std_::coroutine_handle<P>>;
        };

        //* promise concept
        template <typename P>
        concept Promise = requires(P p) {
            { p.initial_suspend() } -> Awaitable<P>;
            { p.final_suspend() } -> Awaitable<P>;
            { p.unhandled_exception() } -> std::same_as<void>;
            { p.get_return_object() } -> Future<P>;
        };
    }
    
    //* coroutine promises
    struct PromiseBase;
    template <typename T> struct Promise;
    template <> struct Promise<void>;

    //* coroutine futures
    struct FutureBase;
    template <typename P, typename T> struct Future;

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
        PromiseBase(std_::coroutine_handle<> h) noexcept : handle(h) {};

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
    //          get_return_object, returns a future object initialized with a handle of this promise type (defined later)
    //          final_suspend, resumes parent coroutine if it exists
    //          yield_value, stores the value from co_yield
    //          return_void
    template<typename T>
    struct Promise : PromiseBase {
        //: constructor
        //      calls the base constructor with this handle, it is automatically saved there as typeless handle
        Promise() noexcept : PromiseBase(std_::coroutine_handle<Promise<T>>::from_promise(*this)) {};

        //: promise value
        std::optional<T> value;

        //: return object
        //      creates a future object and adds this handle (still typed) to it
        Future<Promise<T>, T> get_return_object() noexcept;

        //: final suspend
        std_::suspend_always final_suspend() noexcept { return {}; };

        //: yield value
        std_::suspend_always yield_value(T v) noexcept { value = v; return {}; }

        //: return value
        void return_value(T v) noexcept { value = v; }
    };

    //* void promise type
    template<>
    struct Promise<void> : PromiseBase {
        //: constructor
        //      calls the base constructor with this handle, it is automatically saved there as typeless handle
        Promise() noexcept : PromiseBase(std_::coroutine_handle<Promise<void>>::from_promise(*this)) {};

        //: return object
        //      creates a future object and adds this handle (still typed) to it
        Future<Promise<void>, void> get_return_object() noexcept;

        //: final suspend
        std_::suspend_always final_suspend() noexcept { return {}; };

        //: yield value
        std_::suspend_always yield_value() noexcept { return {}; }

        //: return void
        void return_void() noexcept { }
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
        FutureBase(PromiseBase* p) noexcept : promise(p) {};

        //: promise pointer
        PromiseBase* promise = nullptr;
    };

    //* templated future type
    //      inherits behaviour from the base future type, and extends it adding typing
    //      defines promise_type, required by the coroutine specification, as the relevant typed promise
    //      also stores a typed handled to the coroutine, which is created using promise::get_return_object
    template <typename P, typename T> 
    struct Future : FutureBase {
        //: type aliases
        using promise_type = P;
        using handle_type = std_::coroutine_handle<promise_type>;

        //: constructor
        Future(handle_type h) noexcept : FutureBase(&h.promise()), handle{h} {
            static_assert(concepts::Promise<P>, "P must be a promise");
        }

        //: coroutine handle (typed)
        handle_type handle;
    };

    //* alias for the default promise type
    template <typename T>
    using FuturePromise = Future<Promise<T>, T>;

    //* definition of get_return_object for void specification
    template <typename T>
    inline FuturePromise<T> Promise<T>::get_return_object() noexcept {
        return FuturePromise<T>(FuturePromise<T>::handle_type::from_promise(*this));
    }
    inline FuturePromise<void> Promise<void>::get_return_object() noexcept {
        return FuturePromise<void>(FuturePromise<void>::handle_type::from_promise(*this));
    }

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

    //---

    //* concept checks

    //: promise
    static_assert(concepts::Promise<Promise<int>>, "Promise<int> is not a promise");
    static_assert(concepts::Promise<Promise<void>>, "Promise<void> is not a promise");

    //: future
    static_assert(concepts::Future<Future<Promise<int>, int>, Promise<int>>, "Future<Promise<int>, int> is not a future");
    static_assert(concepts::Future<FuturePromise<int>, Promise<int>>, "FuturePromise<int> is not a future");
}