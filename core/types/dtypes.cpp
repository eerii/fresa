//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "dtypes.h"

using namespace Verse;

//VEC2 - 2D Vector
//-------------------------------------

Vec2::Vec2():x(0),y(0){}
Vec2::Vec2(int p_x, int p_y):x(p_x),y(p_y){}

Vec2 Vec2::operator +(const Vec2 p_v) const { return Vec2(x + p_v.x, y + p_v.y); }
Vec2 Vec2::operator -(const Vec2 p_v) const { return Vec2(x - p_v.x, y - p_v.y); }
Vec2 Vec2::operator /(const float p_f) const { return Vec2(round((float)x / p_f), round((float)y / p_f)); }
Vec2 Vec2::operator *(const float p_f) const { return Vec2(round((float)x * p_f), round((float)y * p_f)); }
Vec2 Vec2::operator -() const { return Vec2(-x, -y); }

Vec2& Vec2::operator +=(const Vec2& p_v) { x += p_v.x; y += p_v.y; return *this; }
Vec2& Vec2::operator -=(const Vec2& p_v) { x -= p_v.x; y -= p_v.y; return *this; }
Vec2& Vec2::operator /=(float p_f) { x = round((float)x / p_f); y = round((float)y / p_f); return *this; }
Vec2& Vec2::operator *=(float p_f) { x = round((float)x * p_f); y = round((float)y * p_f); return *this; }

bool Vec2::operator ==(const Vec2& p_v) const { return x == p_v.x && y == p_v.y; }
bool Vec2::operator !=(const Vec2& p_v) const { return x != p_v.x || y != p_v.y; }

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

Vec2f Vec2::to_float() const {
    return Vec2f(x, y);
}

int Vec2::dot(Vec2 p_v1, Vec2 p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; }

Vec2 Vec2::reflect(const Vec2 &p_v, const Vec2 &p_n) {
    int dot_vn = dot(p_v, p_n);
    
    return Vec2(p_v.x - 2 * dot_vn * p_n.x,
                p_v.y - 2 * dot_vn * p_n.y);
}

const Vec2 Vec2::i = Vec2(1, 0);
const Vec2 Vec2::j = Vec2(0, 1);
const Vec2 Vec2::right = Vec2(1, 0);
const Vec2 Vec2::up = Vec2(0, -1);
const Vec2 Vec2::down = Vec2(0, 1);
const Vec2 Vec2::left = Vec2(-1, 0);
const Vec2 Vec2::zero = Vec2(0, 0);
const Vec2 Vec2::one = Vec2(1, 1);

//-------------------------------------

//VEC2f - 2D Vector (float)
//-------------------------------------

Vec2f::Vec2f():x(.0f),y(.0f){}
Vec2f::Vec2f(float p_x, float p_y):x(p_x),y(p_y){}
Vec2f::Vec2f(int p_x, float p_y):x((float)p_x),y(p_y){}
Vec2f::Vec2f(float p_x, int p_y):x(p_x),y((float)p_y){}
Vec2f::Vec2f(int p_x, int p_y):x((float)p_x),y((float)p_y){}

Vec2f Vec2f::operator +(const Vec2f p_v) const { return Vec2f(x + p_v.x, y + p_v.y); }
Vec2f Vec2f::operator -(const Vec2f p_v) const { return Vec2f(x - p_v.x, y - p_v.y); }
Vec2f Vec2f::operator /(const float p_f) const { return Vec2f(x / p_f, y / p_f); }
Vec2f Vec2f::operator *(const float p_f) const { return Vec2f(x * p_f, y * p_f); }
Vec2f Vec2f::operator -() const { return Vec2f(-x, -y); }

Vec2f& Vec2f::operator +=(const Vec2f& p_v) { x += p_v.x; y += p_v.y; return *this; }
Vec2f& Vec2f::operator -=(const Vec2f& p_v) { x -= p_v.x; y -= p_v.y; return *this; }
Vec2f& Vec2f::operator /=(float p_f) { x /= p_f; y /= p_f; return *this; }
Vec2f& Vec2f::operator *=(float p_f) { x *= p_f; y *= p_f; return *this; }

bool Vec2f::operator ==(const Vec2f& p_v) const { return x == p_v.x && y == p_v.y; }
bool Vec2f::operator !=(const Vec2f& p_v) const { return x != p_v.x || y != p_v.y; }

