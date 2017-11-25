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

#include "tflookandfeel.h"
#include "PluginEditor.h"

juce_ImplementSingleton(Fonts);

Fonts::Fonts()
{
#ifdef JUCE_MAC
    _title = Font("Helvetica", 20.0, Font::bold);
    _large = Font("Helvetica", 14.0, Font::plain);
    _normal = Font("Helvetica", 11.0, Font::plain);
    _small = Font("Helvetica", 10.0, Font::plain);
    _fixed = Font("Menlo", 11.0, Font::plain);
#else
    _title = Font("Arial", 20.0, Font::bold);
    _large = Font("Arial", 14.0, Font::plain);
    _normal = Font("Arial", 12.0, Font::plain);
    _small = Font("Arial", 11.0, Font::plain);
    _fixed = Font("Courier", 12.0, Font::plain);
#endif
}


juce_ImplementSingleton(TfLookAndFeel);

TfLookAndFeel::TfLookAndFeel()
{
#ifdef JUCE_MAC
    setDefaultSansSerifTypefaceName("Helvetica");
#else
    setDefaultSansSerifTypefaceName("Arial");
#endif

    setColour(Slider::rotarySliderOutlineColourId, Colour::fromRGB(0, 0, 0));
    setColour(Slider::trackColourId, Colour::fromRGB(40, 40, 40));
    setColour(Slider::rotarySliderFillColourId, Colour::fromRGB(240, 240, 200));
    setColour(Slider::thumbColourId, Colour::fromRGB(255, 255, 255));

    setColour(Label::textColourId, Colour::fromRGB(255, 255, 255));

    setColour(GroupComponent::textColourId, Colour::fromRGB(255, 255, 255));
    setColour(GroupComponent::outlineColourId, Colour::fromRGB(255, 255, 255));

    setColour(ToggleButton::textColourId, Colour::fromRGB(255, 255, 255));

    setColour(ComboBox::textColourId, Colour::fromRGB(128, 150, 128));
    setColour(ComboBox::backgroundColourId, Colour::fromRGB(60, 60, 60));
    setColour(ComboBox::arrowColourId, Colour::fromRGB(80, 80, 80));
    setColour(ComboBox::outlineColourId, Colour::fromRGB(128, 128, 128));

    setColour(TextButton::textColourOffId, Colour::fromRGB(220, 220, 220));
    setColour(TextButton::textColourOnId, Colour::fromRGB(200, 230, 200));
    setColour(TextButton::buttonColourId, Colour::fromRGB(128, 128, 128));
    setColour(TextButton::buttonOnColourId, Colour::fromRGB(40, 40, 40));
}

void TfLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    const float radius = jmin (width / 2, height / 2) - 2.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    eTfSlider &tfslider = *static_cast<eTfSlider*>(&slider);
    eF32 modValue = tfslider.getModValue();
    eF32 modAngle = rotaryStartAngle + eClamp<eF32>(0.0f, (sliderPos * modValue), 1.0f) * (rotaryEndAngle - rotaryStartAngle);

    if (radius > 12.0f)
    {
        g.setColour (slider.findColour (Slider::trackColourId));

        const float thickness = 0.55f;
        const float modThickness = 0.75f;

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
            g.fillPath (filledArc);
        }

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.9f));
        else
            g.setColour (Colour (0x80808080));

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, thickness);
            g.fillPath (filledArc);
        }

        g.drawText(String(eU32(eRoundNearest(static_cast<eF32>(slider.getValue()) * 99.0f))), x, y, width, height, Justification::centred, false);

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withBrightness(0.5f));
        else
            g.setColour (Colour (0x80808080));

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, modAngle, modThickness);
            g.fillPath (filledArc);
        }

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
        else
            g.setColour (Colour (0x80808080));

        Path outlineArc;
        outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
        outlineArc.closeSubPath();

        g.strokePath (outlineArc, PathStrokeType (slider.isEnabled() ? 2.0f : 0.3f));
    }
    else
    {
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        Path p;
        p.addEllipse (-0.4f * rw, -0.4f * rw, rw * 0.8f, rw * 0.8f);
        PathStrokeType (rw * 0.1f).createStrokedPath (p, p);

        p.addLineSegment (Line<float> (0.0f, 0.0f, 0.0f, -radius), rw * 0.2f);

        g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }
}

