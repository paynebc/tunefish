/*
 ---------------------------------------------------------------------
 Tunefish 4  -  http://tunefish-synth.com
 ---------------------------------------------------------------------
 This file is part of Tunefish.

 Tunefish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Tunefish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Tunefish.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <xmmintrin.h>

// global constants (don't change into
// constants, it's a size thing!)

#define eSQRT2                  1.41421356237f
#define ePI                     3.1415926535897932384626433832795f
#define eTWOPI                  (ePI*2.0f)
#define eHALFPI                 (ePI*0.5f)
#define eSQRPI                  (ePI*ePI)
#define eINVHALFPI              (1.0f/eHALFPI)
#define eEXPONE                 2.718281828459f
#define eALMOST_ZERO            0.00001f
#define eMAX_RAND               2147483647
#define eMAX_PATH               260 // maximum length of a full path name

// function macros
#define eASSERT(x);
#define eELEMENT_COUNT(a)       (sizeof(a)/sizeof(a[0]))
#define eASSERT_ALIGNED16(x)    eASSERT(eU32(x)%16 == 0)
#define eCOALESCE(x, y)         (x == nullptr ? y : x)

#if defined(eRELEASE) && defined(ePLAYER)
ePtr eCDECL operator new(eU32 size);
ePtr eCDECL operator new[](eU32 size);
void eCDECL operator delete(ePtr ptr);
void eCDECL operator delete(ePtr ptr, unsigned int size);
void eCDECL operator delete[](ePtr ptr);
#endif

ePtr    eAllocAligned(eU32 size, eU32 alignment);
ePtr    eAllocAlignedAndZero(eU32 size, eU32 alignment);
void    eFreeAligned(ePtr ptr);
ePtr    eMemRealloc(ePtr ptr, eU32 oldLength, eU32 newLength);
void    eMemSet(ePtr dst, eU8 val, eU32 count);
void    eMemCopy(ePtr dst, eConstPtr src, eU32 count);
void    eMemMove(ePtr dst, eConstPtr src, eU32 count);
eBool   eMemEqual(eConstPtr mem0, eConstPtr mem1, eU32 count);
void    eStrClear(eChar *str);
void    eStrCopy(eChar *dst, const eChar *src);
eChar * eStrClone(const eChar *str);
eU32    eStrLength(const eChar *str);
eChar * eStrAppend(eChar *dst, const eChar *src);
eChar * eStrUpper(eChar *str);
eChar * eIntToStr(eInt val);
eChar * eFloatToStr(eF32 val);
eInt    eStrToInt(const eChar *str);
eF32    eStrToFloat(const eChar *str);
eBool   eIsAlphaNumeric(eChar c);
eBool   eIsDigit(eChar c);
eBool   eIsAlpha(eChar c);
eF32    eAbs(eF32 x);
eInt    eAbs(eInt x);
eF32    ePow(eF32 base, eF32 exp);
eF32    eSinH(eF32 x);
eF32    eCosH(eF32 x);
eF32    eTanH(eF32 x);
eF32    eASin(eF32 x);
eF32    eACos(eF32 x);
eF32    eExp(eF32 x);
eF32    eRoundUp(eF32 x);
eF32    eRoundDown(eF32 x);
eF32    eRoundZero(eF32 x);
eF32    eRoundNearest(eF32 x);
eU32    eRoundToMultiple(eU32 x, eU32 multiple);
eBool   eIsNumber(eF32 x);
eF32    eLog10(eF32 x);
eF32    eLog2(eF32 x);
eF32    eLogE(eF32 x);
eF32    eSin(eF32 x);
eF32    eCos(eF32 x);
eF32    eTan(eF32 x);
eF32    eCot(eF32 x);
eF32    eATan(eF32 x);
eF32    eATan2(eF32 y, eF32 x);
eF32    eATanh(eF32 x);
void    eSinCos(eF32 x, eF32 &sine, eF32 &cosine);
eF32    eSqrt(eF32 x);
eF32    eInvSqrt(eF32 x);
eF32    eMod(eF32 a, eF32 b);
eBool   eIsFloatZero(eF32 x, eF32 thresh=eALMOST_ZERO);
eBool   eAreFloatsEqual(eF32 x, eF32 y, eF32 thresh=eALMOST_ZERO);
eBool   eIsNan(eF32 x);
eF32    eDegToRad(eF32 degrees);
eF32    eRadToDeg(eF32 radians);
eBool   eIsAligned(eConstPtr data, eU32 alignment);

// negative safe modulo  -1 % 5 will return -1.  eNsMod(-1, 5) will return 4.
eFORCEINLINE eInt eNsMod(eInt x, eInt y)
{
    return ((x % y) + y) % y;
}

// inlineable functions

// faster float to long conversion than c-lib's
// default version. must be called explicitly.
eFORCEINLINE eInt eFtoL(eF32 x)
{
    return _mm_cvtt_ss2si(_mm_load_ss(&x));
}

eINLINE void eUndenormalise(eF32 &sample)
{
    if (((*(eU32 *)&sample)&0x7f800000) == 0)
        sample = 0.0f;
}

eINLINE eU16 eLoword(eU32 x)
{
    return (eU16)(x&0xffff);
}

eINLINE eU16 eHiword(eU32 x)
{
    return (eU16)((x>>16)&0xffff);
}

eINLINE eU8 eLobyte(eU16 x)
{
    return (eU8)(x&0xff);
}

eINLINE eU8 eHibyte(eU16 x)
{
    return (eU8)((x>>8)&0xff);
}

eINLINE eU32 eMakeDword(eU16 lo, eU16 hi)
{
    return (((eU32)lo)|(((eU32)hi)<<16));
}

eINLINE eU16 eMakeWord(eU8 lo, eU8 hi)
{
    return (((eU8)lo)|(((eU8)hi)<<8));
}

template<class T> T eSign(T x)
{
    return (x == T(0) ? T(0) : (x < T(0) ? T(-1) : T(1)));
}

template<class T> void eSetBit(T &x, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    x |= (1<<index);
}

template<class T> void eModifyBit(T &x, eU32 index, eBool set)
{
    // clever use of Tow's Complement for
    // obtaining either only zeros or ones:
    // set=0 => -set=0=b00...
    // set=1 => -set=-1=b11...
    const eU32 mask = 1<<index;
    x = (x&(~mask))|((-(eInt)set)&mask);
}

template<class T> void eClearBit(T &x, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    x &= ~(1<<index);
}

template<class T> eBool eGetBit(T x, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    return ((x&((T)1<<index)) != 0);
}

template<class T> void eToggleBit(T &x, eU32 index)
{
    eASSERT(index < sizeof(T)*8);
    x ^= (1<<index);
}

template<class T> void eDelete(T &ptr)
{
    delete ptr;
    ptr = nullptr;
}

template<class T> void eDeleteArray(T &ptr)
{
    delete [] ptr;
    ptr = nullptr;
}

template<class T> void eReleaseCom(T &ptr)
{
    if (ptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

template<class T> void eSwap(T &a, T &b)
{
    T c = a;
    a = b;
    b = c;
}

template<class T> T eMin(const T &a, const T &b)
{
    return (a < b) ? a : b;
}

template<class T> T eMax(const T &a, const T &b)
{
    return (a > b) ? a : b;
}

template<class T> T eClamp(const T &min, const T &x, const T &max)
{
    return (x < min ? min : (x > max ? max : x));
}

template<class T> T eLerp(const T &a, const T &b, eF32 t)
{
    return a+(b-a)*t;
}

template<class T> T eSqr(T x)
{
    return x*x;
}

template<class T> T eCubic(T x)
{
    return x*x*x;
}

template<class T> T eAlign(const T &val, eU64 alignment)
{
    static_assert(sizeof(val) <= sizeof(alignment), "doesn't work correctly if sizeof(alignment) < sizeof(val)");
    return (T)((((T)val)+alignment-1)&(~(alignment-1)));
}

template<class T> eBool eInRange(const T &x, const T &min, const T &max)
{
    return (x >= min && x <= max);
}

template<class TO, class FROM> TO eRawCast(FROM x)
{
    return *(TO *)&x;
}

template<class T> void eMemZero(T &val)
{
    eMemSet(&val, 0, sizeof(T));
}

#endif
