#include <sstream>
#include <assert.h>
#include <cstring>
#include <iostream>
#include "netpack.h"

namespace Netpack
{
    string vchToHexNo0x(vch* data)
    {
        stringstream s;
        for (unsigned int i = 0; i < data->size(); i++)
        {
            s << hex << (unsigned int)((*data)[i]);
        }
        return s.str();
    }

    Builder::Builder(vch* data)
        : debugOutputEnabled(false), data(data) {}

    string Builder::getHexString()
    {
        stringstream s;
        s << "0x";
        for (unsigned int i = 0; i < data->size(); i++)
        {
            s << hex << (unsigned int)((*data)[i]);
        }
        return s.str();
    }

    void Builder::enableDebugOutput()
    {
        debugOutputEnabled = true;
    }
    template <typename V>
    void Builder::maybeDebugOutput(V val, size_t size)
    {
        if (debugOutputEnabled)
        {
            vch beforePlus(data->begin(), data->end() - size);
            vch afterPlus(data->end() - size, data->end());

            cout << val << " -> " << vchToHexNo0x(&beforePlus) << "+" << vchToHexNo0x(&afterPlus) << " (" << data->size() << ")" << endl;
        }
    }

    void Builder::packInt8_t(int8_t i)
    {
        data->insert(data->end(), 1, 0);
        packi8(&(*(data->end() - 1)), i);
        maybeDebugOutput((int)i, 1);
    }
    void Builder::packUint8_t(uint8_t i)
    {
        data->insert(data->end(), 1, 0);
        packi8(&(*(data->end() - 1)), i);
        maybeDebugOutput((int)i, 1);
    }
    void Builder::packInt16_t(int16_t i)
    {
        data->insert(data->end(), 2, 0);
        packi16(&(*(data->end() - 2)), i);
        maybeDebugOutput((int)i, 2);
    }
    void Builder::packUint16_t(uint16_t i)
    {
        data->insert(data->end(), 2, 0);
        packi16(&(*(data->end() - 2)), i);
        maybeDebugOutput((int)i, 2);
    }
    void Builder::packInt32_t(int32_t i)
    {
        data->insert(data->end(), 4, 0);
        packi32(&(*(data->end() - 4)), i);
        maybeDebugOutput((long int)i, 4);
    }
    void Builder::packUint32_t(uint32_t i)
    {
        data->insert(data->end(), 4, 0);
        packi32(&(*(data->end() - 4)), i);
        maybeDebugOutput((long int)i, 4);
    }
    void Builder::packInt64_t(int64_t i)
    {
        data->insert(data->end(), 8, 0);
        packi64(&(*(data->end() - 8)), i);
        maybeDebugOutput((long long int)i, 8);
    }
    void Builder::packUint64_t(uint64_t i)
    {
        data->insert(data->end(), 8, 0);
        packi64(&(*(data->end() - 8)), i);
        maybeDebugOutput((long long int)i, 8);
    }

    void Builder::packBool(bool flag)
    {
        packUint8_t(flag ? 1 : 0);
    }
    void Builder::packStringWithoutSize(string s)
    {
        size_t size = s.size();
        data->insert(data->end(), size, 0);
        memcpy(&(*(data->end() - size)), s.c_str(), size);
        maybeDebugOutput("s", size);
    }
    void Builder::packStringWith16bitSize(string s)
    {
        uint16_t strSize = s.size();
        assert(strSize == s.size()); // verify string size can fit in bits

        // bool reEnableDebug = debugOutputEnabled;
        // debugOutputEnabled = false;
        if (debugOutputEnabled)
        {
            cout << "s]";
        }
        packUint16_t(strSize);
        packStringWithoutSize(s);
    }

