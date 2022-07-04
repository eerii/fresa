# [`math`](https://github.com/josekoalas/fresa/blob/main/types/fresa_math.h)

## linear algebra

### numerical concepts

Collection of concepts for numerical types.

**`concepts::Numeric`** 

Defines a number, it is the union of `std::integral` and `std::floating_point`.

```cpp
concepts::Numeric<int> == true;
concepts::Numeric<float> == true;
concepts::Numeric<str> == false;
```

**`concepts::Matrix`** 

Defines a matrix-like type with a two dimensional size, a type, and a `get` member function with constexpr access. The specialization `concepts::SquareMatrix` is a matrix with the same size on both dimensions. An example of a minimal definition of a matrix is as follows:

```cpp
struct Matrix2x3 {
    //: value type
    M::value_type = int;
    //: size
    constexpr static std::pair<std::size_t, std::size_t> size() { return {2, 3}; }
    //: reference get
    template <std::size_t I, std::size_t J>
    constexpr T& get() { ... }
    //: constant get
    template <std::size_t I, std::size_t J>
    constexpr T get() const { ... }
};
concepts::Matrix<Matrix2x3> == true;
concepts::SquareMatrix<Matrix2x3> == false; // 2 != 3
```

**`concepts::ColumnVector`**

A column vector is just a matrix of size Nx1 with N > 1. It is the most usual type of vector and the one we specificly define later. There is also a row vector concept, `concepts::RowVector`, which is a matrix of size 1xN, but it is mainly there to assist with some advanced linear algebra operations that will be rarely used. The concept `concepts::Vector` is the union of both types of vector.

```cpp
struct Vector4f {
    //: value type
    M::value_type = float;
    //: size
    constexpr static std::pair<std::size_t, std::size_t> size() { return {4, 1}; }
    //: reference get
    template <std::size_t I, std::size_t J>
    constexpr T& get() { ... }
    //: constant get
    template <std::size_t I, std::size_t J>
    constexpr T get() const { ... }
};
concepts::Matrix<Vector4f> == true;
concepts::ColumnVector<Vector4f> == true;
concepts::RowVector<Vector4f> == false;
concepts::Vector<Vector4f> == true;
```

### functions

All operations defined in the algebra library are constant expressions, meaning that they can be resolved at compile time (for constexpr variables) and save performance on runtime. This is the main motivation to have a templated `get` member function instead of a regular lookup.

**scalar operations on vectors/matrices**

```cpp
M mat; // M satisfies concepts::Matrix
T scalar; // T satisfies concepts::Numeric and it is the same as M::value_type

//: scalar multiplication
M result = mat * scalar;
M result = scalar * mat;
mat *= scalar;

//: scalar division
M result = mat / scalar;
mat /= scalar;
```

**arithmetic operations on vectors/matrices**

```cpp
M a, b; // M satisfies concepts::Matrix

//: sum
M result = a + b;
a += b;

//: difference
M result = a - b;
a -= b;
```

**comparison operations on vectors/matrices**

```cpp
M a, b; // M satisfies concepts::Matrix

//: equality
bool result = a == b;

//: inequality
bool result = a != b;

//: less than (element wise less comparison, returns true if **all** elements are less)
bool result = a < b;
```

**vector specific operations**

_Note: technically opearations on vectors mostly require one of them to be a column vector and the other to be a row vector, but for simplicity **fresa** allows operations between two vectors of the same type (for example two column vectors) to be performed correctly. There are special overloads to handle special cases with column with row vector operations in the matrix section._

```cpp
// V satisfies concepts::Vector
// CV satisfies concepts::ColumnVector
// T satisfies concepts::Numeric and it is the same as V::value_type
// S satisfies concepts::Numeric but not necesarily the same type as V::value_type

//: dot product
V a, b;
T result = dot(a, b) == a * b

//: cross product
CV a, b; // The vector size must be 3.
CV result = cross(a, b);

//: norm
V v;
S result = norm(v); 

//: normalized vector (unit length)
V v;
V result = normalize(v);

//: angle between vectors (in radians)
V a, b;
S result = angle(a, b);

//: angles with respect to the coordinate axis
CV v;
S result_x = angle_x(v);
S result_y = angle_y(v);
S result_z = angle_z(v);
```

