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
int8_t NetballUnspooler::consumeInt8_t()
{
    int8_t i = unpacki8(&(*(consumePos)));
    consumePos += 1;
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

int8_t unpacki8(unsigned char *buf)
{
        uint16_t i2 = ((uint16_t)buf[0]);
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