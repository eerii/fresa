//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include <math.h>
#include "dtypes.h"

using namespace Verse;

//VEC2 - 2D Vector
//-------------------------------------

Vec2::Vec2():x(.0f),y(.0f){}
Vec2::Vec2(float p_x, float p_y):x(p_x),y(p_y){}
Vec2::Vec2(int p_x, float p_y):x((float)p_x),y(p_y){}
Vec2::Vec2(float p_x, int p_y):x(p_x),y((float)p_y){}
Vec2::Vec2(int p_x, int p_y):x((float)p_x),y((float)p_y){}

Vec2 Vec2::operator +(const Vec2 p_v) const { return Vec2(x + p_v.x, y + p_v.y); }
Vec2 Vec2::operator -(const Vec2 p_v) const { return Vec2(x - p_v.x, y - p_v.y); }
Vec2 Vec2::operator /(const float p_f) const { return Vec2(x / p_f, y / p_f); }
Vec2 Vec2::operator *(const float p_f) const { return Vec2(x * p_f, y * p_f); }
Vec2 Vec2::operator -() const { return Vec2(-x, -y); }

Vec2& Vec2::operator +=(const Vec2& p_v) { x += p_v.x; y += p_v.y; return *this; }
Vec2& Vec2::operator -=(const Vec2& p_v) { x -= p_v.x; y -= p_v.y; return *this; }
Vec2& Vec2::operator /=(float p_f) { x /= p_f; y /= p_f; return *this; }
Vec2& Vec2::operator *=(float p_f) { x *= p_f; y *= p_f; return *this; }

bool Vec2::operator ==(const Vec2& p_v) { return x == p_v.x && y == p_v.y; }
bool Vec2::operator !=(const Vec2& p_v) { return x != p_v.x || y != p_v.y; }

Vec2 Vec2::normal() const {
    if (x == 0 && y == 0)
        return zero;
    float length = this->length();
    return Vec2(x / length, y / length);
}
float Vec2::length() const {
    return sqrtf(x * x + y * y);
}
float Vec2::length2() const {
    return x * x + y * y;
    
}
Vec2 Vec2::perpendicular() const {
    return Vec2(-y, x);
}
float Vec2::angle() const {
    return atan2(y, x);
}

float Vec2::dot(Vec2 p_v1, Vec2 p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; }

Vec2 Vec2::from_angle(float p_rad, float p_len) {
    return Vec2((float)cos(p_rad) * p_len, (float)sin(p_rad) * p_len);
}
Vec2 Vec2::from_angle(float p_rad) {
    return from_angle(p_rad, 1);
}

Vec2 Vec2::reflect(const Vec2 &p_v, const Vec2 &p_n) {
    float dot_vn = dot(p_v, p_n);
    
    return Vec2(p_v.x - 2.0f * dot_vn * p_n.x,
                p_v.y - 2.0f * dot_vn * p_n.y);
}

const Vec2 Vec2::i = Vec2(1, 0);
const Vec2 Vec2::j = Vec2(0, 1);
const Vec2 Vec2::right = Vec2(1, 0);
const Vec2 Vec2::up = Vec2(0, -1);
const Vec2 Vec2::down = Vec2(0, 1);
const Vec2 Vec2::left = Vec2(-1, 0);
const Vec2 Vec2::zero = Vec2(0, 0);
const Vec2 Vec2::one = Vec2(1, 1);

#define DIAGONAL_UNIT 0.70710678118f
const Vec2 Vec2::up_right = Vec2(DIAGONAL_UNIT, -DIAGONAL_UNIT);
const Vec2 Vec2::up_left = Vec2(-DIAGONAL_UNIT, -DIAGONAL_UNIT);
const Vec2 Vec2::down_right = Vec2(DIAGONAL_UNIT, DIAGONAL_UNIT);
const Vec2 Vec2::down_left = Vec2(-DIAGONAL_UNIT, DIAGONAL_UNIT);
#undef DIAGONAL_UNIT

//-------------------------------------

//RECT

Rect::Rect():pos(Vec2()),size(Vec2()){};
Rect::Rect(Vec2 pos, Vec2 size):pos(pos),size(size){}
Rect::Rect(int pos_x, int pos_y, int size_x, int size_y):pos(Vec2(pos_x, pos_y)),size(Vec2(size_x, size_y)){};
Rect::Rect(float pos_x, float pos_y, float size_x, float size_y):pos(Vec2(pos_x, pos_y)),size(Vec2(size_x, size_y)){};

Rect Rect::operator +(const Vec2 p_pos) const { return Rect(pos + p_pos, size); };
Rect Rect::operator -(const Vec2 p_pos) const { return Rect(pos - p_pos, size); };
Rect Rect::operator *(const float p_f) const { return Rect(pos * p_f, size); };
Rect Rect::operator /(const float p_f) const { return Rect(pos / p_f, size); };
Rect Rect::operator -() const { return Rect(-pos, -size); };

Rect& Rect::operator +=(const Vec2 p_pos) { pos += p_pos; return *this; };
Rect& Rect::operator -=(const Vec2 p_pos) { pos -= p_pos; return *this; };
Rect& Rect::operator *=(const float p_f) { pos *= p_f; return *this; };
Rect& Rect::operator /=(const float p_f) { pos /= p_f; return *this; };

Vertices Rect::toVertices() {
    Vertices vert;
    
    vert[TOPLEFT] = pos;
    vert[TOPRIGHT] = Vec2(pos.x + size.x, pos.y);
    vert[BOTTOMLEFT] = Vec2(pos.x, pos.y + size.y);
    vert[BOTTOMRIGHT] = Vec2(pos.x + size.x, pos.y + size.y);
    
    return vert;
}

SDL_Rect Rect::toSDL() {
    SDL_Rect rect;
    
    rect.x = pos.x;
    rect.y = pos.y;
    rect.w = size.x;
    rect.h = size.y;
    
    return rect;
}


//-------------------------------------
