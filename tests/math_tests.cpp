//* types_tests
#ifdef ENABLE_TESTS

#include "unit_test.h"
#include "fresa_math.h"

namespace test
{
    using namespace fresa;

    //* vector
    inline TestSuite vector_tests("vectors", []{
        //: creation
        "2D vector"_test = []{
            Vec2<> a{1, 2};
            return expect(a.size().first == 2 and a.x == 1 and a.y == 2);
        };
        "3D vector"_test = []{
            Vec3<> a{1, 2, 3};
            return expect(a.size().first == 3 and a.x == 1 and a.y == 2 and a.z == 3);
        };
        //: comparison
        "equality"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{1, 2};
            return expect(a == b);
        };
        "inequality"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{1, 3};
            return expect(a != b);
        };
        //: arithmetic
        "scalar product"_test = []{
            Vec2<> a{1, 2};
            return expect(a * 2 == Vec2<>{2, 4} and 2 * a == Vec2<>{2, 4});
        };
        "scalar division"_test = []{
            Vec2<> a{4, 8};
            return expect(a / 2 == Vec2<>{2, 4});
        };
        "sum of vectors"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{3, 4};
            return expect(a + b == Vec2<>{4, 6});
        };
        "difference of vectors"_test = []{
            Vec2<> a{3, 4};
            Vec2<> b{1, 2};
            return expect(a - b == Vec2<>{2, 2});
        };
        "operations on float vectors"_test = []{
            Vec2<float> a{1.5f, 2.5f};
            Vec2<float> b{3.2f, 4.1f};
            return expect(a + b == Vec2<float>{4.7f, 6.6f});
        };
        //: assignment
        "assignment"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{3, 4};
            a = b;
            return expect(a == b);
        };
        "sum assignment"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{3, 4};
            a += b;
            return expect(a == Vec2<>{4, 6});
        };
        "difference assignment"_test = []{
            Vec2<> a{3, 4};
            Vec2<> b{1, 2};
            a -= b;
            return expect(a == Vec2<>{2, 2});
        };
        "scalar product assignment"_test = []{
            Vec2<> a{1, 2};
            a *= 2;
            return expect(a == Vec2<>{2, 4});
        };
        "scalar division assignment"_test = []{
            Vec2<> a{4, 8};
            a /= 2;
            return expect(a == Vec2<>{2, 4});
        };
        //: vector specific operations
        "dot product"_test = []{
            Vec2<> a{1, 2};
            Vec2<> b{3, 4};
            return expect(dot(a, b) == 11 and a * b == 11);
        };
        "cross product"_test = []{
            Vec3<> a{1, 2, 3};
            Vec3<> b{4, 5, 6};
            return expect(cross(a, b) == Vec3<>{-3, 6, -3});
        };
        "norm of a vector"_test = []{
            Vec2<> a{3, 4};
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
        //: transformations
        "to convertible type (same structure)"_test = []{
            Vec2<int> a{1, 2};
            return expect(to<float>(a) == Vec2<float>{1.0f, 2.0f});
        };
        "to convertible type (different structure)"_test = []{
            Mat<3, 1, int> a({1, 2, 3});
            Vec3<int> b{1, 2, 3};
            return expect(to<Vec3<int>>(a) == b);
        };
        "to row vector"_test = []{
            Vec3<int> a{1, 2, 3};
            auto b = to_row<RVec3<int>>(a);
            return expect(b.get<0, 0>() == 1 and b.get<0, 1>() == 2 and b.get<0, 2>() == 3);
        };
        "to column vector"_test = []{
            RVec3<int> a({1, 2, 3});
            auto b = to_column<Vec3<int>>(a);
            return expect(b.x == 1 and b.y == 2 and b.z == 3);
        };
        //: row by column vector
        "row vector by column vector"_test = []{
            RVec3<int> a({1, 2, 3});
            Vec3<int> b(1, 2, 3);
            return expect(dot(a, b) == 14 and a * b == 14);
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
        "matrix 2x2 multiplication"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat2<int> b({1, 0, 0, 1});
            return expect(dot(a, b) == Mat2<int>({1, 2, 3, 4}) and a * b == Mat2<int>({1, 2, 3, 4}));
        };
        "matrix 3x3 multiplication"_test = []{
            Mat3<int> a({1, 2, 3, 4, 5, 6, 7, 8, 9});
            Mat3<int> b({0, 0, 1, 1, 0, 0, 0, 1, 0});
            return expect(a * b == Mat3<int>({2, 3, 1, 5, 6, 4, 8, 9, 7}));
        };
        "matrix 4x4 multiplication"_test = []{
            Mat4<int> a({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
            Mat4<int> b({1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1});
            return expect(a * b == Mat4<int>({-2, 2, -2, 2, -2, 2, -2, 2, -2, 2, -2, 2, -2, 2, -2, 2}));
        };
        "matrix 2x3 by 3x2 multiplication"_test = []{
            Mat<2, 3, int> a({1, 2, 3, 4, 5, 6});
            Mat<3, 2, int> b({6, 5, 4, 3, 2, 1});
            return expect(a * b == Mat2<int>({20, 14, 56, 41}));
        };
        "transposed matrix"_test = []{
            Mat2<int> a({1, 2, 3, 4});
            Mat3<int> b({1, 2, 3, 4, 5, 6, 7, 8, 9});
            return expect(transpose(a) == Mat2<int>({1, 3, 2, 4}) and transpose(b) == Mat3<int>({1, 4, 7, 2, 5, 8, 3, 6, 9}));
        };
        "identity"_test = []{
            return expect(identity<int, 3>() == Mat3<int>({1, 0, 0, 0, 1, 0, 0, 0, 1}));
        };
        //: matrix-vector operations
        "matrix by column vector multiplication"_test = []{
            Mat2<int> a({0, 1, 1, 0});
            Vec2<int> b(1, 2);
            return expect(dot(a, b) == Vec2<int>({2, 1}) and a * b == Vec2<int>({2, 1}));
        };
        "matrix by row vector multiplication"_test = []{
            RVec2<int> a({1, 2});
            Mat2<int> b({0, 1, 1, 0});
            return expect(dot(a, b) == RVec2<int>({2, 1}) and a * b == RVec2<int>({2, 1}));
        };
        //: column by row vector
        "column vector by row vector"_test = []{
            Vec3<int> a(1, 2, 3);
            RVec3<int> b({1, 2, 3});
            return expect(a * b == Mat3<int>({1, 2, 3, 2, 4, 6, 3, 6, 9}));
        };
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

#endif