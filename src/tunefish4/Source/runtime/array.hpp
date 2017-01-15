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

#ifndef ARRAY_HPP
#define ARRAY_HPP

template<class T> class eArray;
typedef eArray<ePtr> ePtrArray;
typedef eArray<eU8> eByteArray;

// non-templated functions used to avoid code
// bloat, caused by heavy template instantiation
void eArrayInit(ePtrArray *a, eU32 typeSize, eU32 size);
void eArrayCopy(ePtrArray *a, const ePtrArray *ta);
void eArrayClear(ePtrArray *a);
void eArrayFree(ePtrArray *a);
void eArrayReserve(ePtrArray *a, eU32 capacity);
void eArrayResize(ePtrArray *a, eU32 size);
ePtr eArrayAppend(ePtrArray *a);
void eArrayInsert(ePtrArray *a, eU32 index, const ePtr data);
void eArrayRemoveAt(ePtrArray *a, eU32 index);
void eArrayRemoveSwap(ePtrArray *a, eU32 index);
eInt eArrayFind(const ePtrArray *a, const ePtr data);
eBool eArrayEqual(const ePtrArray *a0, const ePtrArray *a1);
void eArrayReverse(ePtrArray *a);

// performs insertion sort. not really fast,
// but already a lot faster than bubble sort
// and quite small, easy to implement and easy
// to get correct.
template<class T> void eSort(T *data, eU32 count, eBool (*predicate)(const T &a, const T &b))
{


//  quickSort
//
//  This public-domain C implementation by Darel Rex Finley.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7


  #define  QUICKSORT_MAX_LEVELS  300
  T piv;
  eS32 beg[QUICKSORT_MAX_LEVELS], end[QUICKSORT_MAX_LEVELS], i=0, L, R, swap;

  eRandom Rand;
  beg[0]=0; end[0]=count;
  while (i>=0) {
	eASSERT(i < QUICKSORT_MAX_LEVELS);
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      // original:
      //piv=data[L];
      // modified: pick random element and "exchange" with data[L]
      eU32 PivIdx = Rand.NextInt(L, R);//L + Rand.nextInt() % (R - L + 1);
      piv=data[PivIdx];
      data[PivIdx] = data[L];

      while (L<R) {
        while (L<R && (predicate(data[R],piv))) R--; if (L<R) data[L++]=data[R];
        while (L<R && (predicate(piv,data[L]))) L++; if (L<R) data[R--]=data[L];
	  }
      data[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
      if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
        swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
        swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
	  }
	} else i--;
  }

/*
    for (eU32 j=1; j<count; j++)
    {
        const T key = data[j]; // don't use a reference here!
        eInt i = (eInt)j-1;

        while (i >= 0 && predicate(data[i], key))
        {
            data[i+1] = data[i];
            i--;
        }

        data[i+1] = key;
    }
*/
}

template<class T> void eMerge(T *arr0, eU32 elements0, T *arr1, eU32 elements1, T* arrFinal, eBool (*predicate)(const T &a, const T &b)) {
	if((elements0 > 0) && (elements1 > 0)) {
		// perform merge
		for(;;) {
			if(!predicate(*arr0, *arr1)) {
				// use first
				*arrFinal = *arr0;
				arrFinal++;
				elements0--;
				if(elements0 == 0)
					break;
				arr0++;
			} else {
				// use second
				*arrFinal = *arr1;
				arrFinal++;
				elements1--;
				if(elements1 == 0)
					break;
				arr1++;
			}
		}
	}
	// append remaining
	// append first
	for(eU32 i = 0; i < elements0; i++) {
		*arrFinal = *arr0;
		arrFinal++;
		arr0++;
	}
	// append second
	for(eU32 i = 0; i < elements1; i++) {
		*arrFinal = *arr1;
		arrFinal++;
		arr1++;
	}
}


