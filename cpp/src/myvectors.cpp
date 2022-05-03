#include <cmath>

#include "myvectors.h"
#include "fpm/math.hpp"

vector2fl::vector2fl()
{
	x = 0;
	y = 0;
}
vector2fl::vector2fl(float _x, float _y)
{
	x = _x;
	y = _y;
}
vector2fl::vector2fl(const vector2fl &c)
{
	x = c.x;
	y = c.y;
}
vector2fl::vector2fl(const vector2i &c)
{
    x = c.x;
    y = c.y;
}
vector2fl vector2fl::operator=(const vector2fl &c)
{
	x = c.x;
	y = c.y;
	return (*this);
}
bool vector2fl::operator==(const vector2fl &c) const
{
	return (x == c.x && y == c.y);
}
vector2fl vector2fl::operator+(const vector2fl &c) const
{
	return vector2fl(x + c.x, y + c.y);
}
void vector2fl::operator+=(const vector2fl &c)
{
	x += c.x;
	y += c.y;
}
vector2fl vector2fl::operator-(const vector2fl &c) const
{
	return vector2fl(x - c.x, y - c.y);
}
void vector2fl::operator-=(const vector2fl &c)
{
	x -= c.x;
	y -= c.y;
}
vector2fl vector2fl::operator*(float c) const
{
	return vector2fl(x * c, y * c);
}
float vector2fl::operator*(const vector2fl &vector) const
    {
        return x * vector.x + y * vector.y;
    }
void vector2fl::operator*=(float c)
{
	x *= c;
	y *= c;
}
vector2fl vector2fl::operator/(float c) const
{
	return vector2fl(x / c, y / c);
}
float vector2fl::getMagnitudeSquared() const
{
	return x * x + y * y;
}
float vector2fl::getMagnitude() const
{
	return sqrt(getMagnitudeSquared());
}
float vector2fl::getAngle() const
{
	return atan2(y, x);
}
vector2fl vector2fl::normalized() const
{
	float magnitude = getMagnitude();
	return vector2fl(x / magnitude, y / magnitude);
}
vector2fl vector2fl::rotated(float angle) const
{
	float newAngle = angle + getAngle();
	return composeVector2fl(newAngle, getMagnitude());
}
vector2fl composeVector2fl(float angle, float magnitude) {
	return vector2fl(magnitude * cos(angle), magnitude * sin(angle));
}
vector2fl interpolate(vector2fl a, vector2fl b, float i) {
    return ((b - a) * i) + a;
}
vector2fl operator*(float s, const vector2fl &vector)
{
    return vector2fl(s * vector.x, s * vector.y);
}
vector2i vector2fl::floored() const
{
    return vector2i((int)x, (int)y);
}
vector2fl vector2fl::operator-() const
{
    return vector2fl(-x, -y);
}

float det(const vector2fl &vector1, const vector2fl &vector2)
{
    return vector1.x * vector2.y - vector1.y * vector2.x;
}


vector2fp::vector2fp()
{
	x = fixed32(0);
	y = fixed32(0);
}
vector2fp::vector2fp(fixed32 _x, fixed32 _y)
{
	x = _x;
	y = _y;
}
vector2fp::vector2fp(const vector2fp &c)
{
	x = c.x;
	y = c.y;
}
vector2fp::vector2fp(const vector2i &c)
{
    x = fixed32(c.x);
    y = fixed32(c.y);
}
vector2fp vector2fp::operator=(const vector2fp &c)
{
	x = c.x;
	y = c.y;
	return (*this);
}
bool vector2fp::operator==(const vector2fp &c) const
{
	return (x == c.x && y == c.y);
}
vector2fp vector2fp::operator+(const vector2fp &c) const
{
	return vector2fp(x + c.x, y + c.y);
}
void vector2fp::operator+=(const vector2fp &c)
{
	x += c.x;
	y += c.y;
}
vector2fp vector2fp::operator-(const vector2fp &c) const
{
	return vector2fp(x - c.x, y - c.y);
}
void vector2fp::operator-=(const vector2fp &c)
{
	x -= c.x;
	y -= c.y;
}
vector2fp vector2fp::operator*(fixed32 c) const
{
	return vector2fp(x * c, y * c);
}
fixed32 vector2fp::operator*(const vector2fp &vector) const
    {
        return x * vector.x + y * vector.y;
    }
