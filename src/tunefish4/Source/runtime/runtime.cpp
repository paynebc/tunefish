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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <math.h>
#include <memory.h>

#include "types.hpp"
#include "runtime.hpp"

#pragma intrinsic(abs, sin, cos, tan, atan, atan2, sqrt, acos, asin, exp, pow)

#if defined(eRELEASE) && defined(ePLAYER)

ePtr eCDECL operator new(eU32 size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

ePtr eCDECL operator new [](eU32 size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void eCDECL operator delete(ePtr ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

void eCDECL operator delete(ePtr ptr, unsigned int size)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

void eCDECL operator delete [](ePtr ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

#endif

ePtr eAllocAligned(eU32 size, eU32 alignment)
{
    eASSERT(alignment > 0 && eIsPowerOf2(alignment));

    // alignment-1 because in worst case only that many
    // bytes to the next properly aligned address are
    // needed. e.g. new returns 49 and we want to align
    // to 16 bytes => 49+(16-1) = 64
    ePtr memPtr = new eU8[size + alignment - 1 + sizeof(eU64)];

    // find aligned memory address as multiples of alignment
    const eU64 memStartAddr = (eU64)memPtr + sizeof(eU64);
    const eU64 alignedAddr = eAlign<eU64>(memStartAddr, alignment);
    ePtr alignedPtr = (ePtr)alignedAddr;

    // store address returned by new directly before
    // pointer to be returned (useful for freeing)
    ((eU64 *)alignedPtr)[-1] = (eU64)memPtr;
    return alignedPtr;
}

ePtr eAllocAlignedAndZero(eU32 size, eU32 alignment)
{
    ePtr mem = eAllocAligned(size, alignment);
    eMemSet(mem, 0, size);
    return mem;
}

void eFreeAligned(ePtr ptr)
{
    if (ptr)
    {
        // find address of original allocation
        ePtr realPtr = (ePtr)(*((eU64 *)ptr-1));
        eDeleteArray(realPtr);
    }
}

ePtr eMemRealloc(ePtr ptr, eU32 oldLength, eU32 newLength)
{
    // reallocation necessary?
    if (newLength <= oldLength && ptr)
        return ptr;

    ePtr newPtr = new eU8[newLength];

    if (ptr)
    {
        eU8 *bptr = (eU8 *)ptr;
        eMemCopy(newPtr, bptr, oldLength);
        eDeleteArray(bptr);
    }

    return newPtr;
}

void eMemSet(ePtr dst, eU8 val, eU32 count)
{
#if defined(eRELEASE) && defined(ePLAYER)
  char *bytes = (char *)dst;
  while (count--)
  {
    *bytes++ = (char)val;
  }
#else
    memset(dst, val, count);
#endif
}

void eMemCopy(ePtr dst, eConstPtr src, eU32 count)
{
#if defined(eRELEASE) && defined(ePLAYER)
    char *dest8 = (char *)dst;
    const char *src8 = (const char *)src;
    while (count--)
    {
      *dest8++ = *src8++;
    }
#else
    memcpy(dst, src, count);
#endif
}

void eMemMove(ePtr dst, eConstPtr src, eU32 count)
{
    const eU8 *psrc = (const eU8 *)src;
    eU8 *pdst = (eU8 *)dst;

    if (dst <= src || pdst >= psrc+count)
    {
        // non-overlapping buffers, so copy from
        // lower addresses to higher addresses
        while (count--)
            *pdst++ = *psrc++;
    }
    else
    {
        // overlapping buffers, so copy from
        // higher addresses to lower addresses
        pdst = pdst+count-1;
        psrc = psrc+count-1;

        while (count--)
            *pdst-- = *psrc--;
    }
}

eBool eMemEqual(eConstPtr mem0, eConstPtr mem1, eU32 count)
{
    const eU8 *ptr0 = (eU8 *)mem0;
    const eU8 *ptr1 = (eU8 *)mem1;

    for (eU32 i=0; i<count; i++)
        if (ptr0[i] != ptr1[i])
            return eFALSE;

    return eTRUE;
}

void eStrClear(eChar *str)
{
    str[0] = '\0';
}

void eStrCopy(eChar *dst, const eChar *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
}

eChar * eStrClone(const eChar *str)
{
    eChar *clone = new eChar[eStrLength(str)+1];
    eStrCopy(clone, str);
    return clone;
}

eU32 eStrLength(const eChar *str)
{
    const eChar *eos = str;
    while (*eos++);
    return (eU32)(eos-str-1);
}

eChar * eStrAppend(eChar *dst, const eChar *src)
{
    // find end of destination string
    eChar *dstEnd = dst;
    while (*dstEnd)
        dstEnd++;

    // append source string
    eStrCopy(dstEnd, src);
    return dst;
}

eChar * eStrUpper(eChar *str)
{
    const eU32 strLen = eStrLength(str);

    for (eU32 i=0; i<strLen; i++)
    {
        eChar &c = str[i];
        if (c >= 'a' && c <= 'z')
            c -= 32;
    }

    return str;
}

eChar * eIntToStr(eInt val)
{
    // remember if integer is negative and
    // if it is, make it positive
    const eBool negative = (val < 0);

    if (negative)
        val = -val;

    // 12 spaces are enough for 32-bit decimal
    // (10 digits + 1 null terminator byte +
    // eventually a sign character)
    static eChar str[12];

    eChar *cp = str+sizeof(str)-1;
    *cp = '\0';

    do
    {
        *(--cp) = val%10+'0';
        val /= 10;
    }
    while (val > 0);

    // prepend negative sign character
    if (negative)
        *(--cp) = '-';

    return cp;
}

#if !defined(ePLAYER) || !defined(eRELEASE)
#include <sstream>

eChar * eFloatToStr(eF32 val)
{
    static eChar str[20];
    std::ostringstream ss;
    ss << val;
    eStrCopy(str, ss.str().c_str());
    return str;
}
#endif

eInt eStrToInt(const eChar *str)
{
    eASSERT(eStrLength(str) > 0);

    const eBool neg = (str[0] == '-');
    eChar c;
    eInt val = 0;

    while ((c = *str++) != '\0' && eIsDigit(c))
        val = val*10+(c-'0');

    return (neg ? -val : val);
}

eF32 eStrToFloat(const eChar *str)
{
    eASSERT(eStrLength(str) > 0);

    eF32 val = 0.0f;
    eInt sign = 1;

    if (*str == '+')
        str++;
    else if (*str == '-')
    {
        sign = -1;
        str++;
    }

    while (eIsDigit(*str))
        val = val*10+(*str++)-'0';

    if (*str == '.')
    {
        str++;
        eF32 dec = 1.0f;

        while (eIsDigit(*str))
        {
            dec = dec/10.0f;
            val = val+((*str++)-'0')*dec;
        }
    }

    return val*sign;
}

eBool eIsAlphaNumeric(eChar c)
{
    return eIsAlpha(c) || eIsDigit(c);
}

eBool eIsAlpha(eChar c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

eBool eIsDigit(eChar c)
{
    return (c >= '0' && c <= '9');
}

eF32 eAbs(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fabs
        fstp    dword ptr[x]
    }

    return x;
#else
    return fabs(x);
#endif
}

eInt eAbs(eInt x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fabs
        fstp    dword ptr[x]
    }

    return x;
#else
    return abs(x);
#endif
}

eF32 ePow(eF32 base, eF32 exp)
{
#ifdef ePLAYER
    return (eF32)ePow64(base, exp);
#else
    return powf(base, exp);
#endif
}

eF32 eSinH(eF32 x)
{
    const eF32 v = eExp(x);
    return 0.5f*(v-1.0f/v);
}

eF32 eCosH(eF32 x)
{
    const eF32 v = eExp(x);
    return 0.5f*(v+1.0f/v);
}

eF32 eTanH(eF32 x)
{
    const eF32 v = eExp(2.0f*x);
    return (v-1.0f)/(v+1.0f);
}

// arcus sine with -1 <= x <= 1
eF32 eASin(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fld     st(0)
        fmul
        fld     st(0)
        fld1
        fsubr
        fdiv
        fsqrt
        fld1
        fpatan
        fstp    dword ptr[x]
    }

    return x;
#else
    return asinf(x);
#endif
}

// arcus cosine with -1 <= x <= 1
eF32 eACos(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fld     st(0)
        fld     st(0)
        fmul
        fld1
        fsubr
        fsqrt
        fxch
        fpatan
        fstp    dword ptr[x]
    }

    return x;
#else
    return acosf(x);
#endif
}

eF32 eExp(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
      fld     dword ptr[x]
      fldl2e
      fmulp   st(1), st
      fld1
      fld     st(1)
      fprem
      f2xm1
      faddp   st(1), st
      fscale
      fstp    st(1)
      fstp    dword ptr[x]
    }

    return x;
#else
    return expf(x);
#endif
}