**matrix product**

```cpp
//: general matrix product
//  A, B and M satisfy concepts::Matrix
//  the sizes must be compatible, meaning A=AIxAJ, B=BIxBJ, M=MIxMJ with AJ==BI, MI==AI, MJ==BJ
//  it is in general non conmutative, a*b != b*a
A a; B b;
M result = dot<M, A, B>(a, b);

//: square matrix product
//  notation can be simplified for square matrices since A = B = M -> SM (concepts::SquareMatrix)
SM a, b;
SM result = dot(a, b) == a * b;

//: vector-matrix products
//  similarly, notation can be simplified for vector multiplication
//  note that the size of the vector must equal the size of the square matrix (3x3 by 3x1 for example)
//  also keep in mind the order of multiplication since only the one defined below is defined

//  column vectors
CV c; SM m;
CV result = dot(m, c) == m * c;
//  row vectors
RV r; SM m;
RV result = dot(r, m) == r * m;
```

While row by column vector operations are already well defined by the regular dot product (since the result is a scalar), column by row vector operations produce a matrix and would require extensive syntax. Therefore a specialization exists only for [**fresa** defined](#vector-implementation) types (`Vec2/3`, `RVec2/3` and `Mat`) that allows a simple dot operator syntax.

```cpp
Vec3<T> c;
RVec3<T> r;

//: row by column vector, scalar (already well defined)
T result = dot(r, c) == r * c

//: column by row vector, matrix
Mat3<T> result = dot(c, v) == c * v; // equivalent to Mat3<T> result = dot<Mat3<t>>(c, v);
```

**other matrix operations**

```cpp
//: transpose
M m;
M result = transpose(m);

//: determinant and inverse implementations are defined on [aguacate/extras](https://github.com/josekoalas/aguacate/blob/main/extras/_linear_algebra.h). i am not 100% happy with the clarity and performance of current implementation so they are not imported directly in the project, but are fully functional and can be used.
```

### vector implementation

**fresa** provides predefined implementations for a series of types that satisfie `concepts::ColumnVector`.

**2D vector**

```cpp
Vec2<T> v;
concepts::ColumnVector<decltype(v)> == true;

//: size
v.size() == {2, 1};

//: vector components
v.x == v.template get<0, 0> and
v.y == v.template get<1, 0> ;
```

**3D vector**

```cpp
Vec3<T> v;
concepts::ColumnVector<decltype(v)> == true;

//: size
v.size() == {3, 1};

//: vector components
v.x == v.template get<0, 0> and
v.y == v.template get<1, 0> and
v.z == v.template get<2, 0> ;
```

### matrix implementation

There is also an implementation for a generic NxM matrix that satisfies `concepts::Matrix` using an `std::array` of size N*M. To retrieve an element using the specification defined `get<I, J>` function it uses the index of the array `data[I * M + J]`. Specializations exist for commonly used matrix dimensions:

```cpp
Mat<N, M, T> m;
concepts::Matrix<decltype(m)> == true;

//: size
m.size() = {N, M};

//: 2x2 matrix
Mat2<T> == Mat<2, 2, T>;
//: 3x3 matrix
Mat3<T> == Mat<3, 3, T>;
//: 4x4 matrix
Mat4<T> == Mat<4, 4, T>;
```

There are also defined types for the less used row vectors, which for simplicity use the matrix impementation (and therefore dont have .x, .y, ... components available, but they can still be [convertible](#type-transformations) to column vectors).

```cpp
//: 2D row vector
RVec2<T> == Mat<1, 2, T>;
//: 3D row vector
RVec3<T> == Mat<1, 3, T>;
```

### type transformations

Matrix-like objects can be converted to other types of the same characteristics that satisfy the same concepts.

```cpp
//: convert to compatible type (same size)
//  A and B both satisfy concepts::Matrix, but are different types
A a; 
B b = to<B>(a);
```

One specialization for [**fresa** defined](#vector-implementation) types (`Mat`, `Vec2` and `Vec3`) is to be able to convert between value types without having to specity the entire name, for example:

```cpp
Vec2<int> a;
Vec2<float> b = to<float>(a);
//: the other possibility without specialization is Vec2<float> b = to<Vec2<float>>(a);
```

There are specializations for column and row vectors, allowing conversions between the two despite not technically having the same two dimensional size. For example a 3x1 column vector can be converted to a 1x3 row vector.

```cpp
//: CV satisfies concepts::ColumnVector and RV satisfies concepts::RowVector

//: to column vector
RV r;
CV c = to_column<CV>(r);

//: to row vector
CV c;
RV r = to_row<RV>(c);
```

### pre-defined matrices

**identity matrix**

An identity matrix constexpr can be generated for any type that satisfies `concepts::SquareMatrix`. There are also specializations for the [**fresa** defined](#matrix-implementation) type `Mat`, specifying only the type and dimension.

```cpp
//: general matrix identity
M i = identity<M>

//: fresa matrix identity
Mat3<int> i = identity<int, 3>();
```

## random number generator

**fresa** has a random number generator based on the `<random>` library. It uses a Mersenne Twister engine (by Matsumoto and Nishimura), either [`mt19937`](https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine) or [`mt19937_64`](https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine) depending on the bytes required.

```cpp
T fresa::random(T min, T max) // with T = concepts::Numeric
```

The `random` function returns a number between `min` and `max` (closed interval, so both included) using a uniform distribution, with support for both [integral](https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution) and [floating](https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution) point types. The default type for the template parameter `T` is `int`. Some examples:

```cpp
int random(0, 100) = 37;
float random<float>(0.f, 1.f) = 0.37f;
ui64 random<ui64>(0, 5) = 4;
```

## general math functions

### factorials

The function `factorial<N>()` provides a constexpr factorial for $N!$. Similarly, `binomial<N, K>()` returns the combinatorial number N over K. A helper function `pascal_triangle<N>()` returns an array of size N+1 with the elements of the Nth row of the pascal triangle (with 0 being the first row).

### constexpr powers

The function `std::pow` does not allow for constexpr powers, so `pow<N>(x)` has been added, providing a constant expression for integer positive powers of x.

## interpolation

The `interpolate` function allows for interpolation between `a` and `b` using `t` as a parameter. Without any more parameters it uses linear interpolation, but a function can be passed to be aplied to `t` before interpolation, such as smoothstep.

```cpp
auto c = interpolate(a, b, t);
auto c = interpolate(a, b, t, func) == interpolate(a, b, func(t));

interpolate(a, b, 0.0) == a;
interpolate(a, b, 1.0) == b;
```

### smoothstep

A [smoothstep](https://en.wikipedia.org/wiki/Smoothstep) function is a sigmoid-like interpolation function that transitions smoothly between 0 and 1, with inputs lower than 0 being always 0 and inputs greater than 1 being 1. There are different orders of the smoothstep function:

- $S_0$: Behaves like a clamping function.
- $S_1$: Classical smoothstep function, same as implemented in _glsl_. Its main term is the 3rd degree polinomial $-2x^3 + 3x^2$.
- $S_2$: Often called _smootherstep_. It uses a 5th degree polinomial, $6x^5 - 15x^4 + 10x^3$.

The function `smoothstep<N>(x)` returns the smoothstep interpolation of order N. If no order is specified, it defaults to N = 1.

```cpp
auto c = interpolate(a, b, t, smoothstep<N, T>);
```

### cosine interpolation

```cpp
auto c = interpolate(a, b, t, cos_interpolation<T>);
```

### cubic interpolation

```cpp
auto c = interpolate(a, b, t, pow<3, T>);
```