void vector2fp::operator*=(fixed32 c)
{
	x *= c;
	y *= c;
}
vector2fp vector2fp::operator/(fixed32 c) const
{
	return vector2fp(x / c, y / c);
}
fixed32 vector2fp::getMagnitudeSquared() const
{
	return x * x + y * y;
}
fixed32 vector2fp::getMagnitude() const
{
	return sqrt(getMagnitudeSquared());
}
fixed32 vector2fp::getAngle() const
{
	return atan2(y, x);
}
vector2fp vector2fp::normalized() const
{
	fixed32 magnitude = getMagnitude();
	return vector2fp(x / magnitude, y / magnitude);
}
vector2fp vector2fp::rotated(fixed32 angle) const
{
	fixed32 newAngle = angle + getAngle();
	return composeVector2fp(newAngle, getMagnitude());
}
vector2fp composeVector2fp(fixed32 angle, fixed32 magnitude) {
	return vector2fp(magnitude * cos(angle), magnitude * sin(angle));
}
vector2fp interpolate(vector2fp a, vector2fp b, fixed32 i) {
    return ((b - a) * i) + a;
}
vector2fp operator*(fixed32 s, const vector2fp &vector)
{
    return vector2fp(s * vector.x, s * vector.y);
}
vector2i vector2fp::floored() const
{
    return vector2i((int)x, (int)y);
}
vector2fp vector2fp::operator-() const
{
    return vector2fp(-x, -y);
}

fixed32 det(const vector2fp &vector1, const vector2fp &vector2)
{
    return vector1.x * vector2.y - vector1.y * vector2.x;
}


vector2i::vector2i()
{
	x = 0;
	y = 0;
}
vector2i::vector2i(int _x, int _y)
{
	x = _x;
	y = _y;
}
vector2i::vector2i(const vector2fl &c)
{
	x = c.x;
	y = c.y;
}
vector2i::vector2i(const vector2i &c)
{
	x = c.x;
	y = c.y;
}
vector2i vector2i::operator=(const vector2i &c)
{
	x = c.x;
	y = c.y;
	return (*this);
}
bool vector2i::operator==(const vector2i &c) const
{
	return (x == c.x && y == c.y);
}
vector2i vector2i::operator+(const vector2i &c) const
{
	return vector2i(x + c.x, y + c.y);
}
void vector2i::operator+=(const vector2i &c)
{
	x += c.x;
	y += c.y;
}
vector2i vector2i::operator-(const vector2i &c) const
{
	return vector2i(x - c.x, y - c.y);
}
void vector2i::operator-=(const vector2i &c)
{
	x -= c.x;
	y -= c.y;
}
vector2fl vector2i::operator*(float c) const
{
	return vector2fl(x * c, y * c);
}
vector2i vector2i::operator*(int c) const
{
	return vector2i(x * c, y * c);
}
void vector2i::operator*=(float c)
{
	x *= c;
	y *= c;
}
void vector2i::operator*=(int c)
{
	x *= c;
	y *= c;
}
vector2fl vector2i::operator/(float c) const
{
	return vector2fl(x / c, y / c);
}
vector2i vector2i::operator/(int c) const
{
	return vector2i(x / c, y / c);
}
int vector2i::getMagnitudeSquared() const
{
	return x * x + y * y;
}
float vector2i::getMagnitude() const
{
	return sqrt(getMagnitudeSquared());
}
float vector2i::getAngle() const
{
	return atan2(y, x);
}
vector2fl vector2i::normalized() const
{
	float magnitude = getMagnitude();
	return vector2fl(x / magnitude, y / magnitude);
}
vector2fl vector2i::rotated(float angle) const
{
	float magnitude = getMagnitude();
	float newAngle = angle + getAngle();
	return vector2fl(magnitude * cos(newAngle), magnitude * sin(newAngle));
}