// rounds upVec towards +inf (e.g. ceil(-2.2) = -2)
eF32 eRoundUp(eF32 x)
{
#ifdef ePLAYER
    eInt holder, setter, res;

    __asm
    {
      fld     dword ptr[x]
      fnstcw  dword ptr[holder]
      movzx   eax, [holder]
      and eax, 0xfffff3ff
      or eax, 0x00000800
      mov     dword ptr[setter], eax
      fldcw   dword ptr[setter]
      fistp   dword ptr[res]
      fldcw   dword ptr[holder]
    }

    return static_cast<eF32>(res);
#else
    return ceilf(x);
#endif
}

// rounds down towards -inf (e.g. floor(-2.2) = -3)
eF32 eRoundDown(eF32 x)
{
#ifdef ePLAYER
    eInt holder, setter, res;

    __asm
    {
      fld     dword ptr[x]
      fnstcw  dword ptr[holder]
      movzx   eax, [holder]
      and eax, 0xfffff3ff
      or eax, 0x00000400
      mov     dword ptr[setter], eax
      fldcw   dword ptr[setter]
      fistp   dword ptr[res]
      fldcw   dword ptr[holder]
    }

    return static_cast<eF32>(res);
#else
    return floorf(x);
#endif
}

// rounds down for positive numbers,
// and rounds upVec for negative numbers
eF32 eRoundZero(eF32 x)
{
    return (x >= 0.0f ? eRoundDown(x) : eRoundUp(x));
}

