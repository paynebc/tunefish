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

#ifndef SIMD_HPP
#define SIMD_HPP

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#define eSimdSelect(v, i0, i1, i2, i3)              _mm_shuffle_ps(v, v, _MM_SHUFFLE(i0, i1, i2, i3))
#define eSimdShuffle(v0, v1, i00, i01, i10, i11)    _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(i00, i01, i10, i11))
#define eSimdBlend(v0, v1, i0, i1, i2, i3)          _mm_blend_ps(v0, v1, ((i0)|(i1)<<1|(i2)<<2|(i3)<<3))
#define eSimdTranspose(row0, row1, row2, row3)      _MM_TRANSPOSE4_PS(row0, row1, row2, row3)
#define eSimdZero()                                 _mm_setzero_ps()
#define eSimdLoad(vals)                             _mm_loadu_ps(vals)
#define eSimdLoadAligned(vals)                      _mm_load_ps(vals)
#define eSimdSetAll(val)                            _mm_set1_ps(val)
#define eSimdSet(val0, val1, val2, val3)            _mm_set_ps(val0, val1, val2, val3)
#define eSimdSet2(val0, val1)                       _mm_set_ps(val0, val1, 0.0f, 0.0f)
#define eSimdMul(v0, v1)                            _mm_mul_ps(v0, v1)
#define eSimdMulScalar(v, scalar)                   _mm_mul_ps(v, _mm_set1_ps(scalar))
#define eSimdDiv(v0, v1)                            _mm_div_ps(v0, v1)
#define eSimdAdd(v0, v1)                            _mm_add_ps(v0, v1)
#define eSimdAddScalar(v, scalar)                   _mm_add_ps(v, _mm_set1_ps(scalar))
#define eSimdSub(v0, v1)                            _mm_sub_ps(v0, v1)
#define eSimdSubScalar(v, scalar)                   _mm_sub_ps(v, _mm_set1_ps(scalar))
#define eSimdRcp(v)                                 _mm_rcp_ps(v) // reciprocal (returns 1/v)
#define eSimdSqrt(v)                                _mm_sqrt_ps(v)
#define eSimdMax(v0, v1)                            _mm_max_ps(v0, v1)
#define eSimdMin(v0, v1)                            _mm_min_ps(v0, v1)
#define eSimdAbs(v)                                 _mm_andnot_ps(v, _mm_castsi128_ps(_mm_set1_epi32(eSIMD_MSB1_REST0)))
#define eSimdNeg(v)                                 _mm_xor_ps(v, _mm_castsi128_ps(_mm_set1_epi32(eSIMD_MSB1_REST0)))
#define eSimdXor(v0, v1)                            _mm_xor_ps(v0, v1)
#define eSimdStore(v, buf)                          _mm_storeu_ps(buf, v)
#define eSimdStoreAligned(v, buf)                   _mm_store_ps(buf, v)
#define eSimdFma(add, mul0, mul1)                   _mm_add_ps(add, _mm_mul_ps(mul0, mul1)) // returns add+mul0*mul1
#define eSimdNfma(sub, mul0, mul1)                  _mm_sub_ps(sub, _mm_mul_ps(mul0, mul1)) // returns add-mul0*mul1
#define eSimdRSqrt(v)                               _mm_rsqrt_ps(v) // returns 1/sqrt(v)

#define eSimdLerp(v0, v1, t)                                        \
{                                                                   \
    eASSERT(t >= 0.0f && t <= 1.0f);                                \
    eSimdFma(eSimdSetAll(t), v1, eSimdNfma(eSimdSetAll(t), v0, v0)) \
}

#define eSimdStore2(v, v0, v1)      \
{                                   \
    eALIGN16 eF32 buf[4];           \
    _mm_store_ps(buf, v);           \
    v0 = buf[3];                    \
    v1 = buf[2];                    \
}

typedef __m128 eF32x2;
typedef __m128 eF32x4;

enum eSimdConsts
{
  eSIMD_MSB1_REST0 = 0x80000000, // 0b10000000 00000000 00000000 00000000
  eSIMD_SLOT0      = 0x00000001,
  eSIMD_SLOT1      = 0x00000002,
  eSIMD_SLOT2      = 0x00000004,
  eSIMD_SLOT3      = 0x00000008,
  eSIMD_SLOTALL    = eSIMD_SLOT0|eSIMD_SLOT1|eSIMD_SLOT2|eSIMD_SLOT3,
  eSIMD_1ST        = 0,
  eSIMD_2ND        = 1,
};

enum eSimdArithmeticFlags
{
    eSAF_FTZ =  1, // flush to zero
    eSAF_DAZ =  2, // denormals are zero
    eSAF_RP  =  4, // round positive
    eSAF_RN  =  8, // round negative
    eSAF_RTZ = 16, // round to zero
    eSAF_RTN = 32  // round to nearest (default)
};

void eSimdSetArithmeticFlags(eInt flags);

#endif
