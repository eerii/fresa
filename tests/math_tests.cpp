//* types_tests

#include "unit_test.h"
#include "fresa_math.h"

namespace test
{
    using namespace fresa;

    //* vector
    inline TestSuite vector_tests("vectors", []{
        //: creation
        "2D vector"_test = []{
            Vec2<int> a{1, 2};
            return expect(a.size().first == 2 and a.x == 1 and a.y == 2);
        };
        "3D vector"_test = []{
            Vec3<int> a{1, 2, 3};
            return expect(a.size().first == 3 and a.x == 1 and a.y == 2 and a.z == 3);
        };
        //: comparison
        "equality"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{1, 2};
            return expect(a == b);
        };
        "inequality"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{1, 3};
            return expect(a != b);
        };
        //: arithmetic
        "scalar product"_test = []{
            Vec2<int> a{1, 2};
            return expect(a * 2 == Vec2<int>{2, 4} and 2 * a == Vec2<int>{2, 4});
        };
        "scalar division"_test = []{
            Vec2<int> a{4, 8};
            return expect(a / 2 == Vec2<int>{2, 4});
        };
        "sum of vectors"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{3, 4};
            return expect(a + b == Vec2<int>{4, 6});
        };
        "difference of vectors"_test = []{
            Vec2<int> a{3, 4};
            Vec2<int> b{1, 2};
            return expect(a - b == Vec2<int>{2, 2});
        };
        "operations on float vectors"_test = []{
            Vec2<float> a{1.5f, 2.5f};
            Vec2<float> b{3.2f, 4.1f};
            return expect(a + b == Vec2<float>{4.7f, 6.6f});
        };
        //: assignment
        "assignment"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{3, 4};
            a = b;
            return expect(a == b);
        };
        "sum assignment"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{3, 4};
            a += b;
            return expect(a == Vec2<int>{4, 6});
        };
        "difference assignment"_test = []{
            Vec2<int> a{3, 4};
            Vec2<int> b{1, 2};
            a -= b;
            return expect(a == Vec2<int>{2, 2});
        };
        "scalar product assignment"_test = []{
            Vec2<int> a{1, 2};
            a *= 2;
            return expect(a == Vec2<int>{2, 4});
        };
        "scalar division assignment"_test = []{
            Vec2<int> a{4, 8};
            a /= 2;
            return expect(a == Vec2<int>{2, 4});
        };
        //: vector specific operations
        "dot product"_test = []{
            Vec2<int> a{1, 2};
            Vec2<int> b{3, 4};
            return expect(dot(a, b) == 11 and a * b == 11);
        };
        "cross product"_test = []{
            Vec3<int> a{1, 2, 3};
            Vec3<int> b{4, 5, 6};
            return expect(cross(a, b) == Vec3<int>{-3, 6, -3});
        };
        "norm of a vector"_test = []{
            Vec2<int> a{3, 4};
            return expect(norm(a) == 5);
        };
        "normalized unit vector"_test = []{
            Vec2<float> a{3.0f, 4.0f};
            return expect(normalize(a) == Vec2<float>{3.0f, 4.0f} / 5.0f);
        };
        "angle between vectors"_test = []{
            Vec2<float> a{1.0f, 0.0f};
            Vec2<float> b{0.0f, 1.0f};
            return expect(angle(a, b) == pi / 2.0f);
        };
        "angle with respect to x-axis"_test = []{
            Vec2<float> a{1.0f, 0.0f};
            Vec2<float> b{0.0f, 1.0f};
            return expect(angle_x(a) == 0.0f and angle_x(b) == pi / 2.0f);
        };
    });

    //* matrices
    inline TestSuite matrices("matrices", []{
        //: creation
        "2x2 matrix"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            return expect(a.size().first == 2 and a.size().second == 2 and a.get<0,0>() == 1 and a.get<0,1>() == 2 and a.get<1,0>() == 3 and a.get<1,1>() == 4);
        };
        "3x3 matrix"_test = []{
            Mat3<int> a({1, 2, 3, 4, 5, 6, 7, 8, 9});
            return expect(a.size().first == 3 and a.size().second == 3 and a.get<0,0>() == 1);
        };
        "4x4 matrix"_test = []{
            Mat4<int> a({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
            return expect(a.size().first == 4 and a.size().second == 4 and a.get<0,0>() == 1);
        };
        //: comparison
        "equality"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({1, 2, 3, 4});
            return expect(a == b);
        };
        "inequality"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({1, 5, 3, 7});
            return expect(a != b);
        };
        //: arithmetic
        "scalar product"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            return expect(a * 2 == Mat2<int>({2, 4, 6, 8}) and 2 * a == Mat2<int>({2, 4, 6, 8}));
        };
        "scalar division"_test = []{
            Mat2<int> a({4, 8, 16, 32});
            return expect(a / 2 == Mat2<int>({2, 4, 8, 16}));
        };
        "sum of matrices"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({5, 6, 7, 8});
            return expect(a + b == Mat2<int>({6, 8, 10, 12}));
        };
        "difference of matrices"_test = []{
            Mat2<int> a({3, 4, 5, 6});
            Mat2<int> b({1, 2, 3, 4});
            return expect(a - b == Mat2<int>({2, 2, 2, 2}));
        };
        "operations on float matrices"_test = []{
            Mat2<float> a({1.1f, 2.2f, 3.3f, 4.4f});
            Mat2<float> b({4.4f, 3.3f, 2.2f, 1.1f});
            return expect(a + b == Mat2<float>({5.5f, 5.5f, 5.5f, 5.5f}));
        };
        //: assignment
        "assignment"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({5, 6, 7, 8});
            a = b;
            return expect(a == b);
        };
        "sum assignment"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({5, 6, 7, 8});
            a += b;
            return expect(a == Mat2<int>({6, 8, 10, 12}));
        };
        "difference assignment"_test = []{
            Mat2<int> a({3, 4, 5, 6});
            Mat2<int> b({1, 2, 3, 4});
            a -= b;
            return expect(a == Mat2<int>({2, 2, 2, 2}));
        };
        "scalar product assignment"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            a *= 2;
            return expect(a == Mat2<int>({2, 4, 6, 8}));
        };
        "scalar division assignment"_test = []{
            Mat2<int> a({4, 8, 16, 32});
            a /= 2;
            return expect(a == Mat2<int>({2, 4, 8, 16}));
        };
        //: matrix specific operations

        //: matrix-vector operations
    });

    //* random
    inline TestSuite random_test("random", []{
        "random int"_test = []{
            int r = random(0, 100);
            return expect(r >= 0 and r <= 100);
        };
        "random float"_test = []{
            float r = random<float>(0.0f, 1.0f);
            return expect(r >= 0.0f and r <= 1.0f);
        };
        "random ui64"_test = []{
            ui64 r = random<ui64>(0, ui64(1) << 48);
            return expect(r >= 0 and r <= ui64(1) << 48);
        };
    });
}