Vec2f Vec2f::normal() const {
    if (x == 0 && y == 0)
        return zero;
    float length = this->length();
    return Vec2f(x / length, y / length);
}
float Vec2f::length() const {
    return sqrtf(x * x + y * y);
}
float Vec2f::length2() const {
    return x * x + y * y;
    
}
Vec2f Vec2f::perpendicular() const {
    return Vec2f(-y, x);
}
float Vec2f::angle() const {
    return atan2(y, x);
}
Vec2 Vec2f::to_int() const {
    return Vec2(floor(x), floor(y));
}

float Vec2f::dot(Vec2f p_v1, Vec2f p_v2) { return p_v1.x * p_v2.x + p_v1.y * p_v2.y; }

Vec2f Vec2f::from_angle(float p_rad, float p_len) {
    return Vec2f((float)cos(p_rad) * p_len, (float)sin(p_rad) * p_len);
}
Vec2f Vec2f::from_angle(float p_rad) {
    return from_angle(p_rad, 1);
}

Vec2f Vec2f::reflect(const Vec2f &p_v, const Vec2f &p_n) {
    float dot_vn = dot(p_v, p_n);
    
    return Vec2f(p_v.x - 2.0f * dot_vn * p_n.x,
                p_v.y - 2.0f * dot_vn * p_n.y);
}

const Vec2f Vec2f::i = Vec2f(1, 0);
const Vec2f Vec2f::j = Vec2f(0, 1);
const Vec2f Vec2f::right = Vec2f(1, 0);
const Vec2f Vec2f::up = Vec2f(0, -1);
const Vec2f Vec2f::down = Vec2f(0, 1);
const Vec2f Vec2f::left = Vec2f(-1, 0);
const Vec2f Vec2f::zero = Vec2f(0, 0);
const Vec2f Vec2f::one = Vec2f(1, 1);

#define DIAGONAL_UNIT 0.70710678118f
const Vec2f Vec2f::up_right = Vec2f(DIAGONAL_UNIT, -DIAGONAL_UNIT);
const Vec2f Vec2f::up_left = Vec2f(-DIAGONAL_UNIT, -DIAGONAL_UNIT);
const Vec2f Vec2f::down_right = Vec2f(DIAGONAL_UNIT, DIAGONAL_UNIT);
const Vec2f Vec2f::down_left = Vec2f(-DIAGONAL_UNIT, DIAGONAL_UNIT);
#undef DIAGONAL_UNIT

//-------------------------------------

//RECT

Rect2::Rect2():pos(0,0),size(0,0){link();};
Rect2::Rect2(Vec2 p_pos, Vec2 p_size):pos(p_pos), size(p_size){link();}
Rect2::Rect2(int pos_x, int pos_y, int size_x, int size_y):pos(pos_x, pos_y), size(size_x, size_y){link();};

void Rect2::link() {
    x = &pos.x;
    y = &pos.y;
    w = &size.x;
    h = &size.y;
}

Rect2f Rect2::to_float() const { return Rect2f((float)*x, (float)*y, (float)*w, (float)*h); };

Rect2 Rect2::operator +(const Vec2 p_pos) const { return (Rect2(pos + p_pos, size)); };
Rect2 Rect2::operator -(const Vec2 p_pos) const { return (Rect2(pos - p_pos, size)); };
Rect2 Rect2::operator +(const Rect2 p_rect) const { return (Rect2(pos + p_rect.pos, size + p_rect.size)); };
Rect2 Rect2::operator -(const Rect2 p_rect) const { return (Rect2(pos - p_rect.pos, size - p_rect.size)); };

Rect2 Rect2::operator *(const Vec2f p_v) const { return Rect2(pos * p_v.x, size * p_v.y); };
Rect2 Rect2::operator /(const Vec2f p_v) const { return Rect2(pos / p_v.x, size / p_v.y); };
Rect2 Rect2::operator *(const float p_f) const { return Rect2(pos * p_f, size * p_f); };
Rect2 Rect2::operator /(const float p_f) const { return Rect2(pos / p_f, size / p_f); };

Rect2 Rect2::operator -() const { return Rect2(-pos, -size); };

Rect2& Rect2::operator +=(const Vec2 p_pos) { pos += p_pos; return *this; };
Rect2& Rect2::operator -=(const Vec2 p_pos) { pos -= p_pos; return *this; };
Rect2& Rect2::operator +=(const Rect2 p_rect) { pos += p_rect.pos; size += p_rect.size; return *this; };
Rect2& Rect2::operator -=(const Rect2 p_rect) { pos -= p_rect.pos; size -= p_rect.size; return *this; };

