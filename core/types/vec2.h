//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include <cmath>

namespace Fresa
{
    /// 2D vector
    ///
    /// Contains two variables, `x` and `y`, representing the components of a 2D vector. They can be of any type, which can be indicated using the template parameter. If no template parameter is specified, it defaults to `int`.
    template<typename T = int, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    struct Vec2 {
        //: 2D vector components
        T x, y;

        //: Constructor
        Vec2():x(0),y(0){};
        Vec2(T p_x, T p_y):x(p_x),y(p_y){};
        
        //: Arithmetic operators
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator +(const Vec2<U> p_v) const { return Vec2<T>(x + p_v.x, y + p_v.y); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator -(const Vec2<U> p_v) const { return Vec2<T>(x - p_v.x, y - p_v.y); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator /(const U p_v) const { return Vec2<T>(x / p_v, y / p_v); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator /(const Vec2<U> p_v) const { return Vec2<T>(x / p_v.x, y / p_v.y); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator *(const U p_v) const { return Vec2<T>(x * p_v, y * p_v); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T> operator *(const Vec2<U> p_v) const { return Vec2<T>(x * p_v.x, y * p_v.y); };
        
        Vec2<T> operator -() const { return Vec2<T>(-x, -y); };
      
        //: Comparison operators
        bool operator ==(const Vec2<T>& p_v) const { return x == p_v.x && y == p_v.y; };
        bool operator !=(const Vec2<T>& p_v) const { return x != p_v.x || y != p_v.y; };
        
        //: Assignment operators
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator =(const Vec2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; return *this; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator +=(const Vec2<U>& p_v) { x += p_v.x; y += p_v.y; return *this; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator -=(const Vec2<U>& p_v) { x -= p_v.x; y -= p_v.y; return *this; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator /=(U p_v) { x = x / p_v; y = y / p_v; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator /=(Vec2<U> p_v) { x = x / p_v.x; y = y / p_v.y; return *this; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator *=(U p_v) { x = x * p_v; y = y * p_v; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator *=(Vec2<U> p_v) { x = x * p_v.x; y = y * p_v.y; return *this; };
        
        /// Module of this vector (r)
        float length() const { return std::sqrt(x*x + y*y); };
        /// Module squared of this vector (r**2)
        float length2() const { return x*x + y*y; };
        /// Angle of this vector (theta)
        float angle() const { return std::atan2(y, x); };
        /// Normalized version of this vector (|v|)
        Vec2<T> normal() const { if (x == 0 && y == 0) return Vec2<T>(0,0); return Vec2<T>(x / this->length(), y / this->length()); };
        /// Perpendicular vector to this one
        Vec2<T> perpendicular() const { return Vec2<T>(-y, x); };
        
        /// Change the vector data type to U
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<U> to() const { return Vec2<U>((U)x, (U)y); };
        
        /// Dot product
        static T dot(Vec2<T> p_v1, Vec2<T> p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; };

        /// Create new vector from polar coordinates
        static Vec2<T> from_angle(float p_rad, float p_len = 1) { return Vec2<T>((T)(cos(p_rad) * p_len), (T)(sin(p_rad) * p_len)); };
        
        /// Reflect the vector p_v with respect to p_n
        template<typename U = int, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true,
                 typename V = int, std::enable_if_t<std::is_arithmetic_v<V>, bool> = true>
        static Vec2<T> reflect(const Vec2<U>& p_v, const Vec2<V>& p_n) {
            return Vec2<T>(p_v.x - 2.0f * dot(p_v, p_n) * p_n.x, p_v.y - 2.0f * dot(p_v, p_n) * p_n.y); };
    };
}
