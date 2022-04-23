#include <cmath>

#ifndef MYVECTORS_H
#define MYVECTORS_H

struct vector2f;
struct vector2i;

struct vector2f
{
    float x, y;
    vector2f();
    vector2f(float _x, float _y);
    vector2f(const vector2f &c);
    vector2f(const vector2i &c);
    vector2f operator=(const vector2f &c);
    bool operator==(const vector2f &c) const;
    vector2f operator+(const vector2f &c) const;
    void operator+=(const vector2f &c);
    vector2f operator-(const vector2f &c) const;
    void operator-=(const vector2f &c);
    vector2f operator*(float c) const;
    float operator*(const vector2f &vector) const;
    void operator*=(float c);
    vector2f operator/(float c) const;
    float getMagnitudeSquared() const;
    float getMagnitude() const;
    float getAngle() const;
    vector2f normalized() const;
    vector2f rotated(float angle) const;
    vector2i floored() const;
    vector2f operator-() const;
};
vector2f composeVector2f(float angle, float magnitude);
vector2f interpolate(const vector2f &, const vector2f &, float);
vector2f operator*(float s, const vector2f &vector);
float det(const vector2f &vector1, const vector2f &vector2);

struct vector2i
{
    int x, y;
    vector2i();
    vector2i(int _x, int _y);
    vector2i(const vector2f &c);
    vector2i(const vector2i &c);
    vector2i operator=(const vector2i &c);
    bool operator==(const vector2i &c) const;
    vector2i operator+(const vector2i &c) const;
    void operator+=(const vector2i &c);
    vector2i operator-(const vector2i &c) const;
    void operator-=(const vector2i &c);
    vector2f operator*(float c) const;
    vector2i operator*(int c) const;
    void operator*=(float c);
    void operator*=(int c);
    vector2f operator/(float c) const;
    vector2i operator/(int c) const;
    int getMagnitudeSquared() const;
    float getMagnitude() const;
    float getAngle() const;
    vector2f normalized() const;
    vector2f rotated(float angle) const;
};

struct vector3f
{
    float x, y, z;
    vector3f();
    vector3f(float _x, float _y, float _z);
    vector3f(const vector3f &c);
    vector3f(const vector2f &c);
    vector2f vec2() const;
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

vector2f vector3fTo2f(const vector3f &c);

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
    vector2f operator*(const vector2f &c);
    quaternion nlerp(const quaternion &c, float s);
    quaternion reversed();
};

#endif // MYVECTORS_H