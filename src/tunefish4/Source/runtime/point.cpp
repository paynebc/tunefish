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

#include "system.hpp"

ePoint2::ePoint2() : x(0), y(0)
{
}

ePoint2::ePoint2(eInt nx, eInt ny) : x(nx), y(ny)
{
}

void ePoint2::set(eInt nx, eInt ny)
{
    x = nx;
    y = ny;
}

void ePoint2::null()
{
    x = y = 0;
}

void ePoint2::min(const ePoint2 &p)
{
    x = eMin(x, p.x);
    y = eMin(y, p.y);
}

void ePoint2::max(const ePoint2 &p)
{
    x = eMax(x, p.x);
    y = eMax(y, p.y);
}

ePoint2 ePoint2::operator + (const ePoint2 &p) const
{
    return ePoint2(x+p.x, y+p.y);
}

ePoint2 ePoint2::operator - (const ePoint2 &p) const
{
    return ePoint2(x-p.x, y-p.y);
}

ePoint2 ePoint2::operator * (eF32 s) const
{
    return ePoint2(eFtoL(x*s), eFtoL(y*s));
}

ePoint2 & ePoint2::operator += (const ePoint2 &p)
{
    *this = *this+p;
    return *this;
}

ePoint2 & ePoint2::operator -= (const ePoint2 &p)
{
    *this = *this-p;
    return *this;
}

ePoint2 & ePoint2::operator *= (eF32 s)
{
    *this = *this*s;
    return *this;
}

eInt ePoint2::operator [] (eInt index) const
{
    eASSERT(index < 2);
    return ((eInt *)this)[index];
}

eInt & ePoint2::operator [] (eInt index)
{
    eASSERT(index < 2);
    return ((eInt *)this)[index];
}

eBool ePoint2::operator == (const ePoint2 &p) const
{
    return (p.x == x && p.y == y);
}

eBool ePoint2::operator != (const ePoint2 &p) const
{
    return !(*this == p);
}

ePoint2 operator * (eF32 s, const ePoint2 &p)
{
    return p*s;
}

ePoint3::ePoint3() : x(0), y(0), z(0)
{
}

ePoint3::ePoint3(eInt nx, eInt ny, eInt nz) : x(nx), y(ny), z(nz)
{
}

void ePoint3::set(eInt nx, eInt ny, eInt nz)
{
    x = nx;
    y = ny;
    z = nz;
}

void ePoint3::null()
{
    x = y = z = 0;
}

void ePoint3::min(const ePoint3 &p)
{
    x = eMin(x, p.x);
    y = eMin(y, p.y);
    z = eMin(y, p.z);
}

void ePoint3::max(const ePoint3 &p)
{
    x = eMax(x, p.x);
    y = eMax(y, p.y);
    z = eMax(y, p.z);
}

ePoint3 ePoint3::operator + (const ePoint3 &p) const
{
    return ePoint3(x+p.x, y+p.y, z+p.z);
}

ePoint3 ePoint3::operator - (const ePoint3 &p) const
{
    return ePoint3(x-p.x, y-p.y, z-p.z);
}

ePoint3 ePoint3::operator * (eF32 s) const
{
    return ePoint3(eFtoL(x*s), eFtoL(y*s), eFtoL(z*s));
}

ePoint3 & ePoint3::operator += (const ePoint3 &p)
{
    *this = *this+p;
    return *this;
}

ePoint3 & ePoint3::operator -= (const ePoint3 &p)
{
    *this = *this-p;
    return *this;
}

ePoint3 & ePoint3::operator *= (eF32 s)
{
    *this = *this*s;
    return *this;
}

eInt ePoint3::operator [] (eInt index) const
{
    eASSERT(index >= 0 && index < 3);
    return ((eInt *)this)[index];
}

eInt & ePoint3::operator [] (eInt index)
{
    eASSERT(index >= 0 && index < 3);
    return ((eInt *)this)[index];
}

eBool ePoint3::operator == (const ePoint3 &p) const
{
    return (x == p.x && y == p.y && z == p.z);
}

eBool ePoint3::operator != (const ePoint3 &p) const
{
    return !(*this == p);
}

ePoint3 operator * (eF32 s, const ePoint3 &p)
{
    return p*s;
}