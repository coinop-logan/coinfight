#include "myvectors.h"
#include "vchpack.h"
#include <stdint.h>
#include <optional>

#ifndef COMMON_H
#define COMMON_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using EntityRef = uint16_t;

void packTypechar(vch *dest, unsigned char typechar);

void debugOutputVch(vch);

void prependVchWithSize(vch *vchDest);

void packVector2f(vch *destVch, const vector2f &v);
vchIter unpackVector2f(vchIter src, vector2f *v);

vchIter unpackTypecharFromIter(vchIter src, unsigned char *typechar);

void packEntityRef(vch *destVch, EntityRef id);
vchIter unpackEntityRef(vchIter iter, EntityRef *id);

bool entityRefIsNull(EntityRef);
std::optional<unsigned int> safeUIntAdd(unsigned int, unsigned int);

#endif // COMMON_H