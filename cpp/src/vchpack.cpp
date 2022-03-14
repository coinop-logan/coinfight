#include "vchpack.h"
#include <string>
#include <vector>

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/
unsigned long long int pack754(long double f, unsigned bits, unsigned expbits)
{
        long double fnorm;
        int shift;
        long long sign, exp, significand;
        unsigned significandbits = bits - expbits - 1; // -1 for sign bit

        if (f == 0.0)
                return 0; // get this special case out of the way

        // check sign and begin normalization
        if (f < 0)
        {
                sign = 1;
                fnorm = -f;
        }
        else
        {
                sign = 0;
                fnorm = f;
        }

        // get the normalized form of f and track the exponent
        shift = 0;
        while (fnorm >= 2.0)
        {
                fnorm /= 2.0;
                shift++;
        }
        while (fnorm < 1.0)
        {
                fnorm *= 2.0;
                shift--;
        }
        fnorm = fnorm - 1.0;

        // calculate the binary form (non-float) of the significand data
        significand = fnorm * ((1LL << significandbits) + 0.5f);

        // get the biased exponent
        exp = shift + ((1 << (expbits - 1)) - 1); // shift + bias

        // return the final answer
        return (sign << (bits - 1)) | (exp << (bits - expbits - 1)) | significand;
}

