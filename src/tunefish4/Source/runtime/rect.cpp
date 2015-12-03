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

eRect::eRect() : x0(0), y0(0), x1(0), y1(0)
{
}

eRect::eRect(eInt nx0, eInt ny0, eInt nx1, eInt ny1) : x0(nx0), y0(ny0), x1(nx1), y1(ny1)
{
}

eRect::eRect(const ePoint2 &upperLeft, const ePoint2 &bottomRight)
{
    set(upperLeft.x, upperLeft.y, bottomRight.x, bottomRight.y);
}

void eRect::set(eInt nx0, eInt ny0, eInt nx1, eInt ny1)
{
    x0 = nx0;
    y0 = ny0;
    x1 = nx1;
    y1 = ny1;
}

void eRect::setWidth(eInt width)
{
    right = left+width;
}

void eRect::setHeight(eInt height)
{
    bottom = top+height;
}

void eRect::translate(eInt transX, eInt transY)
{
    x0 += transX;
    x1 += transX;
    y0 += transY;
    y1 += transY;
}

// makes sure that left < right and top < bottom
void eRect::normalize()
{
    if (x0 > x1)
        eSwap(x0, x1);
    if (y0 > y1)
        eSwap(y0, y1);
}

eBool eRect::pointInRect(const ePoint2 &p) const
{
    return (p.x >= x0 && p.x <= x1 && p.y >= y0 && p.y <= y1);
}

eBool eRect::rectInRect(const eRect &r) const
{
    return (r.x0 >= x0 && r.x1 <= x1 && r.y0 >= y0 && r.y1 <= y1);
}

eBool eRect::intersect(const eRect &r) const
{
    return !(x0 > r.x1 || x1 < r.x0 || y0 > r.y1 || y1 < r.y0);
}

// might be negative if rect isn't normalized
eInt eRect::getWidth() const
{
    return x1-x0;
}

// might be negative if rect isn't normalized
eInt eRect::getHeight() const
{
    return y1-y0;
}

ePoint2 eRect::getCenter() const
{
    return ePoint2((x0+x1)/2, (y0+y1)/2);
}

ePoint2 eRect::getUpperLeft() const
{
    return ePoint2(x0, y0);
}

ePoint2 eRect::getBottomRight() const
{
    return ePoint2(x1, y1);
}

// might be negative if rect isn't normalized
eSize2 eRect::getSize() const
{
    return eSize2(x1-x0, y1-y0);
}

eInt eRect::operator [] (eInt index) const
{
    eASSERT(index < 4);
    return ((eInt *)this)[index];
}

eInt & eRect::operator [] (eInt index)
{
    eASSERT(index < 4);
    return ((eInt *)this)[index];
}

eBool eRect::operator == (const eRect &r) const
{
    return (r.x0 == x0 && r.y0 == y0 && r.x1 == x1 && r.y1 == y1);
}

eBool eRect::operator != (const eRect &r) const
{
    return !(*this == r);
}