/**************************************************************************************
 *      LevelMeter
 **************************************************************************************/

LevelMeter::LevelMeter (LevelMeterSource& source_,
                        int numChannels_,
                        int meterId_,
                        int numLEDs_,
                        bool linearDisplay,
                        bool peakBox)  :
source (source_),
linear (linearDisplay),
peak (peakBox),
meterId (meterId_),
numChannels (jmin (MaxMeterChannels, numChannels_)),
numLEDs (jlimit (8, MaxMeterLEDs, numLEDs_)),
background (Colour::fromRGB(40, 40, 40)),
ledHeight (0)
{
    zerostruct(gauges);
    zerostruct(levels);
    zerostruct(ledWidth);
    
    const int limitRed      = numLEDs-1;
    const int limitOrange   = numLEDs-3;
    
    for (int i = 0; i < numLEDs; ++i)
    {
        colours[i] = Colours::lightgreen.darker(0.2f);
        if (i >= limitOrange)  colours[i] = Colours::orange;
        if (i >= limitRed)     colours[i] = Colours::red;
    }
}

LevelMeter::~LevelMeter ()
{
}

void LevelMeter::resized()
{
    int padding = 1;
    if (numChannels > 1 && getHeight() > 30)
        padding = 2;
    
    int gap = 1;
    if (getHeight() > 30)
        gap = 2;
    if (numChannels < 2)
        gap = 0;
    
    const int vspace = getHeight() - padding*2 - (numChannels-1)*gap;
    const int hspace = getWidth() - (padding*2);
    const int w = hspace / numLEDs;
    for (int i=0; i < numLEDs; ++i) ledWidth[i] = w;
    ledWidth[numLEDs-1] = hspace - (numLEDs-1)*w;
    ledHeight = vspace / numChannels;
    
    for (int chan=0; chan < numChannels; ++chan)
        gauges[chan] = Rectangle<int>(padding,
                                      padding+(chan * (ledHeight + gap)),
                                      hspace,
                                      ledHeight);
    // fix unsymmetric placement
    if (numChannels > 1
        && gauges[numChannels-1].getBottom() < (getHeight()-padding))
        gauges[numChannels-1].translate(0,1);
    
}

void LevelMeter::paint (Graphics& g, int channel, int level)
{
    g.setColour (background);
    g.fillRect (gauges[channel]);
    const int x = gauges[channel].getX();
    const int y = gauges[channel].getY();
    
    // level is 1-based here: 1...numLEDs, 0=off
    if (level > 0)
    {
        int offset = x;
        for (int i=0; i < level; ++i)
        {
            g.setColour(colours[i]);
            g.fillRect(offset, y, ledWidth[i] - 1, ledHeight);
            offset += ledWidth[i];
        }
    }
    if (peak && level < numLEDs)
    {
        g.setColour(Colours::black);
        g.fillRect(x + ledWidth[0]*(numLEDs-1),
                   y,
                   ledWidth[numLEDs-1] - 1,
                   ledHeight);
    }
}

void LevelMeter::paint (Graphics& g)
{
    g.fillAll(Colour::fromRGB(40, 40, 40));
    
    for (int chan=0; chan < numChannels; ++chan)
        paint (g, chan, levels[chan]);
}

void LevelMeter::refreshDisplayIfNeeded ()
{
    bool dirty = false;
    
    for (int chan=0; chan < numChannels; ++chan)
    {
        int levelShown = levels[chan];
        int levelInput = map (source.getMeterLevel (chan, meterId));
        jassert (levelInput <= numLEDs);
        if (levelInput > levelShown)
        {
            levels[chan] = levelInput;
            dirty = true;
        }
        else
        {
            if (levelShown > 0)
            {
                if (--levels[chan] > 0)
                    --levels[chan];
                dirty = true;
            }
        }
    }
    if (dirty)
        repaint ();
}

int LevelMeter::map (float gain)
{
    // Trigger red warning LED only beyond unity level
    const float input = gain * 0.99f;
    
    if (linear)
        return jlimit(0,numLEDs,roundToInt(input * numLEDs));
    
    float scale = jlimit (0.0f, 1.0f, 0.96f + logf(sqrt(input) + 0.1f));
    return roundToInt(scale * numLEDs);
}

