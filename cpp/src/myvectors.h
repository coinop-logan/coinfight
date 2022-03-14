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
    bool operator==(const vector2f &c);
    vector2f operator+(const vector2f &c);
    void operator+=(const vector2f &c);
    vector2f operator-(const vector2f &c);
    void operator-=(const vector2f &c);
    vector2f operator*(float c);
    void operator*=(float c);
    vector2f operator/(float c);
    float getMagnitudeSquared();
    float getMagnitude();
    float getAngle();
    vector2f normalized();
    vector2f rotated(float angle);
};
vector2f composeVector2f(float angle, float magnitude);
vector2f interpolate(const vector2f &, const vector2f &, float);

struct vector2i
{
    int x, y;
    vector2i();
    vector2i(int _x, int _y);
    vector2i(const vector2f &c);
    vector2i(const vector2i &c);
    vector2i operator=(const vector2i &c);
    bool operator==(const vector2i &c);
    vector2i operator+(const vector2i &c);
    void operator+=(const vector2i &c);
    vector2i operator-(const vector2i &c);
    void operator-=(const vector2i &c);
    vector2f operator*(float c);
    vector2i operator*(int c) const;
    void operator*=(float c);
    void operator*=(int c);
    vector2f operator/(float c);
    vector2i operator/(int c);
    int getMagnitudeSquared();
    float getMagnitude();
    float getAngle();
    vector2f normalized();
    vector2f rotated(float angle);
};

struct vector3f
{
    float x, y, z;
    vector3f();
    vector3f(float _x, float _y, float _z);
    vector3f(const vector3f &c);
    vector3f(const vector2f &c);
    vector2f vec2();
    vector3f operator=(const vector3f &c);
    bool operator==(const vector3f &c);
    vector3f operator+(const vector3f &c);
    void operator+=(const vector3f &c);
    vector3f operator-(const vector3f &c);
    vector3f operator*(float c);
    void operator*=(float c);
    vector3f operator/(float c);
    vector3f cross(const vector3f &c);
    float dot(const vector3f &c);
    float getMagnitudeSquared();
    float getMagnitude();
    vector3f normalized();
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