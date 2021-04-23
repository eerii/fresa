//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>
#include <string>

#define TOPLEFT 0
#define TOPRIGHT 1
#define BOTTOMLEFT 2
#define BOTTOMRIGHT 3

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

    //VEC2 - 2D Vector
    struct Vec2 {
        float x, y;

        Vec2();
        Vec2(float p_x, float p_y);
        Vec2(int p_x, float p_y);
        Vec2(float p_x, int p_y);
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
        
        Vec2 normal() const;
        float length() const;
        float length2() const;
        Vec2 perpendicular() const;
        float angle() const;
        
        static float dot(Vec2 p_v1, Vec2 p_v2);

        static Vec2 from_angle(float p_rad, float p_len);
        static Vec2 from_angle(float p_rad);
        static Vec2 reflect(const Vec2& p_v, const Vec2& p_n);

        static const Vec2 i;
        static const Vec2 j;
        static const Vec2 right;
        static const Vec2 up;
        static const Vec2 down;
        static const Vec2 left;
        static const Vec2 zero;
        static const Vec2 one;
        static const Vec2 up_right;
        static const Vec2 up_left;
        static const Vec2 down_right;
        static const Vec2 down_left;
    };

    typedef struct Vertices { Vec2 x[4]; Vec2& operator[](int i){ return x[i]; } } Vertices;

    struct Rect {
        Vec2 pos, size;
        
        Rect();
        Rect(Vec2 pos, Vec2 size);
        Rect(int pos_x, int pos_y, int size_x, int size_y);
        Rect(float pos_x, float pos_y, float size_x, float size_y);
        
        Rect operator +(const Vec2 p_pos) const;
        Rect operator -(const Vec2 p_pos) const;
        Rect operator *(const float p_f) const;
        Rect operator /(const float p_f) const;
        Rect operator -() const;
        
        Rect& operator +=(const Vec2 p_pos);
        Rect& operator -=(const Vec2 p_pos);
        Rect& operator *=(const float p_f);
        Rect& operator /=(const float p_f);
        
        Vertices toVertices();
        SDL_Rect toSDL();
    };

    //SIGN
    template <typename T> int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }
}
