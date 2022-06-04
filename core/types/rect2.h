//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include <SDL2/SDL.h>
#include "vec2.h"

namespace Fresa
{
    //---Rect2--- (4D Vector)
    template<typename T = int, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    struct Rect2 {
        T x, y, w, h;
        
        Rect2():x(0),y(0),w(0),h(0){};
        Rect2(Vec2<T> p_pos, Vec2<T> p_size):x(p_pos.x),y(p_pos.y),w(p_size.x),h(p_size.y){};
        Rect2(T pos_x, T pos_y, T size_x, T size_y):x(pos_x),y(pos_y),w(size_x),h(size_y){};
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator +(const Rect2<U> p_v) const { return (Rect2<T>(x + p_v.x, y + p_v.y, w + p_v.w, h + p_v.h)); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator -(const Rect2<U> p_v) const { return (Rect2<T>(x - p_v.x, y - p_v.y, w - p_v.w, h - p_v.h)); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator /(const Rect2<U> p_v) const { return Rect2<T>(x / p_v.x, y / p_v.y, w / p_v.w, h / p_v.h); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator /(const Vec2<U> p_v) const { return Rect2<T>(x / p_v.x, y / p_v.x, w / p_v.y, h / p_v.y); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator /(const U p_v) const { return Rect2<T>(x / p_v, y / p_v, w / p_v, h / p_v); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator *(const Rect2<U> p_v) const { return Rect2<T>(x * p_v.x, y * p_v.y, w * p_v.w, h * p_v.h); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator *(const Vec2<U> p_v) const { return Rect2<T>(x * p_v.x, y * p_v.x, w * p_v.y, h * p_v.y); };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T> operator *(const U p_v) const { return Rect2<T>(x * p_v, y * p_v, w * p_v, h * p_v); };
        
        Rect2<T> operator -() const { return Rect2<T>(-x, -y, -w, -h); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator +=(const Rect2<U> p_v) { x += p_v.x; y += p_v.y; w += p_v.w; h += p_v.h; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator +=(const Vec2<U> p_v) { x += p_v.x; y += p_v.y; return *this; }; //Notation shorthand
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator -=(const Rect2<U> p_v) { x -= p_v.x; y -= p_v.y; w -= p_v.w; h -= p_v.h; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator -=(const Vec2<U> p_v) { x -= p_v.x; y -= p_v.y; return *this; }; //Notation shorthand
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator /=(const Rect2<U> p_v) { x /= p_v.x; y /= p_v.y; w /= p_v.w; h /= p_v.h; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator /=(const Vec2<U> p_v) { x /= p_v.x; y /= p_v.x; w /= p_v.y; h /= p_v.y; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator /=(const U p_v) { x /= p_v; y /= p_v; w /= p_v; h /= p_v; return *this; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator *=(const Rect2<U> p_v) { x *= p_v.x; y *= p_v.y; w *= p_v.w; h *= p_v.h; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator *=(const Vec2<U> p_v) { x *= p_v.x; y *= p_v.x; w *= p_v.y; h *= p_v.y; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator *=(const U p_v) { x *= p_v; y *= p_v; w *= p_v; h *= p_v; return *this; };
        
        bool operator ==(const Rect2<T>& p_v) const { return x == p_v.x && y == p_v.y && w == p_v.w && h == p_v.h; };
        bool operator !=(const Rect2<T>& p_v) const { return x != p_v.x || y != p_v.y || w != p_v.w || h != p_v.h; };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator =(const Rect2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; w = (T)p_v.w; h = (T)p_v.h; return *this; };
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
        Rect2<T>& operator =(const Vec2<U> p_v) { x = (T)p_v.x; y = (T)p_v.y; return *this; }; //Notation shorthand
        
        Vec2<T> pos() { return Vec2<T>(x, y); };
        Vec2<T> size() { return Vec2<T>(w, h); };
        
        template<typename U, std::enable_if_t<std::is_arithmetic_v<U>, bool> = true>
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
}
