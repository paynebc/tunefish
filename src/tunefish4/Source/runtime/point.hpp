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

#ifndef POINT_HPP
#define POINT_HPP

typedef class ePoint2 eSize2;
typedef class ePoint3 eSize3;

class ePoint2
{
public:
    ePoint2();
    ePoint2(eInt nx, eInt ny);

    void            set(eInt nx, eInt ny);
    void            null();
    void            min(const ePoint2 &p);
    void            max(const ePoint2 &p);

    ePoint2         operator + (const ePoint2 &p) const;
    ePoint2         operator - (const ePoint2 &p) const;
    ePoint2         operator * (eF32 s) const;
    ePoint2 &       operator += (const ePoint2 &p);
    ePoint2 &       operator -= (const ePoint2 &p);
    ePoint2 &       operator *= (eF32 s);
    eInt            operator [] (eInt index) const;
    eInt &          operator [] (eInt index);
    eBool           operator == (const ePoint2 &p) const;
    eBool           operator != (const ePoint2 &p) const;

    friend ePoint2  operator * (eF32 s, const ePoint2 &p);

public:
    union
    {
        struct
        {
            eInt    x;
            eInt    y;
        };

        struct
        {
            eInt    width;
            eInt    height;
        };
    };
};

class ePoint3
{
public:
    ePoint3();
    ePoint3(eInt nx, eInt ny, eInt nz);

    void            set(eInt nx, eInt ny, eInt nz);
    void            null();
    void            min(const ePoint3 &p);
    void            max(const ePoint3 &p);

    ePoint3         operator + (const ePoint3 &p) const;
    ePoint3         operator - (const ePoint3 &p) const;
    ePoint3         operator * (eF32 s) const;
    ePoint3 &       operator += (const ePoint3 &p);
    ePoint3 &       operator -= (const ePoint3 &p);
    ePoint3 &       operator *= (eF32 s);
    eInt            operator [] (eInt index) const;
    eInt &          operator [] (eInt index);
    eBool           operator == (const ePoint3 &p) const;
    eBool           operator != (const ePoint3 &p) const;

    friend ePoint3  operator * (eF32 s, const ePoint3 &p);

public:
    union
    {
        struct
        {
            eInt    x;
            eInt    y;
            eInt    z;
        };

        struct
        {
            eInt    width;
            eInt    height;
            eInt    depth;
        };
    };
};

#endif