eF32 eRoundNearest(eF32 x)
{
    return eRoundDown(x + 0.5f);
}

eU32 eRoundToMultiple(eU32 x, eU32 multiple)
{
    eASSERT(multiple > 0);
    const eU32 remainder = x%multiple;
    return (!remainder ? x : x+multiple-remainder);
}

eBool eIsNumber(eF32 x)
{
   return (!eIsNan(x)) && (x <= eF32_MAX && x >= -eF32_MAX);
}

eF32 eLog10(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld1
        fld     dword ptr[x]
        fyl2x
        fldl2t
        fdiv
        fstp    dword ptr[x]
    }

    return x;
#else
    return log10f(x);
#endif
}

eF32 eLog2(eF32 x)
{
    const eF32 log2 = 0.30102999566f;
    return eLog10(x) / log2;
}

eF32 eLogE(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld1
        fld     dword ptr[x]
        fyl2x
        fldl2e
        fdiv
        fstp    dword ptr[x]
    }

    return x;
#else
    return logf(x);
#endif
}

eF32 eSin(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fsin
        fstp    dword ptr[x]
    }

    return x;
#else
    return sin(x);
#endif
}

eF32 eCos(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fcos
        fstp    dword ptr[x]
    }

    return x;
#else
    return cos(x);
#endif
}

eF32 eTan(eF32 x)
{
    return static_cast<eF32>(tan(x));
}

eF32 eCot(eF32 x)
{
    eASSERT(!eIsFloatZero(eTan(x)));
    return 1.0f/eTan(x);
}

eF32 eATan(eF32 x)
{
    return static_cast<eF32>(atan(x));
}

eF32 eATan2(eF32 y, eF32 x)
{
    return static_cast<eF32>(atan2(y, x));
}

eF32 eATanh(eF32 x)
{
    return 0.5f*eLogE((1.0f+x)/(1.0f-x));
}

void eSinCos(eF32 x, eF32 &sine, eF32 &cosine)
{
    sine = eSin(x);
    cosine = eCos(x);
}

eF32 eSqrt(eF32 x)
{
#ifdef ePLAYER
    __asm
    {
        fld     dword ptr[x]
        fsqrt
        fstp    dword ptr[x]
    }

    return x;
#else
    return sqrtf(x);
#endif
}

eF32 eInvSqrt(eF32 x)
{
    eASSERT(x >= 0.0f);
    return 1.0f/eSqrt(x);
}

// returns the floating point remainder of a
// and b (so a=i*b+f, with f being returned
// and i is an integer value).
eF32 eMod(eF32 a, eF32 b)
{
#ifdef ePLAYER
    // "crude" implementation from https://stackoverflow.com/questions/26342823/implementation-of-fmod-function
    return (a - b * eRoundDown(a / b));
#else
    return fmodf(a, b);
#endif
}

eBool eIsFloatZero(eF32 x, eF32 thresh)
{
    return (eAbs(x) < thresh);
}

eBool eAreFloatsEqual(eF32 x, eF32 y, eF32 thresh)
{
    return eIsFloatZero(x-y, thresh);
}

eBool eIsNan(eF32 x)
{
   volatile eF32 temp = x; // avoid compiler optimization
   return (temp != x);
}

eF32 eDegToRad(eF32 degrees)
{
    return degrees/360.0f*eTWOPI;
}

eF32 eRadToDeg(eF32 radians)
{
    return radians/eTWOPI*360.0f;
}

// returns wether or not the given
// pointer is properly aligned
eBool eIsAligned(eConstPtr data, eU32 alignment)
{
    // check that the alignment is a power-of-two
    eASSERT((alignment&(alignment-1)) == 0);
    return (((eU64)data&(alignment-1)) == 0);
}

