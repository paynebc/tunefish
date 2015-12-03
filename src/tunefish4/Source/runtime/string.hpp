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
 along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#ifndef STRING_HPP
#define STRING_HPP

// helper template structs for compile time string hashes
// (taken from "Quasi compile-time string hashing" article)

template<eU32 N, eU32 I> struct eFnvHash
{
    eINLINE static eU32 hash(const eChar (&str)[N])
    {
        return (eFnvHash<N, I-1>::hash(str)^str[I-1])*16777619;
    }
};

template<eU32 N> struct eFnvHash<N, 1>
{
    eINLINE static eU32 hash(const eChar (&str)[N])
    {
        return (2166136261^str[0])*16777619;
    }
};

// use this function to calculate a
// string hash at compile time
template<eU32 N> eU32 eHashStr(const eChar (&str)[N])
{
    return eFnvHash<N, N>::hash(str);
}

// dynamic string class
class eString
{
public:
    eString();
    eString(eChar chr);
    eString(const eChar *str);
    eString(const eChar *str, eU32 length);
    eString(const eString &str);

    eU32            length() const;
    eBool           equals(const eString &str, eU32 count) const;

    void            padLeft(eU32 totalLen, eChar chr);
    void            makeUpper();
    eBool           split(eChar token, eString &left, eString &right) const;
    eString         subStr(eU32 startIndex, eU32 endIndex) const;
    eString         simplified() const;
    void            remove(eU32 startIndex, eU32 endIndex);
    void            removeAt(eU32 index);
    eString         insert(eU32 index, const eString &str);
    eString         insert(eU32 index, const eChar c);


    eString         operator + (const eString &str) const;
    eString &       operator += (eChar c);
    eString &       operator += (const eString &str);
    eString &       operator = (const eChar *str);

    const eChar &   at(eU32 index) const;
    eChar &         at(eU32 index);
    const eChar &   operator [] (eInt index) const;
    eChar &         operator [] (eInt index);

    eBool           operator == (const eString &str) const;
    eBool           operator == (const eChar *str) const;
    eBool           operator != (const eString &str) const;
    eBool           operator != (const eChar *str) const;

    operator const eChar * () const;

#if !defined(ePLAYER) || !defined(eRELEASE)
    static eString	FromInt(eU32 Value);
	static eString	FromInts(const eU32* Value, eU32 Num);
    static eString	FromFloat(eF32 Value);
	static eString	FromFloats(const eF32* Value, eU32 Num);
#endif

private:
    eArray<eChar>   m_data;
};

#endif