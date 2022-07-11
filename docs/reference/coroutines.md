# [`coroutines`](https://github.com/josekoalas/fresa/blob/main/types/coroutines.h)

Coroutines are a new feature in `c++20` that allow for functions to stop and resume execution later. It introduces three new keywords, `co_await`, `co_yield` and `co_return`. However, the implementation in the standard is missing the basic primitives which would allow for easier creation of coroutines, but these are suppossed to come later with `c++23`. The [`cppcoro`](https://github.com/lewissbaker/cppcoro) library is an excelent implementation of most of these features.

Since in **fresa** coroutines are used very sparsely, mainly to easily use the [job system](jobs.md), a custom very basic implementation is made. For a very good list of coroutine resources, see [this gist](https://gist.github.com/MattPD/9b55db49537a90545a90447392ad3aeb) by MattPD, which includes links to many talks and articles explaining the new features.

The **fresa** implementation derives from the talks on a coroutine job system by [Tanki Zhang](https://www.youtube.com/watch?v=KWi793v5uA8) ([github](https://github.com/tankiJong/cpp-coroutine-job-system)) and the [vienna job system](https://www.youtube.com/watch?v=XtFWxwDX7D4) by Helmut Hlavacs ([github](https://github.com/hlavacs/ViennaGameJobSystem)), as well as the magnificent blog posts of [Lewis Baker](https://lewissbaker.github.io/2017/09/25/coroutine-theory), [David Mazières](https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html) and [Rainer Grimm](https://www.modernescpp.com/index.php/c-20-coroutines-the-first-overview).

It defines three base types: promises, futures and awaitables.

## promises

A promise is the implementation of the `promise_type` that all futures require in `c++20`. It must specify how the coroutine behaves when it is initialized and when it is finalized, when a value is yielded from it, and it must return a **future** object. In **fresa** there are `PromiseBase`, which is an untemplated promise base type that contain general definitions. It suspends always when a coroutine is started, so it must explicitly be resumed. It also ignores any exception by design. Finally, it includes an unspecialized `std::coroutine_handle<> handle`, which is used to control the flow of the coroutine.

Then an specialization of promises is made separately for regular and void types in `Promise<T>`. They create the `get_return_object` to a `Future<P, T>`, and always suspend on finish and yielding. The `Promise<void>` has a `return_void` and doesn't require the use of `co_return`, while the `Promise<T>` has a `return_value` which can be stored in the promise.

## futures

All coroutine functions must return a `Future<P, T>` object, which encapsulates the state of the coroutine and can be used to access the underlying promise and handle. `FutureBase` only contains a pointer to the untyped promise type for type erasure purposes. Its child `Future<P, T>` defines the `promise_type` and is in turned passed to the underlying promise to be used by `get_return_object`. A simplification of notation is provided with `FuturePromise<T> = Future<Promise<T>, T>`.

## awaitables

These define how the coroutine suspends with `await_ready`, `await_suspend` and `await_resume`. The trivial awaitables defined by the standard are `std::suspend_always` and `std::suspend_never`. Custom awaitables can be created, and are in fact an essential part of the [job system](jobs.md).

Ok, this was a mess. This is probably the least clear article in the documentation section due to the nature of how coroutines are implemented in `c++`. This will need a rewrite with an extensive explanation of how they work. For now, if you want to dive deep into coroutines, take a look at the [links](https://gist.github.com/MattPD/9b55db49537a90545a90447392ad3aeb) I mentioned before, and lets see a practical example for now to clarify things.

## simple yield counter

```cpp
FuturePromise<int> counter_yield() {
    for (int i = 0 ;; i++)
        co_yield i;
}

//...

auto c = counter_yield();
c.handle.resume();
for (int i = 0; i < 3; i++)  {
    log::info(c.handle.promise().value.value());
    c.handle();
}
c.handle.destroy();

> 0
> 1
> 2
```

This example shows how to create an infinite counter using `co_yield` and our simple future type. You can see more examples in the [type unit test](https://github.com/josekoalas/fresa/blob/main/tests/type_tests.cpp) file.