Rect2& Rect2::operator *=(const Vec2f p_v) { pos *= p_v.x; size *= p_v.y; return *this; };
Rect2& Rect2::operator /=(const Vec2f p_v) { pos /= p_v.x; size /= p_v.y; return *this; };
Rect2& Rect2::operator *=(const float p_f) { pos *= p_f; size *= p_f; return *this; };
Rect2& Rect2::operator /=(const float p_f) { pos /= p_f; size /= p_f; return *this; };

Rect2& Rect2::operator =(const Rect2 p_rect) { pos = p_rect.pos; size = p_rect.size; return *this; };
Rect2& Rect2::operator =(const Vec2 p_pos) { pos = p_pos; return *this; };

SDL_Rect Rect2::toSDL() {
    SDL_Rect rect;
    
    rect.x = *x;
    rect.y = *y;
    rect.w = *w;
    rect.h = *h;
    
    return rect;
}

//RECT (float)

Rect2f::Rect2f():pos(0,0),size(0,0){link();};
Rect2f::Rect2f(Vec2f p_pos, Vec2f p_size):pos(p_pos), size(p_size){link();}
Rect2f::Rect2f(float pos_x, float pos_y, float size_x, float size_y):pos(pos_x, pos_y), size(size_x, size_y){link();};

void Rect2f::link() {
    x = &pos.x;
    y = &pos.y;
    w = &size.x;
    h = &size.y;
}

Rect2 Rect2f::to_int() const { return Rect2((int)floor(*x), (int)floor(*y), (int)floor(*w), (int)floor(*h)); };

Rect2f Rect2f::operator +(const Vec2f p_pos) const { return (Rect2f(pos + p_pos, size)); };
Rect2f Rect2f::operator -(const Vec2f p_pos) const { return (Rect2f(pos - p_pos, size)); };
Rect2f Rect2f::operator +(const Rect2f p_rect) const { return (Rect2f(pos + p_rect.pos, size + p_rect.size)); };
Rect2f Rect2f::operator -(const Rect2f p_rect) const { return (Rect2f(pos - p_rect.pos, size - p_rect.size)); };

Rect2f Rect2f::operator *(const Vec2f p_v) const { return Rect2f(pos * p_v.x, size * p_v.y); };
Rect2f Rect2f::operator /(const Vec2f p_v) const { return Rect2f(pos / p_v.x, size / p_v.y); };
Rect2f Rect2f::operator *(const float p_f) const { return Rect2f(pos * p_f, size * p_f); };
Rect2f Rect2f::operator /(const float p_f) const { return Rect2f(pos / p_f, size / p_f); };

Rect2f Rect2f::operator -() const { return Rect2f(-pos, -size); };

Rect2f& Rect2f::operator +=(const Vec2f p_pos) { pos += p_pos; return *this; };
Rect2f& Rect2f::operator -=(const Vec2f p_pos) { pos -= p_pos; return *this; };
Rect2f& Rect2f::operator +=(const Rect2f p_rect) { pos += p_rect.pos; size += p_rect.size; return *this; };
Rect2f& Rect2f::operator -=(const Rect2f p_rect) { pos -= p_rect.pos; size -= p_rect.size; return *this; };

Rect2f& Rect2f::operator *=(const Vec2f p_v) { pos *= p_v.x; size *= p_v.y; return *this; };
Rect2f& Rect2f::operator /=(const Vec2f p_v) { pos /= p_v.x; size /= p_v.y; return *this; };
Rect2f& Rect2f::operator *=(const float p_f) { pos *= p_f; size *= p_f; return *this; };
Rect2f& Rect2f::operator /=(const float p_f) { pos /= p_f; size /= p_f; return *this; };

Rect2f& Rect2f::operator =(const Rect2f p_rect) { pos = p_rect.pos; size = p_rect.size; return *this; };
Rect2f& Rect2f::operator =(const Vec2f p_pos) { pos = p_pos; return *this; };

SDL_Rect Rect2f::toSDL() {
    SDL_Rect rect;
    
    rect.x = *x;
    rect.y = *y;
    rect.w = *w;
    rect.h = *h;
    
    return rect;
}

//-------------------------------------
