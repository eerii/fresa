//* ecs_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "ecs.h"

namespace test
{
    using namespace fresa;

    inline TestSuite component_pool_tests("ecs_component_pool", []{
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

    inline TestSuite scene_tests("ecs_scene", []{
        ecs::Scene scene;

        "create pool"_test = [&]{
            scene.cpool<int>();
            return expect(scene.component_pools.size() == 1);
        };

        "get pool"_test = [&]{
            auto& pool = scene.cpool<int>();
            return expect(scene.component_pools.size() == 1);
        };

        "add entity"_test = [&]{
            auto e1 = scene.add();
            auto e2 = scene.add();
            return expect(e1 == ecs::id(0, 0) and e2 == ecs::id(1, 0) and
                          scene.free_entities.size() == 1 and scene.free_entities.front() == ecs::id(2, 0));
        };

        "component types"_test = [&]{
            auto e = scene.add(int{1}, float{2.0f});
            return expect(scene.component_pools.size() == 2 and
                          scene.component_pools.contains(type_hash<int>()) and
                          scene.component_pools.contains(type_hash<float>()));
        };

        "get component"_test = [&]{
            auto e = scene.add(int{5}, float{1.68f});
            return expect(*scene.get<int>(e) == 5 and *scene.get<float>(e) == 1.68f);
        };

        "remove entity"_test = [&]{
            auto e = scene.add(int{16});
            scene.remove(e);
            return expect(scene.free_entities.size() == 2 and scene.free_entities.front() == e and scene.get<int>(e) == nullptr);
        };
    });

    inline TestSuite scene_view_tests("ecs_scene_view", []{
        ecs::Scene scene;
        scene.add(int{1});
        scene.add(int{3});
        scene.add(int{5});

        "scene view"_test = [&]{
            ecs::View<int> view(scene);
            for (auto a : view) {
                log::info("{}", a);
            };
            return expect(true);
         };
    });
}

#endif