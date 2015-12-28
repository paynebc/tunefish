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

void eArrayInit(ePtrArray *a, eU32 typeSize, eU32 size)
{
    eASSERT(typeSize > 0);

    a->m_data = nullptr;
    a->m_size = 0;
    a->m_capacity = 0;
    a->m_typeSize = typeSize;

    if (size > 0)
        eArrayResize(a, size);
}

void eArrayCopy(ePtrArray *a, const ePtrArray *ta)
{
    a->m_typeSize = ta->m_typeSize;
    eArrayReserve(a, ta->m_capacity);
    a->m_size = ta->m_size;

    if (ta->m_size)
        eMemCopy(a->m_data, ta->m_data, ta->m_size*ta->m_typeSize);
}

void eArrayClear(ePtrArray *a)
{
    a->m_size = 0;
}

void eArrayFree(ePtrArray *a)
{
    eFreeAligned(a->m_data);
    a->m_data = nullptr;
    a->m_size = 0;
    a->m_capacity = 0;
}

void eArrayReserve(ePtrArray *a, eU32 capacity)
{
    if (!capacity)
        eArrayClear(a);
    else if (a->m_capacity < capacity)
    {
        eU8 *temp = (eU8 *)eAllocAligned(capacity*a->m_typeSize, 16);
        eASSERT(temp);
        eU32 newSize = 0;

        if (a->m_data)
        {
            newSize = eMin(a->m_size, capacity);
            eMemCopy(temp, a->m_data, newSize*a->m_typeSize);
            eFreeAligned(a->m_data);
            a->m_data = nullptr;
        }

        a->m_data = (ePtr *)temp;
        a->m_size = newSize;
        a->m_capacity = capacity;
    }
}

void eArrayResize(ePtrArray *a, eU32 size)
{
    if (size > a->m_capacity)
        eArrayReserve(a, size);

    a->m_size = size;
}

void eArrayReverse(ePtrArray *a)
{
	const eU32 halfSize = a->m_size/2;
	eU8 temp[8];

	for (eU32 i=0; i<halfSize; i++)
	{
		eU8 *pos0 = ((eU8 *)a->m_data)+i*a->m_typeSize;
		eU8 *pos1 = ((eU8 *)a->m_data)+(a->m_size-i-1)*a->m_typeSize;

		eMemCopy(temp, pos0, a->m_typeSize);
		eMemCopy(pos0, pos1, a->m_typeSize);
		eMemCopy(pos1, temp, a->m_typeSize);
	}
}

ePtr eArrayAppend(ePtrArray *a)
{
    if (a->m_size >= a->m_capacity)
    {
        const eU32 newCapacity = (a->m_capacity > 0 ? a->m_capacity*2 : 32);
        eArrayReserve(a, newCapacity);
    }

    ePtr res = ((eU8 *)a->m_data)+a->m_size*a->m_typeSize;
    a->m_size++;
    return res;
}

void eArrayInsert(ePtrArray *a, eU32 index, const ePtr data)
{
    eASSERT(index <= a->m_size);

    if (a->m_size >= a->m_capacity)
    {
        const eU32 newCapacity = (a->m_capacity > 0 ? a->m_capacity*2 : 32);
        eArrayReserve(a, newCapacity);
    }

    eMemMove(((eU8 *)a->m_data)+(index+1)*a->m_typeSize,
             ((eU8 *)a->m_data)+index*a->m_typeSize,
             (a->m_size-index)*a->m_typeSize);

    eMemCopy(((eU8 *)a->m_data)+index*a->m_typeSize, data, a->m_typeSize);
    a->m_size++;
}

void eArrayRemoveAt(ePtrArray *a, eU32 index)
{
    eASSERT(index < a->m_size);

    eMemMove(((eU8 *)a->m_data)+index*a->m_typeSize,
             ((eU8 *)a->m_data)+(index+1)*a->m_typeSize,
             (a->m_size-index-1)*a->m_typeSize);

    a->m_size--;
}

void eArrayRemoveSwap(ePtrArray *a, eU32 index)
{
    eASSERT(index < a->m_size);

    if (index < a->m_size-1)
    {
        eMemCopy(((eU8 *)a->m_data)+index*a->m_typeSize,
                 ((eU8 *)a->m_data)+(a->m_size-1)*a->m_typeSize,
                 a->m_typeSize);
    }

    a->m_size--;
}

eInt eArrayFind(const ePtrArray *a, const ePtr data)
{
    for (eU32 i=0, index=0; i<a->size(); i++, index+=a->m_typeSize)
        if (eMemEqual(((eU8 *)a->m_data)+index, data, a->m_typeSize))
            return i;

    return -1;
}

eBool eArrayEqual(const ePtrArray *a0, const ePtrArray *a1)
{
    if (a0->size() != a1->size())
        return eFALSE;
    else if (a0->m_size == 0) // => both empty because of first condition
        return eTRUE;
    else
        return eMemEqual(a0->m_data, a1->m_data, a0->size()*a0->m_typeSize);
}
