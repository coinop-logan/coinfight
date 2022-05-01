#include "netpack.h"

NetballSpooler::NetballSpooler()
    : data() {}

NetballUnspooler::NetballUnspooler(vch data)
    : data(data), consumePos(data.begin()) {}

NetballUnspooler::NetballUnspooler(const NetballSpooler &spooler)
    : data(spooler.getVch()), consumePos(data.begin()) {}

vch NetballSpooler::getVch() const
{
    return data;
}
vch NetballUnspooler::getVch() const
{
    return data;
}
vchIter NetballUnspooler::getCurrentIter() const
{
    return consumePos;
}

void NetballSpooler::pack(int8_t i)
{
    data.insert(data.end(), 1, 0);
    packi8(&(*(data.end() - 1)), i);
}
void NetballSpooler::pack(uint8_t i)
{
    data.insert(data.end(), 1, 0);
    packi8(&(*(data.end() - 1)), i);
}
void NetballSpooler::pack(int16_t i)
{
    data.insert(data.end(), 2, 0);
    packi16(&(*(data.end() - 2)), i);
}
void NetballSpooler::pack(uint16_t i)
{
    data.insert(data.end(), 2, 0);
    packi16(&(*(data.end() - 2)), i);
}
void NetballSpooler::pack(int32_t i)
{
    data.insert(data.end(), 4, 0);
    packi32(&(*(data.end() - 4)), i);
}
void NetballSpooler::pack(uint32_t i)
{
    data.insert(data.end(), 4, 0);
    packi32(&(*(data.end() - 4)), i);
}
void NetballSpooler::pack(int64_t i)
{
    data.insert(data.end(), 8, 0);
    packi64(&(*(data.end() - 8)), i);
}
void NetballSpooler::pack(uint64_t i)
{
    data.insert(data.end(), 8, 0);
    packi64(&(*(data.end() - 8)), i);
}

int8_t NetballUnspooler::consumeInt8_t()
{
    int8_t i = unpacki8(&(*(consumePos)));
    consumePos += 1;
    return i;
}
uint8_t NetballUnspooler::consumeUint8_t()
{
    uint8_t i = unpacku8(&(*(consumePos)));
    consumePos += 1;
    return i;
}
int16_t NetballUnspooler::consumeInt16_t()
{
    int16_t i = unpacki16(&(*(consumePos)));
    consumePos += 2;
    return i;
}
uint16_t NetballUnspooler::consumeUint16_t()
{
    uint16_t i = unpacku16(&(*(consumePos)));
    consumePos += 2;
    return i;
}
int32_t NetballUnspooler::consumeInt32_t()
{
    int32_t i = unpacki32(&(*(consumePos)));
    consumePos += 4;
    return i;
}
uint32_t NetballUnspooler::consumeUint32_t()
{
    uint32_t i = unpacku32(&(*(consumePos)));
    consumePos += 4;
    return i;
}
int64_t NetballUnspooler::consumeInt64_t()
{
    int64_t i = unpacki64(&(*(consumePos)));
    consumePos += 8;
    return i;
}
uint64_t NetballUnspooler::consumeUint64_t()
{
    uint64_t i = unpacku64(&(*(consumePos)));
    consumePos += 8;
    return i;
}



void packi8(unsigned char *buf, uint8_t i)
{
    *buf++ = i;
}

void packi16(unsigned char *buf, uint16_t i)
{
    *buf++ = i >> 8;
    *buf++ = i;
}

void packi32(unsigned char *buf, uint32_t i)
{
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

void packi64(unsigned char *buf, uint64_t i)
{
    *buf++ = i >> 56;
    *buf++ = i >> 48;
    *buf++ = i >> 40;
    *buf++ = i >> 32;
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

uint8_t unpacku8(unsigned char *buf)
{
    return (uint8_t)buf[0];
}

int8_t unpacki8(unsigned char *buf)
{
    uint8_t i2 = unpacku8(buf);
    int8_t i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fu)
    {
            i = i2;
    }
    else
    {
            i = -1 - (int8_t)(0xffu - i2);
    }

    return i;
}

uint16_t unpacku16(unsigned char *buf)
{
    return ((uint16_t)buf[0] << 8) |
           buf[1];
}

int16_t unpacki16(unsigned char *buf)
{
    int16_t i2 = unpacku16(buf);
    int16_t i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu)
    {
            i = i2;
    }
    else
    {
            i = -1 - (uint16_t)(0xffffu - i2);
    }

    return i;
}

uint32_t unpacku32(unsigned char *buf)
{
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8) |
           buf[3];
}

int32_t unpacki32(unsigned char *buf)
{
    uint32_t i2 = unpacku32(buf);
    int32_t i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu)
    {
            i = i2;
    }
    else
    {
            i = -1 - (int32_t)(0xffffffffu - i2);
    }

    return i;
}

uint64_t unpacku64(unsigned char *buf)
{
    return ((uint64_t)buf[0] << 56) |
           ((uint64_t)buf[1] << 48) |
           ((uint64_t)buf[2] << 40) |
           ((uint64_t)buf[3] << 32) |
           ((uint64_t)buf[4] << 24) |
           ((uint64_t)buf[5] << 16) |
           ((uint64_t)buf[6] << 8) |
           buf[7];
}

int64_t unpacki64(unsigned char *buf)
{
    uint64_t i2 = unpacku64(buf);
    int64_t i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu)
    {
            i = i2;
    }
    else
    {
            i = -1 - (int64_t)(0xffffffffffffffffu - i2);
    }

    return i;
}