    void Builder::prependWith64bitSize()
    {
        vch sizeData;
        Builder sizePacket(&sizeData);
        if (debugOutputEnabled)
        {
            cout << "prepending with 64 bit size: " << data->size() << endl;
            sizePacket.enableDebugOutput();
        }
        sizePacket.packUint64_t(data->size());

        data->insert(data->begin(), sizeData.begin(), sizeData.end());
        if (debugOutputEnabled)
        {
            cout << "new data: " << vchToHexNo0x(data) << endl;
        }
    }
    void Builder::prependWith16bitSize()
    {
        vch sizeData;
        Builder sizePacket(&sizeData);
        if (debugOutputEnabled)
        {
            cout << "prepending with 16 bit size: " << data->size() << endl;
            sizePacket.enableDebugOutput();
        }
        sizePacket.packUint16_t(data->size());

        data->insert(data->begin(), sizeData.begin(), sizeData.end());
        if (debugOutputEnabled)
        {
            cout << "new data: " << vchToHexNo0x(data) << endl;
        }
    }



    Consumer::Consumer(vchIter consumePos)
        : consumePos(consumePos), debugOutputEnabled(false) {}

    Consumer::Consumer(const Builder &builder)
        : consumePos(builder.data->begin()), debugOutputEnabled(false) {}
    
    vchIter Consumer::getCurrentIter() const
    {
        return consumePos;
    }
    void Consumer::enableDebugOutput()
    {
        debugOutputEnabled = true;
    }
    template<typename V>
    void Consumer::maybeDebugOutput(V val, size_t size)
    {
        if (debugOutputEnabled)
        {
            vch consumed(consumePos - size, consumePos);
            cout << val << " <- " << vchToHexNo0x(&consumed) << " (" << size << ")" << endl;
        }
    }

    int8_t Consumer::consumeInt8_t()
    {
        int8_t i = unpacki8(&(*(consumePos)));
        consumePos += 1;
        maybeDebugOutput((int)i, 1);
        return i;
    }
    uint8_t Consumer::consumeUint8_t()
    {
        uint8_t i = unpacku8(&(*(consumePos)));
        consumePos += 1;
        maybeDebugOutput((int)i, 1);
        return i;
    }
    int16_t Consumer::consumeInt16_t()
    {
        int16_t i = unpacki16(&(*(consumePos)));
        consumePos += 2;
        maybeDebugOutput((int)i, 2);
        return i;
    }
    uint16_t Consumer::consumeUint16_t()
    {
        uint16_t i = unpacku16(&(*(consumePos)));
        consumePos += 2;
        maybeDebugOutput((int)i, 2);
        return i;
    }
    int32_t Consumer::consumeInt32_t()
    {
        int32_t i = unpacki32(&(*(consumePos)));
        consumePos += 4;
        maybeDebugOutput((long int)i, 4);
        return i;
    }
    uint32_t Consumer::consumeUint32_t()
    {
        uint32_t i = unpacku32(&(*(consumePos)));
        consumePos += 4;
        maybeDebugOutput((long int)i, 4);
        return i;
    }
    int64_t Consumer::consumeInt64_t()
    {
        int64_t i = unpacki64(&(*(consumePos)));
        consumePos += 8;
        maybeDebugOutput((long int)i, 8);
        return i;
    }
    uint64_t Consumer::consumeUint64_t()
    {
        uint64_t i = unpacku64(&(*(consumePos)));
        consumePos += 8;
        maybeDebugOutput((long int)i, 8);
        return i;
    }

    bool Consumer::consumeBool()
    {
        return (bool)(consumeUint8_t());
    }
    string Consumer::consumeStringGivenSize(size_t size)
    {
        string s(&(*consumePos), &(*(consumePos + size)));

        if (debugOutputEnabled)
        {
            vch strData(&(*consumePos), &(*(consumePos + size)));
            cout << "s" << " <- " << vchToHexNo0x(&strData) << endl;
        }

        consumePos += size;
        
        return s;
    }
    string Consumer::consumeStringWith16bitSize()
    {
        if (debugOutputEnabled)
        {
            cout << "s]";
        }
        uint16_t size = consumeUint16_t();
        return consumeStringGivenSize(size);
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
        uint16_t i2 = unpacku16(buf);
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
}