/*
** unpack754() -- unpack a floating point number from IEEE-754 format
*/
long double unpack754(unsigned long long int i, unsigned bits, unsigned expbits)
{
        long double result;
        long long shift;
        unsigned bias;
        unsigned significandbits = bits - expbits - 1; // -1 for sign bit

        if (i == 0)
                return 0.0;

        // pull the significand
        result = (i & ((1LL << significandbits) - 1)); // mask
        result /= (1LL << significandbits);            // convert back to float
        result += 1.0f;                                // add the one back on

        // deal with the exponent
        bias = (1 << (expbits - 1)) - 1;
        shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;
        while (shift > 0)
        {
                result *= 2.0;
                shift--;
        }
        while (shift < 0)
        {
                result /= 2.0;
                shift++;
        }

        // sign it
        result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;

        return result;
}

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/
void packi16(unsigned char *buf, unsigned int i)
{
        *buf++ = i >> 8;
        *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
void packi32(unsigned char *buf, unsigned long int i)
{
        *buf++ = i >> 24;
        *buf++ = i >> 16;
        *buf++ = i >> 8;
        *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/
void packi64(unsigned char *buf, unsigned long long int i)
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

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/
short int unpacki16(unsigned char *buf)
{
        unsigned int i2 = ((unsigned int)buf[0] << 8) | buf[1];
        int i;

        // change unsigned numbers to signed
        if (i2 <= 0x7fffu)
        {
                i = i2;
        }
        else
        {
                i = -1 - (unsigned int)(0xffffu - i2);
        }

        return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs())
*/
unsigned short int unpacku16(unsigned char *buf)
{
        return ((unsigned int)buf[0] << 8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/
long int unpacki32(unsigned char *buf)
{
        unsigned long int i2 = ((unsigned long int)buf[0] << 24) |
                               ((unsigned long int)buf[1] << 16) |
                               ((unsigned long int)buf[2] << 8) |
                               buf[3];
        long int i;

        // change unsigned numbers to signed
        if (i2 <= 0x7fffffffu)
        {
                i = i2;
        }
        else
        {
                i = -1 - (long int)(0xffffffffu - i2);
        }

        return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl())
*/
unsigned long int unpacku32(unsigned char *buf)
{
        return ((unsigned long int)buf[0] << 24) |
               ((unsigned long int)buf[1] << 16) |
               ((unsigned long int)buf[2] << 8) |
               buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/
long long int unpacki64(unsigned char *buf)
{
        unsigned long long int i2 = ((unsigned long long int)buf[0] << 56) |
                                    ((unsigned long long int)buf[1] << 48) |
                                    ((unsigned long long int)buf[2] << 40) |
                                    ((unsigned long long int)buf[3] << 32) |
                                    ((unsigned long long int)buf[4] << 24) |
                                    ((unsigned long long int)buf[5] << 16) |
                                    ((unsigned long long int)buf[6] << 8) |
                                    buf[7];
        long long int i;

        // change unsigned numbers to signed
        if (i2 <= 0x7fffffffffffffffu)
        {
                i = i2;
        }
        else
        {
                i = -1 - (long long int)(0xffffffffffffffffu - i2);
        }

        return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl())
*/
unsigned long long int unpacku64(unsigned char *buf)
{
        return ((unsigned long long int)buf[0] << 56) |
               ((unsigned long long int)buf[1] << 48) |
               ((unsigned long long int)buf[2] << 40) |
               ((unsigned long long int)buf[3] << 32) |
               ((unsigned long long int)buf[4] << 24) |
               ((unsigned long long int)buf[5] << 16) |
               ((unsigned long long int)buf[6] << 8) |
               buf[7];
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C         
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16-bit unsigned length is automatically prepended to strings)
*/

void packToVch(std::vector<unsigned char> *vch, const char *format, ...)
{
        va_list ap;

        signed char c; // 8-bit
        unsigned char C;

        int h; // 16-bit
        unsigned int H;

        long int l; // 32-bit
        unsigned long int L;

        long long int q; // 64-bit
        unsigned long long int Q;

        float f; // floats
        double d;
        long double g;
        unsigned long long int fhold;

        char *s; // strings
        unsigned int len;

        va_start(ap, format);

        for (; *format != '\0'; format++)
        {
                switch (*format)
                {
                case 'B':                                   // treat as unsigned char
                        C = (unsigned char)va_arg(ap, int); // promoted
                        vch->push_back(C);
                        break;

                case 'c':                                 // 8-bit
                        c = (signed char)va_arg(ap, int); // promoted
                        assert(c >= PACK_SC_MIN && c <= PACK_SC_MAX);
                        vch->push_back(c);
                        break;

                case 'C':                                            // 8-bit unsigned
                        C = (unsigned char)va_arg(ap, unsigned int); // promoted
                        assert(C >= PACK_UC_MIN && C <= PACK_UC_MAX);
                        vch->push_back(C);
                        break;

                case 'h': // 16-bit
                        h = va_arg(ap, int);
                        assert(h >= PACK_SH_MIN && h <= PACK_SH_MAX);

                        vch->insert(vch->end(), PACK_H_SIZE, 0);
                        packi16(&(*(vch->end() - PACK_H_SIZE)), h);

                        break;

                case 'H': // 16-bit unsigned
                        H = va_arg(ap, unsigned int);
                        assert(H >= PACK_UH_MIN && H <= PACK_UH_MAX);

                        vch->insert(vch->end(), PACK_H_SIZE, 0);
                        packi16(&(*(vch->end() - PACK_H_SIZE)), H);

                        break;

                case 'l': // 32-bit
                        l = va_arg(ap, long int);
                        assert(l >= PACK_SL_MIN && l <= PACK_SL_MAX);

                        vch->insert(vch->end(), PACK_L_SIZE, 0);
                        packi32(&(*(vch->end() - PACK_L_SIZE)), l);

                        break;

                case 'L': // 32-bit unsigned
                        L = va_arg(ap, unsigned long int);
                        assert(L >= PACK_UL_MIN && L <= PACK_UL_MAX);

                        vch->insert(vch->end(), PACK_L_SIZE, 0);
                        packi32(&(*(vch->end() - PACK_L_SIZE)), L);

                        break;

                case 'q': // 64-bit
                        q = va_arg(ap, long long int);
                        assert(q >= PACK_SQ_MIN && q <= PACK_SQ_MAX);

                        vch->insert(vch->end(), PACK_Q_SIZE, 0);
                        packi64(&(*(vch->end() - PACK_Q_SIZE)), q);

                        break;

                case 'Q': // 64-bit unsigned
                        Q = va_arg(ap, unsigned long long int);
                        assert(Q >= PACK_UQ_MIN && Q <= PACK_UQ_MAX);

                        vch->insert(vch->end(), PACK_Q_SIZE, 0);
                        packi64(&(*(vch->end() - PACK_Q_SIZE)), Q);

                        break;

                case 'f':                              // float-16
                        f = (float)va_arg(ap, double); // promoted
                        fhold = pack754_16(f);         // convert to IEEE 754

                        vch->insert(vch->end(), 2, 0);
                        packi16(&(*(vch->end() - 2)), fhold);

                        break;

                case 'd': // float-32
                        d = va_arg(ap, double);
                        fhold = pack754_32(d); // convert to IEEE 754

                        vch->insert(vch->end(), 4, 0);
                        packi32(&(*(vch->end() - 4)), fhold);

                        break;

                case 'g': // float-64
                        g = va_arg(ap, long double);
                        fhold = pack754_64(g); // convert to IEEE 754

                        vch->insert(vch->end(), 8, 0);
                        packi64(&(*(vch->end() - 8)), fhold);

                        break;

                case 's': // string
                        s = va_arg(ap, char *);
                        len = strlen(s);
                        unsigned int packSize = len + 2;
                        assert(packSize - 2 == len); // check for overflow

                        vch->insert(vch->end(), packSize, 0);
                        packi16(&(*(vch->end() - packSize)), len);

                        memcpy(&(*(vch->end() - len)), s, len);

                        break;
                }
        }

        va_end(ap);
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C         
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
std::vector<unsigned char>::iterator unpackFromIter(std::vector<unsigned char>::iterator iter, const char *format, ...)
{
        va_list ap;

        bool *B;

        signed char *c; // 8-bit
        unsigned char *C;

        short int *h; // 16-bit
        unsigned short int *H;

        long int *l; // 32-bit
        unsigned long int *L;

        long long int *q; // 64-bit
        unsigned long long int *Q;

        float *f; // floats
        double *d;
        long double *g;
        unsigned long long int fhold;

        char *s;
        unsigned int len, maxstrlen = 0, count;

        unsigned int size = 0;

        va_start(ap, format);

        for (; *format != '\0'; format++)
        {
                switch (*format)
                {
                case 'B': //treat as unsigned char
                        size += PACK_B_SIZE;
                        B = va_arg(ap, bool *);
                        *B = *iter++;
                        break;

                case 'c': // 8-bit
                        size += PACK_C_SIZE;
                        c = va_arg(ap, signed char *);
                        if (*iter <= 0x7f)
                        {
                                *c = *iter++;
                        } // re-sign
                        else
                        {
                                *c = -1 - (unsigned char)(0xffu - *iter);
                        }
                        break;

                case 'C': // 8-bit unsigned
                        size += PACK_C_SIZE;
                        C = va_arg(ap, unsigned char *);
                        *C = *iter++;
                        break;

                case 'h': // 16-bit
                        size += PACK_H_SIZE;
                        h = va_arg(ap, short int *);
                        *h = unpacki16(&(*iter));
                        iter += PACK_H_SIZE;
                        break;

                case 'H': // 16-bit unsigned
                        size += PACK_H_SIZE;
                        H = va_arg(ap, unsigned short int *);
                        *H = unpacku16(&(*iter));
                        iter += PACK_H_SIZE;
                        break;

                case 'l': // 32-bit
                        size += PACK_L_SIZE;
                        l = va_arg(ap, long int *);
                        *l = unpacki32(&(*iter));
                        iter += PACK_L_SIZE;
                        break;

                case 'L': // 32-bit unsigned
                        size += PACK_L_SIZE;
                        L = va_arg(ap, unsigned long int *);
                        *L = unpacku32(&(*iter));
                        iter += PACK_L_SIZE;
                        break;

                case 'q': // 64-bit
                        size += PACK_Q_SIZE;
                        q = va_arg(ap, long long int *);
                        *q = unpacki64(&(*iter));
                        iter += PACK_Q_SIZE;
                        break;

                case 'Q': // 64-bit unsigned
                        size += PACK_Q_SIZE;
                        Q = va_arg(ap, unsigned long long int *);
                        *Q = unpacku64(&(*iter));
                        iter += PACK_Q_SIZE;
                        break;

                case 'f': // float
                        size += 2;
                        f = va_arg(ap, float *);
                        fhold = unpacku16(&(*iter));
                        *f = unpack754_16(fhold);
                        iter += 2;
                        break;

                case 'd': // float-32
                        size += 4;
                        d = va_arg(ap, double *);
                        fhold = unpacku32(&(*iter));
                        *d = unpack754_32(fhold);
                        iter += 4;
                        break;

                case 'g': // float-64
                        size += 8;
                        g = va_arg(ap, long double *);
                        fhold = unpacku64(&(*iter));
                        *g = unpack754_64(fhold);
                        iter += 8;
                        break;

                case 's': // string
                        s = va_arg(ap, char *);
                        len = unpacku16(&(*iter));
                        size += len + 2;

                        iter += 2;
                        if (maxstrlen > 0 && len > maxstrlen)
                                count = maxstrlen - 1;
                        else
                                count = len;
                        memcpy(s, &(*iter), count);
                        s[count] = '\0';
                        iter += len;
                        break;

                default:
                        if (isdigit(*format))
                        { // track max str len
                                maxstrlen = maxstrlen * 10 + (*format - '0');
                        }
                }

                if (!isdigit(*format))
                        maxstrlen = 0;
        }

        va_end(ap);

        return iter;
}