// templated dynamic array. this class is intro-safe,
// because all array functions which are duplicated
// during template instantiation are inlined, using
// non-templated functions. works for all kind of
// data types, not only pointers.
// one drawback is that object's (T) constructors
// aren't called when copying or instantiating array,
// same goes for destructors.
// all memory is for SIMD usage 16-byte aligned.
template<class T> class eArray
{
public:
    eFORCEINLINE eArray(eU32 size=0)
    {
        eArrayInit((ePtrArray *)this, sizeof(T), size);
    }

    eFORCEINLINE eArray(const eArray &a)
    {
        eArrayInit((ePtrArray *)this, sizeof(T), 0);
        eArrayCopy((ePtrArray *)this, (const ePtrArray *)&a);
    }

	eFORCEINLINE eArray(const eU8 *data, eU32 len)
	{
		eArrayInit((ePtrArray *)this, sizeof(eU8), len);
		eMemCopy(m_data, data, len);
	}

    eFORCEINLINE ~eArray()
    {
        free();
    }

    eFORCEINLINE eBool isEmpty() const
    {
        return (m_size == 0);
    }

    eFORCEINLINE void resize(eU32 size)
    {
        eArrayResize((ePtrArray *)this, size);
    }

    eFORCEINLINE void reserve(eU32 capacity)
    {
        eArrayReserve((ePtrArray *)this, capacity);
    }

    eFORCEINLINE void clear()
    {
        eArrayClear((ePtrArray *)this);
    }

    eFORCEINLINE void free()
    {
        eArrayFree((ePtrArray *)this);
    }

    eFORCEINLINE void reverse()
    {
        eArrayReverse((ePtrArray *)this);
    }

    eFORCEINLINE T & append(const T &data)
    {
        *(T *)eArrayAppend((ePtrArray *)this) = data;
        return last();
    }

    eFORCEINLINE T & appendNew()
    {
        *(T *)eArrayAppend((ePtrArray *)this) = T();
        return last();
    }

    // only appends item if no element with
    // the same value exists in array
    eFORCEINLINE T & appendUnique(const T &data)
    {
        const eInt index = find(data);
        return (index != -1 ? m_data[index] : append(data));
    }

    eFORCEINLINE T & append()
    {
        append(T());
        return last();
    }

    eFORCEINLINE void append(const eArray &a)
    {
        for (eU32 i=0; i<a.size(); i++)
            append(a[i]);
    }

    eFORCEINLINE T & push(const T &data)
    {
        return append(data);
    }

    eFORCEINLINE T & push()
    {
        return append();
    }

    eFORCEINLINE T pop()
    {
        T data = last();
        removeLast();
        return data;
    }

    eFORCEINLINE void insert(eU32 index, const T &data)
    {
        eArrayInsert((ePtrArray *)this, index, (ePtr *)&data);
    }

    eFORCEINLINE void removeAt(eU32 index)
    {
        eArrayRemoveAt((ePtrArray *)this, index);
    }

    eFORCEINLINE void removeSwap(eU32 index)
    {
        eArrayRemoveSwap((ePtrArray *)this, index);
    }

    eFORCEINLINE void removeLast()
    {
        m_size--;
    }

    eFORCEINLINE eInt find(const T &data) const
    {
        return eArrayFind((ePtrArray *)this, (ePtr *)&data);
    }

    eFORCEINLINE eBool contains(const T &data) const
    {
        return (find(data) != -1);
    }

    eFORCEINLINE eU32 size() const
    {
        return m_size;
    }

    eFORCEINLINE eU32 sizeInBytes() const
    {
        return m_size*m_typeSize;
    }

    eFORCEINLINE eU32 capacity() const
    {
        return m_capacity;
    }

    eFORCEINLINE T & last()
    {
        eASSERT(m_size > 0);
        return m_data[m_size-1];
    }

    eFORCEINLINE const T & last() const
    {
        eASSERT(m_size > 0);
        return m_data[m_size-1];
    }

    eFORCEINLINE T & first()
    {
        eASSERT(m_size > 0);
        return m_data[0];
    }

    eFORCEINLINE const T & first() const
    {
        eASSERT(m_size > 0);
        return m_data[0];
    }

    eFORCEINLINE void sort(eBool (*predicate)(const T &a, const T &b))
    {
        eSort(m_data, m_size, predicate);
    }

	eFORCEINLINE void merge(eArray<T>& arr0, eArray<T>& arr1, eBool (*predicate)(const T &a, const T &b))
    {
		this->resize(arr0.size() + arr1.size());
		eMerge(arr0.m_data, arr0.m_size, arr1.m_data, arr1.m_size, m_data, predicate);
    }

    eFORCEINLINE eArray & operator = (const eArray &a)
    {
        if (this != &a)
            eArrayCopy((ePtrArray *)this, (const ePtrArray *)&a);

        return *this;
    }

    eFORCEINLINE T & operator [] (eInt index)
    {
        eASSERT(index >= 0 && (eU32)index < m_size);
        return m_data[index];
    }

    eFORCEINLINE const T & operator [] (eInt index) const
    {
        eASSERT(index >= 0 && (eU32)index < m_size);
        return m_data[index];
    }

    eFORCEINLINE eBool operator == (const eArray &a) const
    {
        return eArrayEqual((const ePtrArray *)this, (const ePtrArray *)&a);
    }

    eFORCEINLINE eBool operator != (const eArray &a) const
    {
        return !(*this == a);
    }

public:
    T *     m_data;
    eU32    m_size;
    eU32    m_capacity;
    eU32    m_typeSize;
};

#endif
