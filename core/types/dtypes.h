//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>
#include <string>

#define USE_OPENGL

namespace Verse
{
    //UNSIGNED INT
    typedef std::uint8_t ui8;
    typedef std::uint16_t ui16;
    typedef std::uint32_t ui32;
    typedef std::uint64_t ui64;

    //STRING
    typedef std::string str;

    struct Vec2f;

    //VEC2 - 2D Vector
    struct Vec2 {
        int x, y;

        Vec2();
        Vec2(int p_x, int p_y);
        
        Vec2 operator +(const Vec2 p_v) const;
        Vec2 operator -(const Vec2 p_v) const;
        Vec2 operator /(const float p_f) const;
        Vec2 operator *(const float p_f) const;
        Vec2 operator -() const;
        
        Vec2& operator +=(const Vec2& p_v);
        Vec2& operator -=(const Vec2& p_v);
        Vec2& operator /=(float p_f);
        Vec2& operator *=(float p_f);
        
        bool operator ==(const Vec2& p_v);
        bool operator !=(const Vec2& p_v);
        
        float length() const;
        float length2() const;
        Vec2 perpendicular() const;
        float angle() const;
        
        Vec2f to_float() const;
        
        static int dot(Vec2 p_v1, Vec2 p_v2);

        static Vec2 reflect(const Vec2& p_v, const Vec2& p_n);

        static const Vec2 i;
        static const Vec2 j;
        static const Vec2 right;
        static const Vec2 up;
        static const Vec2 down;
        static const Vec2 left;
        static const Vec2 zero;
        static const Vec2 one;
    };

    //VEC2f - 2D Vector (float)
    struct Vec2f {
        float x, y;

        Vec2f();
        Vec2f(float p_x, float p_y);
        Vec2f(int p_x, float p_y);
        Vec2f(float p_x, int p_y);
        Vec2f(int p_x, int p_y);
        
        Vec2f operator +(const Vec2f p_v) const;
        Vec2f operator -(const Vec2f p_v) const;
        Vec2f operator /(const float p_f) const;
        Vec2f operator *(const float p_f) const;
        Vec2f operator -() const;
        
        Vec2f& operator +=(const Vec2f& p_v);
        Vec2f& operator -=(const Vec2f& p_v);
        Vec2f& operator /=(float p_f);
        Vec2f& operator *=(float p_f);
        
        bool operator ==(const Vec2f& p_v);
        bool operator !=(const Vec2f& p_v);
        
        Vec2f normal() const;
        float length() const;
        float length2() const;
        Vec2f perpendicular() const;
        float angle() const;
        
        Vec2 to_int() const;
        
        static float dot(Vec2f p_v1, Vec2f p_v2);

        static Vec2f from_angle(float p_rad, float p_len);
        static Vec2f from_angle(float p_rad);
        static Vec2f reflect(const Vec2f& p_v, const Vec2f& p_n);

        static const Vec2f i;
        static const Vec2f j;
        static const Vec2f right;
        static const Vec2f up;
        static const Vec2f down;
        static const Vec2f left;
        static const Vec2f zero;
        static const Vec2f one;
        static const Vec2f up_right;
        static const Vec2f up_left;
        static const Vec2f down_right;
        static const Vec2f down_left;
    };

    struct Rect2f;

    //RECT2 - 4D Vector
    struct Rect2 {
        int x, y, w, h;
        
        Rect2();
        Rect2(Vec2 pos, Vec2 size);
        Rect2(int pos_x, int pos_y, int size_x, int size_y);
        
        Vec2 pos() const;
        Vec2 size() const;
        
        Rect2f to_float() const;
        
        Rect2 operator +(const Vec2 p_pos) const;
        Rect2 operator -(const Vec2 p_pos) const;
        Rect2 operator *(const float p_f) const;
        Rect2 operator /(const float p_f) const;
        Rect2 operator -() const;
        
        Rect2& operator +=(const Vec2 p_pos);
        Rect2& operator -=(const Vec2 p_pos);
        Rect2& operator *=(const float p_f);
        Rect2& operator /=(const float p_f);
        Rect2& operator =(const Vec2 p_pos);
        
        SDL_Rect toSDL();
    };

    //RECT2f - 4D Vector (float)
    struct Rect2f {
        int x, y, w, h;
        
        Rect2f();
        Rect2f(Vec2f pos, Vec2f size);
        Rect2f(float pos_x, float pos_y, float size_x, float size_y);
        
        Vec2f pos() const;
        Vec2f size() const;
        
        Rect2 to_int() const;
        
        Rect2f operator +(const Vec2f p_pos) const;
        Rect2f operator -(const Vec2f p_pos) const;
        Rect2f operator *(const float p_f) const;
        Rect2f operator /(const float p_f) const;
        Rect2f operator -() const;
        
        Rect2f& operator +=(const Vec2f p_pos);
        Rect2f& operator -=(const Vec2f p_pos);
        Rect2f& operator *=(const float p_f);
        Rect2f& operator /=(const float p_f);
        Rect2f& operator =(const Vec2f p_pos);
        
        SDL_Rect toSDL();
    };

    //SIGN
    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
    int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }
}
