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

        g.drawText(String(eU32(eRoundNearest(slider.getValue() * 99))), x, y, width, height, Justification::centred, false);

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
