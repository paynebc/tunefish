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

#include "datastream.hpp"

eDataStream::eDataStream(eConstPtr mem, eU32 length) :
    Reading(eFALSE),
    ReadIdx(0),
    NumBits(0),
    CurByte(0)
{
    Attach(mem, length);
}

eDataStream::eDataStream(const eByteArray &data) : eDataStream(data.isEmpty() ? nullptr : &data[0], data.size())
{
}

void eDataStream::Attach(eConstPtr data, eU32 size)
{
    if (data && size)
    {
        Data.resize(size);
        eMemCopy(&Data[0], data, size);
        Reading = eTRUE;
        NumBits = 8;
    }
}

void eDataStream::Flush()
{
    Data.append(CurByte);
}

void eDataStream::WriteBit(eBool bit, eU32 count)
{
    eASSERT(!Reading);

    for (eU32 i = 0; i<count; i++)
    {
        if (bit)
            eSetBit(CurByte, NumBits);

        NumBits++;

        if (NumBits == 8)
        {
            Data.append(CurByte);
            NumBits = 0;
            CurByte = 0;
        }
    }
}

void eDataStream::WriteBits(eU32 val, eU32 bitCount)
{
    eASSERT(bitCount <= 32);

    for (eU32 i = 0; i<bitCount; i++)
        WriteBit(eGetBit(val, i));
}

void eDataStream::WriteU8(eU8 val)
{
    WriteBits(val, 8);
}

void eDataStream::WriteU16(eU16 val)
{
    WriteBits(val, 16);
}

void eDataStream::WriteU32(eU32 val)
{
    WriteBits(val, 32);
}

eBool eDataStream::ReadBit()
{
    eASSERT(Reading);
    eASSERT(ReadIdx <= Data.size());

    if (NumBits == 8)
    {
        CurByte = Data[ReadIdx++];
        NumBits = 0;
    }

    return eGetBit(CurByte, NumBits++);
}

eBool eDataStream::ReadBitOrZero()
{
    return (ReadIdx >= Data.size() && NumBits >= 7 ? 0 : ReadBit());
}

eU32 eDataStream::ReadBits(eU32 bitCount)
{
    eU32 res = 0;

    for (eU32 i = 0; i<bitCount; i++)
        res |= ((eU32)ReadBit() << i);

    return res;
}

eU8 eDataStream::ReadU8()
{
    return (eU8)ReadBits(8);
}

eU16 eDataStream::ReadU16()
{
    return (eU16)ReadBits(16);
}

eU32 eDataStream::ReadU32()
{
    return (eU32)ReadBits(32);
}