vector3f::vector3f()
{
	x = 0;
	y = 0;
	z = 0;
}
vector3f::vector3f(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}
vector3f::vector3f(const vector3f &c)
{
	x = c.x;
	y = c.y;
	z = c.z;
}
vector3f::vector3f(const vector2fl &c)
{
	x = c.x;
	y = c.y;
	z = 0;
}
vector2fl vector3f::vec2() const
{
	return vector2fl(x, y);
}
vector3f vector3f::operator=(const vector3f &c)
{
	x = c.x;
	y = c.y;
	z = c.z;
	return (*this);
}
bool vector3f::operator==(const vector3f &c) const
{
	return (x == c.x && y == c.y && z == c.z);
}
vector3f vector3f::operator+(const vector3f &c) const
{
	return vector3f(x + c.x, y + c.y, z + c.z);
}
void vector3f::operator+=(const vector3f &c)
{
	x += c.x;
	y += c.y;
	z += c.z;
}
vector3f vector3f::operator-(const vector3f &c) const
{
	int j = 0;
	j++;
	return vector3f(x - c.x, y - c.y, z - c.z);
}
vector3f vector3f::operator*(float c) const
{
	return vector3f(x * c, y * c, z * c);
}
void vector3f::operator*=(float c)
{
	x *= c;
	y *= c;
	z *= c;
}
vector3f vector3f::operator/(float c) const
{
	return vector3f(x / c, y / c, z / c);
}
vector3f vector3f::cross(const vector3f &c) const
{
	return vector3f(y * c.z - z * c.y,
					z * c.x - x * c.z,
					x * c.y - y * c.x);
}
float vector3f::dot(const vector3f &c) const
{
	return (x * c.x + y * c.y + z * c.z);
}
float vector3f::getMagnitudeSquared() const
{
	return x * x + y * y + z * z;
}
float vector3f::getMagnitude() const
{
	return sqrt(getMagnitudeSquared());
}
vector3f vector3f::normalized() const
{
	float magnitude = getMagnitude();
	return vector3f(x / magnitude, y / magnitude, z / magnitude);
}
void vector3f::normalize()
{
	*this = normalized();
}

vector2fl vector3fTo2f(const vector3f &c)
{
	return vector2fl(c.x, c.y);
}

void quaternion::normalize()
{
	float mag = sqrt(w * w + x * x + y * y + z * z);
	x /= mag;
	y /= mag;
	z /= mag;
	w /= mag;
}
quaternion::quaternion()
{
	x = y = z = 0;
	w = 1;
}
quaternion::quaternion(float _x, float _y, float _z, float _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
	normalize();
}
void quaternion::setFromAxisRotation(const vector3f &v, float angle)
{
	float sinAngle;
	angle *= 0.5f;
	vector3f vn(v);
	vn.normalize();

	sinAngle = sin(angle);

	x = (vn.x * sinAngle);
	y = (vn.y * sinAngle);
	z = (vn.z * sinAngle);
	w = cos(angle);
}
void quaternion::setFromTwoVectors(const vector3f &vecFrom, const vector3f &vecTo)
{
	vector3f from(vecFrom);
	vector3f to(vecTo);
	vector3f c(from.cross(to));
	float d = from.dot(to);

	quaternion q(c.x, c.y, c.z, d + (float)sqrt(from.getMagnitudeSquared() * to.getMagnitudeSquared()));
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
}
quaternion quaternion::getConjugate()
{
	return quaternion(-x, -y, -z, w);
}
quaternion quaternion::operator*(const quaternion &c)
{
	return quaternion(w * c.x + x * c.w + y * c.z - z * c.y,
					  w * c.y + y * c.w + z * c.x - x * c.z,
					  w * c.z + z * c.w + x * c.y - y * c.x,
					  w * c.w - x * c.x - y * c.y - z * c.z);
}
vector3f quaternion::operator*(const vector3f &c)
{
	vector3f vn(c);
	float vecMag = vn.getMagnitude();
	vn.normalize();

	quaternion vecQuat, resQuat;
	vecQuat.x = vn.x;
	vecQuat.y = vn.y;
	vecQuat.z = vn.z;
	vecQuat.w = 0;

	resQuat = vecQuat * getConjugate();
	resQuat = *this * resQuat;

	return vector3f(resQuat.x, resQuat.y, resQuat.z) * vecMag;
}
vector2fl quaternion::operator*(const vector2fl &c)
{
	vector3f v = (*this) * vector3f(c.x, 0, c.y);
	return vector2fl(v.x, v.z);
}
quaternion quaternion::nlerp(const quaternion &c, float s)
{
	float sm = 1 - s;
	quaternion result(sm * x + s * c.x,
					  sm * y + s * c.y,
					  sm * z + s * c.z,
					  sm * w + s * c.w);
	result.normalize();
	return result;
}
quaternion quaternion::reversed()
{
	return quaternion(x, y, z, -w);
}