#include <cmath>

#include "myvectors.h"

vector2f::vector2f()
{
	x = 0;
	y = 0;
}
vector2f::vector2f(float _x, float _y)
{
	x = _x;
	y = _y;
}
vector2f::vector2f(const vector2f &c)
{
	x = c.x;
	y = c.y;
}
vector2f::vector2f(const vector2i &c)
{
    x = c.x;
    y = c.y;
}
vector2f vector2f::operator=(const vector2f &c)
{
	x = c.x;
	y = c.y;
	return (*this);
}
bool vector2f::operator==(const vector2f &c)
{
	return (x == c.x && y == c.y);
}
vector2f vector2f::operator+(const vector2f &c)
{
	return vector2f(x + c.x, y + c.y);
}
void vector2f::operator+=(const vector2f &c)
{
	x += c.x;
	y += c.y;
}
vector2f vector2f::operator-(const vector2f &c)
{
	return vector2f(x - c.x, y - c.y);
}
void vector2f::operator-=(const vector2f &c)
{
	x -= c.x;
	y -= c.y;
}
vector2f vector2f::operator*(float c)
{
	return vector2f(x * c, y * c);
}
void vector2f::operator*=(float c)
{
	x *= c;
	y *= c;
}
vector2f vector2f::operator/(float c)
{
	return vector2f(x / c, y / c);
}
float vector2f::getMagnitudeSquared()
{
	return x * x + y * y;
}
float vector2f::getMagnitude()
{
	return sqrt(getMagnitudeSquared());
}
float vector2f::getAngle()
{
	return atan2(y, x);
}
vector2f vector2f::normalized()
{
	float magnitude = getMagnitude();
	return vector2f(x / magnitude, y / magnitude);
}
vector2f vector2f::rotated(float angle)
{
	float newAngle = angle + getAngle();
	return composeVector2f(newAngle, getMagnitude());
}
vector2f composeVector2f(float angle, float magnitude) {
	return vector2f(magnitude * cos(angle), magnitude * sin(angle));
}
vector2f interpolate(vector2f a, vector2f b, float i) {
    return ((b - a) * i) + a;
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
vector2i::vector2i(const vector2f &c)
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
bool vector2i::operator==(const vector2i &c)
{
	return (x == c.x && y == c.y);
}
vector2i vector2i::operator+(const vector2i &c)
{
	return vector2i(x + c.x, y + c.y);
}
void vector2i::operator+=(const vector2i &c)
{
	x += c.x;
	y += c.y;
}
vector2i vector2i::operator-(const vector2i &c)
{
	return vector2i(x - c.x, y - c.y);
}
void vector2i::operator-=(const vector2i &c)
{
	x -= c.x;
	y -= c.y;
}
vector2f vector2i::operator*(float c)
{
	return vector2f(x * c, y * c);
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
vector2f vector2i::operator/(float c)
{
	return vector2f(x / c, y / c);
}
vector2i vector2i::operator/(int c)
{
	return vector2i(x / c, y / c);
}
int vector2i::getMagnitudeSquared()
{
	return x * x + y * y;
}
float vector2i::getMagnitude()
{
	return sqrt(getMagnitudeSquared());
}
float vector2i::getAngle()
{
	return atan2(y, x);
}
vector2f vector2i::normalized()
{
	float magnitude = getMagnitude();
	return vector2f(x / magnitude, y / magnitude);
}
vector2f vector2i::rotated(float angle)
{
	float magnitude = getMagnitude();
	float newAngle = angle + getAngle();
	return vector2f(magnitude * cos(newAngle), magnitude * sin(newAngle));
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
vector3f::vector3f(const vector2f &c)
{
	x = c.x;
	y = c.y;
	z = 0;
}
vector2f vector3f::vec2()
{
	return vector2f(x, y);
}
vector3f vector3f::operator=(const vector3f &c)
{
	x = c.x;
	y = c.y;
	z = c.z;
	return (*this);
}
bool vector3f::operator==(const vector3f &c)
{
	return (x == c.x && y == c.y && z == c.z);
}
vector3f vector3f::operator+(const vector3f &c)
{
	return vector3f(x + c.x, y + c.y, z + c.z);
}
void vector3f::operator+=(const vector3f &c)
{
	x += c.x;
	y += c.y;
	z += c.z;
}
vector3f vector3f::operator-(const vector3f &c)
{
	int j = 0;
	j++;
	return vector3f(x - c.x, y - c.y, z - c.z);
}
vector3f vector3f::operator*(float c)
{
	return vector3f(x * c, y * c, z * c);
}
void vector3f::operator*=(float c)
{
	x *= c;
	y *= c;
	z *= c;
}
vector3f vector3f::operator/(float c)
{
	return vector3f(x / c, y / c, z / c);
}
vector3f vector3f::cross(const vector3f &c)
{
	return vector3f(y * c.z - z * c.y,
					z * c.x - x * c.z,
					x * c.y - y * c.x);
}
float vector3f::dot(const vector3f &c)
{
	return (x * c.x + y * c.y + z * c.z);
}
float vector3f::getMagnitudeSquared()
{
	return x * x + y * y + z * z;
}
float vector3f::getMagnitude()
{
	return sqrt(getMagnitudeSquared());
}
vector3f vector3f::normalized()
{
	float magnitude = getMagnitude();
	return vector3f(x / magnitude, y / magnitude, z / magnitude);
}
void vector3f::normalize()
{
	*this = normalized();
}

vector2f vector3fTo2f(const vector3f &c)
{
	return vector2f(c.x, c.y);
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
vector2f quaternion::operator*(const vector2f &c)
{
	vector3f v = (*this) * vector3f(c.x, 0, c.y);
	return vector2f(v.x, v.z);
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