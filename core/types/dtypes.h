//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>
#include <string>

namespace Verse
{
    //UNSIGNED INT
    typedef std::uint8_t ui8;
    typedef std::uint16_t ui16;
    typedef std::uint32_t ui32;
    typedef std::uint64_t ui64;

    //STRING
    typedef std::string str;

    //VEC2 - 2D Vector
    template<typename T = int, typename = typename std::enable_if<std::is_arithmetic_v<T>, T>::type>
    struct Vec2 {
        T x, y;

        
        Vec2():x(0),y(0){};
        Vec2(T p_x, T p_y):x(p_x),y(p_y){};
        
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator +(const Vec2<U> p_v) const { return Vec2<T>(x + p_v.x, y + p_v.y); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator -(const Vec2<U> p_v) const { return Vec2<T>(x - p_v.x, y - p_v.y); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator /(const U p_v) const { return Vec2<T>(x / p_v, y / p_v); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator /(const Vec2<U> p_v) const { return Vec2<T>(x / p_v.x, y / p_v.y); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator *(const U p_v) const { return Vec2<T>(x * p_v, y * p_v); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T> operator *(const Vec2<U> p_v) const { return Vec2<T>(x * p_v.x, y * p_v.y); };
        
        Vec2<T> operator -() const { return Vec2<T>(-x, -y); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator +=(const Vec2<U>& p_v) { x += p_v.x; y += p_v.y; return *this; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator -=(const Vec2<U>& p_v) { x -= p_v.x; y -= p_v.y; return *this; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator /=(U p_v) { x = x / p_v; y = y / p_v; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator /=(Vec2<U> p_v) { x = x / p_v.x; y = y / p_v.y; return *this; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator *=(U p_v) { x = x * p_v; y = y * p_v; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator *=(Vec2<U> p_v) { x = x * p_v.x; y = y * p_v.y; return *this; };
      
        bool operator ==(const Vec2<T>& p_v) const { return x == p_v.x && y == p_v.y; };
        bool operator !=(const Vec2<T>& p_v) const { return x != p_v.x || y != p_v.y; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<T>& operator =(const Vec2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; return *this; };
        
        
        float length() const { return sqrtf(x*x + y*y); };
        float length2() const { return x*x + y*y; };
        Vec2<T> normal() const { if (x == 0 && y == 0) return Vec2<T>(0,0); return Vec2<T>(x / this->length(), y / this->length()); };
        Vec2<T> perpendicular() const { return Vec2<T>(-y, x); };
        float angle() const { return atan2(y, x); };
        
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Vec2<U> to() const { return Vec2<U>((U)x, (U)y); };
        
        
        template<typename U = int, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type,
                 typename V = int, typename = typename std::enable_if<std::is_arithmetic_v<V>, V>::type>
        static int dot(Vec2<U> p_v1, Vec2<V> p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; };

        
        static Vec2<T> from_angle(float p_rad, float p_len) { return Vec2<T>((T)(cos(p_rad) * p_len), (T)(sin(p_rad) * p_len)); };
        static Vec2<T> from_angle(float p_rad) { return from_angle<T>(p_rad, 1); };
        
        
        template<typename U = int, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type,
                 typename V = int, typename = typename std::enable_if<std::is_arithmetic_v<V>, V>::type>
        static Vec2<T> reflect(const Vec2<U>& p_v, const Vec2<V>& p_n) {
            return Vec2<T>(p_v.x - 2.0f * dot(p_v, p_n) * p_n.x, p_v.y - 2.0f * dot(p_v, p_n) * p_n.y); };
    };


    //RECT2 - 4D Vector
    template<typename T = int, typename = typename std::enable_if<std::is_arithmetic_v<T>, T>::type>
    struct Rect2 {
        T x, y, w, h;
        
        
        Rect2():x(0),y(0),w(0),h(0){};
        Rect2(Vec2<T> p_pos, Vec2<T> p_size):x(p_pos.x),y(p_pos.y),w(p_size.x),h(p_size.y){};
        Rect2(T pos_x, T pos_y, T size_x, T size_y):x(pos_x),y(pos_y),w(size_x),h(size_y){};
        
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator +(const Rect2<U> p_v) const { return (Rect2<T>(x + p_v.x, y + p_v.y, w + p_v.w, h + p_v.h)); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator -(const Rect2<U> p_v) const { return (Rect2<T>(x - p_v.x, y - p_v.y, w - p_v.w, h - p_v.h)); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator /(const Rect2<U> p_v) const { return Rect2<T>(x / p_v.x, y / p_v.y, w / p_v.w, h / p_v.h); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator /(const Vec2<U> p_v) const { return Rect2<T>(x / p_v.x, y / p_v.x, w / p_v.y, h / p_v.y); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator /(const U p_v) const { return Rect2<T>(x / p_v, y / p_v, w / p_v, h / p_v); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator *(const Rect2<U> p_v) const { return Rect2<T>(x * p_v.x, y * p_v.y, w * p_v.w, h * p_v.h); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator *(const Vec2<U> p_v) const { return Rect2<T>(x * p_v.x, y * p_v.x, w * p_v.y, h * p_v.y); };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T> operator *(const U p_v) const { return Rect2<T>(x * p_v, y * p_v, w * p_v, h * p_v); };
        
        Rect2<T> operator -() const { return Rect2<T>(-x, -y, -w, -h); };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator +=(const Rect2<U> p_v) { x += p_v.x; y += p_v.y; w += p_v.w; h += p_v.h; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator +=(const Vec2<U> p_v) { x += p_v.x; y += p_v.y; return *this; }; //Notation shorthand
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator -=(const Rect2<U> p_v) { x -= p_v.x; y -= p_v.y; w -= p_v.w; h -= p_v.h; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator -=(const Vec2<U> p_v) { x -= p_v.x; y -= p_v.y; return *this; }; //Notation shorthand
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator /=(const Rect2<U> p_v) { x /= p_v.x; y /= p_v.y; w /= p_v.w; h /= p_v.h; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator /=(const Vec2<U> p_v) { x /= p_v.x; y /= p_v.x; w /= p_v.y; h /= p_v.y; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator /=(const U p_v) { x /= p_v; y /= p_v; w /= p_v; h /= p_v; return *this; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator *=(const Rect2<U> p_v) { x *= p_v.x; y *= p_v.y; w *= p_v.w; h *= p_v.h; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator *=(const Vec2<U> p_v) { x *= p_v.x; y *= p_v.x; w *= p_v.y; h *= p_v.y; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator *=(const U p_v) { x *= p_v; y *= p_v; w *= p_v; h *= p_v; return *this; };
        
        bool operator ==(const Rect2<T>& p_v) const { return x == p_v.x && y == p_v.y && w == p_v.w && h == p_v.h; };
        bool operator !=(const Rect2<T>& p_v) const { return x != p_v.x || y != p_v.y || w != p_v.w || h != p_v.h; };
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator =(const Rect2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; w = (T)p_v.w; h = (T)p_v.h; return *this; };
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<T>& operator =(const Vec2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; return *this; }; //Notation shorthand
        
        
        Vec2<T> pos() { return Vec2<T>(x, y); };
        Vec2<T> size() { return Vec2<T>(w, h); };
        
        
        template<typename U, typename = typename std::enable_if<std::is_arithmetic_v<U>, U>::type>
        Rect2<U> to() const { return Rect2<U>((U)x, (U)y, (U)w, (U)h); };
        
        
        SDL_Rect toSDL() {
            SDL_Rect rect;
            
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
            
            return rect;
        };
    };

    //SIGN
    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
    int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }

    //CONSTEXPR FOR
    template<std::size_t N>
    struct num { static const constexpr auto value = N; };

    template <class F, std::size_t... Is>
    void for_(F func, std::index_sequence<Is...>) {
        (func(num<Is>{}), ...);
    }

    template <std::size_t N, typename F>
    void for_(F func) {
        for_(func, std::make_index_sequence<N>());
    }
}
