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

/**************************************************************************************
 *      LevelMeter
 **************************************************************************************/

#define MaxMeterChannels 2
#define MaxMeterLEDs 16

class LevelMeter;

/** A simple interface that can supply one or more LevelMeters with current values */

class LevelMeterSource
{
public:
    LevelMeterSource () {};
    virtual ~LevelMeterSource () {};
    
    /** Derived classes must provide linear scale float values.
     Logarithmic scaling is applied at display time, if necessary.
     */
    virtual float getMeterLevel (int channel, int meter = 0) = 0;
};


/**
 This is a simple level meter component that can take up to 2 channels
 and be refreshed only if needed. The constructor takes an optional meter id,
 which can be used to distinguish multiple meters fed by the same LevelMeterSource.
 By default the meter uses a logarithmic scale mapping signals from 0-1.
 */

class LevelMeter : public Component
{
public:
    LevelMeter (LevelMeterSource& source_,
                int numChannels_,
                int meterId_ = 0,
                int numLEDs_ = MaxMeterLEDs,
                bool linearDisplay = false,
                bool peakBox = true);
    
    virtual ~LevelMeter();
    
    /** Update meter levels from source and trigger re-display only if necessary.
     Must call this on the message thread */
    void refreshDisplayIfNeeded ();
    
    void resized() override;
    void paint (Graphics& g, int channel, int level);
    void paint (Graphics& g) override;
    
private:
    
    int map (float level);
    
    LevelMeterSource& source;
    bool linear;
    bool peak;
    int meterId;
    int numChannels;
    int numLEDs;
    
    Rectangle<int> gauges[MaxMeterChannels];
    Colour colours[MaxMeterLEDs];
    Colour background;
    int ledHeight;
    int ledWidth[MaxMeterLEDs];
    int levels[MaxMeterChannels];
    
    JUCE_LEAK_DETECTOR (LevelMeter);
};


#endif
