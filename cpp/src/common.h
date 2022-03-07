#include <stdint.h>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "myvectors.h"
#include "vchpack.h"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

const glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 10000.0f);

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using EntityRef = uint16_t;

void packTypechar(vch *dest, unsigned char typechar);

void debugOutputVch(vch);
void debugOutputVector(const char *,vector2f);
void debugOutputVector(const char *,vector3f);
void debugOutputVector(const char *,glm::vec3);

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

glm::vec3 toGlmVec3(vector3f v);

struct BalanceUpdate
{
    string userAddress;
    coinsInt amount;
    bool isDeposit;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    BalanceUpdate(string userAddress, coinsInt amount, bool isDeposit);
    BalanceUpdate(vchIter *iter);
};

#endif // COMMON_H