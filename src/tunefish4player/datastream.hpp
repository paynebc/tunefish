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

#ifndef DATA_STREAM_HPP
#define DATA_STREAM_HPP

#include "../tunefish4/Source/runtime/system.hpp"

// bit- and byte-wise buffer I/O
class eDataStream
{
public:
    eDataStream(eConstPtr data = nullptr, eU32 size = 0);
    eDataStream(const eByteArray &data);

    void        Attach(eConstPtr data, eU32 size);
    void        Flush();

    void        WriteBit(eBool bit, eU32 count = 1);
    void        WriteBits(eU32 val, eU32 bitCount);
    void        WriteU8(eU8 val);
    void        WriteU16(eU16 val);
    void        WriteU32(eU32 val);

    eBool       ReadBit();
    eBool       ReadBitOrZero();
    eU32        ReadBits(eU32 bitCount);
    eU8         ReadU8();
    eU16        ReadU16();
    eU32        ReadU32();

public:
    eByteArray  Data;
    eBool       Reading;
    eU32        ReadIdx;

private:
    eU32        NumBits;
    eU8         CurByte;
};

#endif