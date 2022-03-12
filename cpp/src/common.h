#include <stdint.h>
#include <optional>
#include "myvectors.h"
#include "vchpack.h"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using EntityRef = uint16_t;

void packTypechar(vch *dest, unsigned char typechar);

void debugOutputVch(vch);
void debugOutputVector(const char *,vector2f);
void debugOutputVector(const char *,vector3f);

void prependVchWithSize(vch *vchDest);

void packVector2f(vch *destVch, const vector2f &v);
vchIter unpackVector2f(vchIter src, vector2f *v);

vchIter unpackTypecharFromIter(vchIter src, unsigned char *typechar);

void packEntityRef(vch *destVch, EntityRef ref);
vchIter unpackEntityRef(vchIter iter, EntityRef *ref);

void packStringToVch(std::vector<unsigned char> *vch, string s);
vchIter unpackStringFromIter(vchIter iter, uint16_t maxSize, string *s);

bool entityRefIsNull(EntityRef);
std::optional<unsigned int> safeUIntAdd(unsigned int, unsigned int);

coinsInt dollarsToCoinsInt(float dollars);
float coinsIntToDollars(coinsInt coins);

float degToRad(float);
float radToDeg(float);

vector2f randomVector(float magnitude);

#endif // COMMON_H