#include <vector>
#include <stdint.h>
#include <string>

#ifndef NETPACK_H
#define NETPACK_H

using namespace std;

namespace Netpack
{
    using vch = vector<unsigned char>;
    using vchIter = vch::iterator;

    class Builder
    {
        vch data;
    public:
        Builder();
        vch getVch() const;
        string getHexString();

        void packUint8_t(uint8_t);
        void packUint16_t(uint16_t);
        void packUint32_t(uint32_t);
        void packUint64_t(uint64_t);
        void packInt8_t(int8_t);
        void packInt16_t(int16_t);
        void packInt32_t(int32_t);
        void packInt64_t(int64_t);

        void packBool(bool);
        void packStringWithoutSize(string);
        void packStringWith16bitSize(string);

        void prependWith64bitSize();
    };

    class Consumer
    {
        vch data;
        vchIter consumePos;
    public:
        Consumer(vch data);
        Consumer(const Builder &);
        vch getVch() const;
        vchIter getCurrentIter() const;
        void debugOutput();

        uint8_t consumeUint8_t();
        uint16_t consumeUint16_t();
        uint32_t consumeUint32_t();
        uint64_t consumeUint64_t();
        int8_t consumeInt8_t();
        int16_t consumeInt16_t();
        int32_t consumeInt32_t();
        int64_t consumeInt64_t();

        bool consumeBool();
        string consumeStringGivenSize(size_t size);
        string consumeStringWith16bitSize();
    };

    void packi8(unsigned char *buf, uint8_t i);
    void packi16(unsigned char *buf, uint16_t i);
    void packi32(unsigned char *buf, uint32_t i);
    void packi64(unsigned char *buf, uint64_t i);

    int8_t unpacki8(unsigned char *buf);
    int16_t unpacki16(unsigned char *buf);
    int32_t unpacki32(unsigned char *buf);
    int64_t unpacki64(unsigned char *buf);

    uint8_t unpacku8(unsigned char *buf);
    uint16_t unpacku16(unsigned char *buf);
    uint32_t unpacku32(unsigned char *buf);
    uint64_t unpacku64(unsigned char *buf);
}

#endif // NETPACK_H
