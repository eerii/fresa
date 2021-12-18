//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <cmath>

namespace Fresa
{
    //---Vec2--- (2D Vector)
    template<typename T = int, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    struct Vec2 {
        T x, y;

        Vec2():x(0),y(0){};
        Vec2(T p_x, T p_y):x(p_x),y(p_y){};
        
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
      
        bool operator ==(const Vec2<T>& p_v) const { return x == p_v.x && y == p_v.y; };
        bool operator !=(const Vec2<T>& p_v) const { return x != p_v.x || y != p_v.y; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<T>& operator =(const Vec2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; return *this; };
        
        float length() const { return sqrtf(x*x + y*y); };
        float length2() const { return x*x + y*y; };
        Vec2<T> normal() const { if (x == 0 && y == 0) return Vec2<T>(0,0); return Vec2<T>(x / this->length(), y / this->length()); };
        Vec2<T> perpendicular() const { return Vec2<T>(-y, x); };
        float angle() const { return atan2(y, x); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Vec2<U> to() const { return Vec2<U>((U)x, (U)y); };
        
        template<typename U = int, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true,
                 typename V = int, std::enable_if_t<std::is_arithmetic_v<V>, bool> = true>
        static int dot(Vec2<U> p_v1, Vec2<V> p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; };

        static Vec2<T> from_angle(float p_rad, float p_len) { return Vec2<T>((T)(cos(p_rad) * p_len), (T)(sin(p_rad) * p_len)); };
        static Vec2<T> from_angle(float p_rad) { return from_angle<T>(p_rad, 1); };
        
        template<typename U = int, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true,
                 typename V = int, std::enable_if_t<std::is_arithmetic_v<V>, bool> = true>
        static Vec2<T> reflect(const Vec2<U>& p_v, const Vec2<V>& p_n) {
            return Vec2<T>(p_v.x - 2.0f * dot(p_v, p_n) * p_n.x, p_v.y - 2.0f * dot(p_v, p_n) * p_n.y); };
    };
}
