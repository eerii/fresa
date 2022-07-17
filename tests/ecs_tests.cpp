//* ecs_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "ecs.h"

namespace test
{
    using namespace fresa;

    inline TestSuite component_pool_tests("component_pool", []{
        ecs::ComponentPool<int> cpool;

        "add items"_test = [&]{
            for (int i = 0; i < 10; i++)
                cpool.add(ecs::id(i, 0), int{i});
            return expect(cpool.size() == 10);
        };

        "get item"_test = [&]{
            return expect(*cpool.get(ecs::id(5, 0)) == 5);
        };

        "remove item"_test = [&]{
            cpool.remove(ecs::id(5, 0));
            return expect(cpool.size() == 9 and cpool.get(ecs::id(5, 0)) == nullptr and
                          cpool.sparse.at(0).at(5) == ecs::invalid_id and cpool.sparse.at(0).at(9) == 5);
        };

        "iteration"_test = [&]{
            int i = 0;
            for (auto& item : cpool) i++;
            return expect(i == 9);
        };

        "update entity"_test = [&]{
            cpool.add(ecs::id(3, 1), 13);
            return expect(cpool.size() == 9 and *cpool.get(ecs::id(3, 1)) == 13);
        };
    });
}

#endif