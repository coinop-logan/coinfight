#include <cmath>
#include "fpm/fixed.hpp"

#ifndef MYVECTORS_H
#define MYVECTORS_H

struct vector2fl;
struct vector2fp;
struct vector2i;

struct vector2fl
{
    float x, y;
    vector2fl();
    vector2fl(float _x, float _y);
    vector2fl(const vector2fl &c);
    vector2fl(const vector2i &c);
    vector2fl operator=(const vector2fl &c);
    bool operator==(const vector2fl &c) const;
    vector2fl operator+(const vector2fl &c) const;
    void operator+=(const vector2fl &c);
    vector2fl operator-(const vector2fl &c) const;
    void operator-=(const vector2fl &c);
    vector2fl operator*(float c) const;
    float operator*(const vector2fl &vector) const;
    void operator*=(float c);
    vector2fl operator/(float c) const;
    float getMagnitudeSquared() const;
    float getMagnitude() const;
    float getAngle() const;
    vector2fl normalized() const;
    vector2fl rotated(float angle) const;
    vector2i floored() const;
    vector2fl operator-() const;
};
vector2fl composeVector2fl(float angle, float magnitude);
vector2fl interpolate(const vector2fl &, const vector2fl &, float);
vector2fl operator*(float s, const vector2fl &vector);
float det(const vector2fl &vector1, const vector2fl &vector2);

using fixed32 = fpm::fixed_16_16;

struct vector2fp
{
    fixed32 x, y;
    vector2fp();
    vector2fp(fixed32 _x, fixed32 _y);
    vector2fp(const vector2fp &c);
    vector2fp(const vector2i &c);
    static const vector2fp zero;
    vector2fp operator=(const vector2fp &c);
    bool operator==(const vector2fp &c) const;
    vector2fp operator+(const vector2fp &c) const;
    void operator+=(const vector2fp &c);
    vector2fp operator-(const vector2fp &c) const;
    void operator-=(const vector2fp &c);
    vector2fp operator*(fixed32 c) const;
    fixed32 operator*(const vector2fp &vector) const;
    void operator*=(fixed32 c);
    vector2fp operator/(fixed32 c) const;
    vector2fp operator/(int32_t c) const;
    uint32_t getFloorMagnitudeSquared() const;
    fixed32 getRoughMagnitude() const;
    fixed32 getAngle() const;
    vector2fp normalized() const;
    vector2fp rotated(fixed32 angle) const;
    vector2i floored() const;
    vector2fp operator-() const;
};
vector2fp composeVector2fp(fixed32 angle, fixed32 magnitude);
vector2fp interpolate(const vector2fp &, const vector2fp &, fixed32);
vector2fp operator*(fixed32 s, const vector2fp &vector);
fixed32 det(const vector2fp &vector1, const vector2fp &vector2);

struct vector2i
{
    int x, y;
    vector2i();
    vector2i(int _x, int _y);
    vector2i(const vector2fl &c);
    vector2i(const vector2fp &c);
    vector2i(const vector2i &c);
    vector2i operator=(const vector2i &c);
    bool operator==(const vector2i &c) const;
    vector2i operator+(const vector2i &c) const;
    void operator+=(const vector2i &c);
    vector2i operator-(const vector2i &c) const;
    void operator-=(const vector2i &c);
    vector2fl operator*(float c) const;
    vector2i operator*(int c) const;
    void operator*=(float c);
    void operator*=(int c);
    vector2fl operator/(float c) const;
    vector2i operator/(int c) const;
    int getMagnitudeSquared() const;
    float getMagnitude() const;
    float getAngle() const;
    vector2fl normalized() const;
    vector2fl rotated(float angle) const;
};

struct vector3f
{
    float x, y, z;
    vector3f();
    vector3f(float _x, float _y, float _z);
    vector3f(const vector3f &c);
    vector3f(const vector2fl &c);
    vector2fl vec2() const;
    vector3f operator=(const vector3f &c);
    bool operator==(const vector3f &c) const;
    vector3f operator+(const vector3f &c) const;
    void operator+=(const vector3f &c);
    vector3f operator-(const vector3f &c) const;
    vector3f operator*(float c) const;
    void operator*=(float c);
    vector3f operator/(float c) const;
    vector3f cross(const vector3f &c) const;
    float dot(const vector3f &c) const;
    float getMagnitudeSquared() const;
    float getMagnitude() const;
    vector3f normalized() const;
    void normalize();
};

vector2fl vector3fTo2f(const vector3f &c);

struct quaternion
{
    float x, y, z, w;
    void normalize();
    quaternion();
    quaternion(float _x, float _y, float _z, float _w);
    void setFromAxisRotation(const vector3f &v, float angle);
    void setFromTwoVectors(const vector3f &vecFrom, const vector3f &vecTo);
    quaternion getConjugate();
    quaternion operator*(const quaternion &c);
    vector3f operator*(const vector3f &c);
    vector2fl operator*(const vector2fl &c);
    quaternion nlerp(const quaternion &c, float s);
    quaternion reversed();
};

#endif // MYVECTORS_H