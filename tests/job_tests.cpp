//* job_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "jobs.h"
#include "system.h"

namespace test
{
    using namespace fresa;

    namespace detail
    {
        jobs::JobFuture<int> job_returns_number() {
            co_return 64;
        }

        std::atomic<bool> did_it_run = false;
        jobs::JobFuture<void> job_void() {
            did_it_run = true;
            co_return;
        }

        jobs::JobFuture<int> job_with_parameters(int a, int b) {
            co_return a + b;
        }

        jobs::JobFuture<int> job_child_a() {
            co_return 1;
        }
        jobs::JobFuture<int> job_child_b() {
            co_return 2;
        }
        jobs::JobFuture<int> job_parent() {
            int a = co_await job_child_a();
            int b = co_await job_child_b();
            co_return a + b;
        }

        jobs::JobFuture<int> job_counter() {
            int n = 0;
            while (true)
                co_yield ++n;
        }
        jobs::JobFuture<int> job_count_to(int n) {
            auto j = job_counter();
            for (int i = 0; i < n; i++) {
                j.handle.promise().resume();
                jobs::waitFor(j);
            }
            co_return j.get();
        }
    }

    inline TestSuite job_test("jobs", []{
        system::add(jobs::JobSystem());

        "job that returns a number"_test = [] {
            auto j = detail::job_returns_number();
            jobs::schedule(j);
            jobs::waitFor(j);
            return expect(j.ready() and j.get() == 64);
        };

        "job without a return"_test = [] {
            auto j = detail::job_void();
            jobs::schedule(j);
            while (not j.done());
            return expect(detail::did_it_run == true);
        };

        "job with parameters"_test = [] {
            auto j = detail::job_with_parameters(2, 4);
            jobs::schedule(j);
            jobs::waitFor(j);
            return expect(j.ready() and j.get() == 6);
        };

        "job with children"_test = [] {
            auto j = detail::job_parent();
            jobs::schedule(j);
            jobs::waitFor(j);
            return expect(j.ready() and j.get() == 3);
        };

        "job with yield"_test = [] {
            auto j = detail::job_count_to(5);
            jobs::schedule(j);
            jobs::waitFor(j);
            return expect(j.ready() and j.get() == 5);
        };

        "job on a specific thread"_test = [] {
            auto j = detail::job_returns_number();
            jobs::schedule(j, nullptr, 3);
            jobs::waitFor(j);
            return expect(j.ready() and j.get() == 64);
        };

        system::manager.stop.top().f();
        system::manager.stop.pop();
        log::debug("stopping system 'JobSystem'");
    });
}

#endif