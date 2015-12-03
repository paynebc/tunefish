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

#ifndef RECT_HPP
#define RECT_HPP

class eRect
{
public:
    eRect();
    eRect(eInt nx0, eInt ny0, eInt nx1, eInt ny1);
    eRect(const ePoint2 &upperLeft, const ePoint2 &bottomRight);

    void            set(eInt nx0, eInt ny0, eInt nx1, eInt ny1);
    void            setWidth(eInt width);
    void            setHeight(eInt height);
    void            translate(eInt transX, eInt transY);
    void            normalize();
    eBool           pointInRect(const ePoint2 &p) const;
    eBool           rectInRect(const eRect &r) const;
    eBool           intersect(const eRect &r) const;

    eInt            getWidth() const;
    eInt            getHeight() const;
    ePoint2         getCenter() const;
    ePoint2         getUpperLeft() const;
    ePoint2         getBottomRight() const;
    eSize2          getSize() const;

    eInt            operator [] (eInt index) const;
    eInt &          operator [] (eInt index);
    eBool           operator == (const eRect &r) const;
    eBool           operator != (const eRect &r) const;

public:
    union
    {
        struct
        {
            eInt    left;
            eInt    top;
            eInt    right;
            eInt    bottom;
        };

        struct
        {
            eInt    x0;
            eInt    y0;
            eInt    x1;
            eInt    y1;
        };
    };
};

#endif