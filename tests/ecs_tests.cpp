//* ecs_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "ecs.h"

#include "_debug_cpool.h" //! ONLY FOR TESTING

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

        "update entity"_test = [&]{
            cpool.add(ecs::id(3, 1), 13);
            return expect(cpool.size() == 9 and *cpool.get(ecs::id(3, 1)) == 13);
        };
    });

    inline TestSuite scene_tests("ecs_scene", []{
        {
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
                return expect(scene.free_entities.size() == 2 and
                              ecs::index(scene.free_entities.front()) == ecs::index(e) and
                              ecs::version(scene.free_entities.front()) == ecs::version(e) + ecs::Version(1) and
                              scene.get<int>(e) == nullptr);
            };
        }
        {
            ecs::Scene scene;
            std::vector<ecs::EntityID> entities;
            for (int i = 0; i < 10; i++)
                entities.push_back(scene.add(int{1 << i}));

            "remove a few entities"_test = [&]{
                scene.remove(entities[5]);
                scene.remove(entities[2]);

                bool verify_sparse = [&]{
                    auto& s = scene.cpool<int>().sparse.at(0);
                    return s.at(0) == ecs::id(0, 0) and s.at(1) == ecs::id(1, 0) and s.at(2) == ecs::invalid_id and
                           s.at(3) == ecs::id(3, 0) and s.at(4) == ecs::id(4, 0) and s.at(5) == ecs::invalid_id and
                           s.at(6) == ecs::id(6, 0) and s.at(7) == ecs::id(7, 0) and
                           s.at(8) == ecs::id(2, 0) and s.at(9) == ecs::id(5, 0);
                }();

                bool verify_dense = [&]{
                    auto& d = scene.cpool<int>().dense;
                    return d.at(0) == ecs::Index(0) and d.at(1) == ecs::Index(1) and d.at(2) == ecs::Index(8) and
                           d.at(3) == ecs::Index(3) and d.at(4) == ecs::Index(4) and d.at(5) == ecs::Index(9) and
                           d.at(6) == ecs::Index(6) and d.at(7) == ecs::Index(7);
                }();

                bool verify_data = [&]{
                    auto& d = scene.cpool<int>().data;
                    return d.at(0) == 1 and d.at(1) == 2 and d.at(2) == 256 and d.at(3) == 8 and
                           d.at(4) == 16 and d.at(5) == 512 and d.at(6) == 64 and d.at(7) == 128;
                }();

                return expect(scene.free_entities == std::deque<ecs::EntityID>({ecs::id(2, 1), ecs::id(5, 1), ecs::id(10, 0)}) and
                              verify_sparse and verify_dense and verify_data);
            };

            "add entity back"_test = [&]{
                scene.add(int{1024});

                bool verify_sparse = [&]{
                    auto& s = scene.cpool<int>().sparse.at(0);
                    return s.at(0) == ecs::id(0, 0) and s.at(1) == ecs::id(1, 0) and s.at(2) == ecs::id(8, 1) and
                           s.at(3) == ecs::id(3, 0) and s.at(4) == ecs::id(4, 0) and s.at(5) == ecs::invalid_id and
                           s.at(6) == ecs::id(6, 0) and s.at(7) == ecs::id(7, 0) and
                           s.at(8) == ecs::id(2, 0) and s.at(9) == ecs::id(5, 0);
                }();

                bool verify_dense = [&]{
                    auto& d = scene.cpool<int>().dense;
                    return d.at(0) == ecs::Index(0) and d.at(1) == ecs::Index(1) and d.at(2) == ecs::Index(8) and
                           d.at(3) == ecs::Index(3) and d.at(4) == ecs::Index(4) and d.at(5) == ecs::Index(9) and
                           d.at(6) == ecs::Index(6) and d.at(7) == ecs::Index(7) and d.at(8) == ecs::Index(2);
                }();

                bool verify_data = [&]{
                    auto& d = scene.cpool<int>().data;
                    return d.at(0) == 1 and d.at(1) == 2 and d.at(2) == 256 and d.at(3) == 8 and
                           d.at(4) == 16 and d.at(5) == 512 and d.at(6) == 64 and d.at(7) == 128 and d.at(8) == 1024;
                }();

                return expect(scene.free_entities == std::deque<ecs::EntityID>({ecs::id(5, 1), ecs::id(10, 0)}) and
                              verify_sparse and verify_dense and verify_data);
            };
        }
    });

    inline TestSuite scene_view_tests("ecs_scene_view", []{
        ecs::Scene scene;
        scene.add(int{1});
        scene.add(int{3});
        scene.add(int{5}, float{3.14f});
        scene.add(float{0.50f});
        scene.add(int{7}, float{1.68f});
        scene.add(int{9}, float{0.32f});

        "one component scene view"_test = [&]{
            auto view = ecs::View<int>(scene);
            bool test_result = true;
            int i = 0;
            for (auto [e, d] : view()) {
                test_result &= d == 2*i + 1 and e == std::vector<ecs::Index>({0, 1, 2, 4, 5}).at(i); i++;
            }
            return expect(test_result);
        };

        "data only view"_test = [&]{
            auto view = ecs::View<float>(scene);
            bool test_result = true;
            int i = 0;
            for (auto d : view.data()) {
                test_result &= d == std::vector<float>({3.14f, 0.50f, 1.68f, 0.32f}).at(i); i++;
            }
            return expect(test_result);
        };

        "multiple components"_test = [&]{
            auto view = ecs::View<int, float>(scene);
            bool test_result = true;
            int i = 0;
            for (auto [e, d_int, d_float] : view()) {
                test_result &= e == std::vector<ecs::Index>({2, 4, 5}).at(i) and
                               d_int == std::vector<int>({5, 7, 9}).at(i) and
                               d_float == std::vector<float>({3.14f, 1.68f, 0.32f}).at(i); i++;
            }
            return expect(test_result);
        };

        "modify values"_test = [&]{
            auto view = ecs::View<int>(scene);
            bool test_result = true;
            for (auto [e, d] : view()) { d = 10; /* can't modify entities by design, e = ecs::Index(0) does not compile */ }
            for (auto [e, d] : view()) { test_result &= d == 10; }
            test_result &= *scene.get<int>(0) == 10;
            return expect(test_result);
        };
    });
}

#endif