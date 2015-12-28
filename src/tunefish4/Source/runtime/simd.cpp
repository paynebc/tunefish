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

#include "system.hpp"

// DZ   bit 6 = 1       denormals are zero
// FZ   bit 15 = 1      flush to zero
// R+   bit 14 = 1      round positive
// R-   bit 13 = 1      round negative
// RZ   bits 13&14 = 1  round to zero
// RN   bits 13&14 = 0  round to nearest
void eSimdSetArithmeticFlags(eInt flags)
{
    // default control register value
    // FTZ=off | RTN | EXC. on | DAZ=off | flags (volatile)
    // 0       | 00  | 111111  | 0       | 000000
    static const eU16 DEFAULT_FLAGS = 0x1f80;

    eU16 mxcsr = DEFAULT_FLAGS;
    eModifyBit(mxcsr, 15, (flags&eSAF_FTZ) != 0);
    eModifyBit(mxcsr,  6, (flags&eSAF_DAZ) != 0);
    eModifyBit(mxcsr, 13, (flags&eSAF_RN) || (flags&eSAF_RTZ));
    eModifyBit(mxcsr, 14, (flags&eSAF_RP) || (flags&eSAF_RTZ));
    _mm_setcsr(mxcsr);
}
