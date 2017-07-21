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

#ifndef TF_LOOKANDFEEL_H
#define TF_LOOKANDFEEL_H

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class Fonts
{
public:
    Fonts();
    virtual ~Fonts() { }

    juce_DeclareSingleton(Fonts, false);

    Font& title() { return _title; }
    Font& large() { return _large; }
    Font& normal() { return _normal; }
    Font& small() { return _small; }
    Font& fixed() { return _fixed; }

private:
    Font _title;
    Font _large;
    Font _normal;
    Font _small;
    Font _fixed;
};

class TfLookAndFeel : public LookAndFeel_V3
{
public:
    juce_DeclareSingleton(TfLookAndFeel, false);

    TfLookAndFeel();

    void drawRotarySlider (Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           Slider&) override;
};

#endif
