#include <vector>
#include <stdint.h>

#ifndef NETPACK_H
#define NETPACK_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vch::iterator;

class NetballSpooler
{
    vch data;
public:
    NetballSpooler();
    vch getVch() const;

    void pack(uint8_t);
    void pack(uint16_t);
    void pack(uint32_t);
    void pack(uint64_t);
    void pack(int8_t);
    void pack(int16_t);
    void pack(int32_t);
    void pack(int64_t);
};

class NetballUnspooler
{
    vch data;
    vchIter consumePos;
public:
    NetballUnspooler(vch data);
    NetballUnspooler(const NetballSpooler &);
    vch getVch() const;
    vchIter getCurrentIter() const;

    uint8_t consumeUint8_t();
    uint16_t consumeUint16_t();
    uint32_t consumeUint32_t();
    uint64_t consumeUint64_t();
    int8_t consumeInt8_t();
    int16_t consumeInt16_t();
    int32_t consumeInt32_t();
    int64_t consumeInt64_t();
};

void packi8(unsigned char *buf, uint8_t i);
void packi16(unsigned char *buf, uint16_t i);
void packi32(unsigned char *buf, uint32_t i);
void packi64(unsigned char *buf, uint64_t i);

int8_t unpacki8(unsigned char *buf);

#endif // NETPACK_H