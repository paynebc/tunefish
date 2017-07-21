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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "synth/tfrecorder.hpp"

void eTfGroupComponent::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colour::fromRGB(80, 80, 80), 0, 0, Colour::fromRGB(70, 70, 70), 0, (float) getHeight(), false));
    g.fillRect(0, 15, getWidth(), getHeight()-15);

    g.setColour(Colour::fromRGB(0, 0, 0));
    g.drawRect(0, 15, getWidth(), getHeight()-15);
    g.setColour(Colour::fromRGB(128, 128, 128));
    g.drawRect(1, 16, getWidth()-2, getHeight()-17);

    g.setFont(Font(14.0f));
    g.setColour(Colours::white);
    g.drawText(getText(), 5, 2, getWidth(), getHeight(), Justification::topLeft, true);
}

//==============================================================================
eTfFreqView::eTfFreqView() :
    m_synth(nullptr),
    m_instr(nullptr), 
    m_processor(nullptr)
{
    m_voice = new eTfVoice(eFALSE);
}

eTfFreqView::~eTfFreqView()
{
    eDelete(m_voice);
}

void eTfFreqView::setSynth(Tunefish4AudioProcessor *processor, eTfSynth *synth, eTfInstrument *instr)
{
    m_processor = processor;
    m_synth = synth;
    m_instr = instr;
}

void eTfFreqView::paint (Graphics& g)
{
    const eU32 viewWidth = getWidth()-1;
    const eU32 viewHeight = getHeight()-1;
    const eU32 halfViewHeight = viewHeight / 2;
    const eU32 quarterViewHeight = viewHeight / 4;

    g.setGradientFill (ColourGradient (Colour::fromRGB(60, 30, 30), 0, 0, Colour::fromRGB(20, 0, 0), (float) getWidth(), (float) getHeight()/2, false));
    g.fillRect(0, 0, getWidth(), getHeight()/2);

    g.setGradientFill (ColourGradient (Colour::fromRGB(60, 30, 30), 0, (float) getHeight()/2, Colour::fromRGB(20, 0, 0), (float) getWidth(), (float) getHeight(), false));
    g.fillRect(0, getHeight()/2, getWidth(), getHeight()/2);

    if (m_synth != nullptr && m_instr != nullptr)
    {
        g.setColour(Colours::white);

        // lock the synth and copy the current voice data
        // -----------------------------------------------------------
        m_processor->getSynthCriticalSection().enter();

        if (m_instr->latestTriggeredVoice) {
            eTfVoice *voice = m_instr->latestTriggeredVoice;
            m_voice->modMatrix = voice->modMatrix;
            m_voice->generator.modulation = voice->generator.modulation;
        }
        
        m_processor->getSynthCriticalSection().exit();

        // calculate the waveform
        // -----------------------------------------------------------
        eTfVoiceReset(*m_voice);
        eTfGeneratorUpdate(*m_synth, *m_instr, *m_voice, m_voice->generator, 1.0f);
        eF32 *freqTable = m_voice->generator.freqTable;

        if (eTfGeneratorModulate(*m_synth, *m_instr, m_voice->generator)) 
        {
            freqTable = m_voice->generator.freqModTable;
        }

        eF32 next_sep = 0.1f;
        for (eU32 x=0; x<viewWidth; x++)
        {
            eF32 pos = (eF32)x / viewWidth;
            pos *= pos;

            if (pos > next_sep)
            {
                next_sep += 0.1f;
                //painter.setPen(penSep);
                //painter.drawLine(x, 0, x, viewHeight/2);
            }

            eU32 offset = (eU32)(pos * TF_IFFT_FRAMESIZE);
            eF32 value = freqTable[offset];

            if (x % 2 == 0)
                g.setColour(Colour::fromRGB(255, 128, 128));
            else
                g.setColour(Colour::fromRGB(128, 64, 64));

            g.drawLine(x, halfViewHeight, x, halfViewHeight - (value * halfViewHeight));
        }

        eTfGeneratorFft(IFFT, TF_IFFT_FRAMESIZE, freqTable);
        eTfGeneratorNormalize(freqTable, TF_IFFT_FRAMESIZE);

        eF32 drive = m_instr->params[TF_GEN_DRIVE];
        drive *= 32.0f;
        drive += 1.0f;

        eF32 lastValue = 0.0f;
        eF32 lastValueDrv = 0.0f;
        for (eU32 x=1; x<viewWidth; x++)
        {
            eF32 pos = (eF32)x / viewWidth;

            eU32 offset = (eU32)(pos * TF_IFFT_FRAMESIZE);
            eF32 value = freqTable[offset*2];
            eF32 valueDrv = value * drive;

            value = eClamp<eF32>(-1.0f, value, 1.0f);
            valueDrv = eClamp<eF32>(-1.0f, valueDrv, 1.0f);

            g.setColour(Colour::fromRGB(128, 64, 64));
            g.drawLine(x-1, quarterViewHeight*3 - (lastValue * quarterViewHeight),
                       x,   quarterViewHeight*3 - (value * quarterViewHeight));

            g.setColour(Colour::fromRGB(255, 128, 128));
            g.drawLine(x-1, quarterViewHeight*3 - (lastValueDrv * quarterViewHeight),
                       x,   quarterViewHeight*3 - (valueDrv * quarterViewHeight));

            lastValue = value;
            lastValueDrv = valueDrv;
        }
    }

    g.setColour(Colour::fromRGB(0, 0, 0));
    g.drawRect(0, 0, getWidth(), getHeight());
    g.setColour(Colour::fromRGB(128, 128, 128));
    g.drawRect(1, 1, getWidth()-2, getHeight()-2);
}

const eU32 PIXWIDTH = 14;
const eU32 PIXHEIGHT = 14;

//==============================================================================
Tunefish4AudioProcessorEditor::Tunefish4AudioProcessorEditor (Tunefish4AudioProcessor* ownerFilter, eTfSynth *synth)
    : AudioProcessorEditor (ownerFilter),
    m_wasWindowHidden(false),
    m_btnLFO1ShapeSine("lfo1shapesine", DrawableButton::ImageOnButtonBackground),
    m_btnLFO1ShapeSawUp("lfo1shapesawup", DrawableButton::ImageOnButtonBackground),
    m_btnLFO1ShapeSawDown("lfo1shapesawdown", DrawableButton::ImageOnButtonBackground),
    m_btnLFO1ShapeSquare("lfo1shapesquare", DrawableButton::ImageOnButtonBackground),
    m_btnLFO1ShapeNoise("lfo1shapenoise", DrawableButton::ImageOnButtonBackground),
    m_btnLFO2ShapeSine("lfo2shapesine", DrawableButton::ImageOnButtonBackground),
    m_btnLFO2ShapeSawUp("lfo2shapesawup", DrawableButton::ImageOnButtonBackground),
    m_btnLFO2ShapeSawDown("lfo2shapesawdown", DrawableButton::ImageOnButtonBackground),
    m_btnLFO2ShapeSquare("lfo2shapesquare", DrawableButton::ImageOnButtonBackground),
    m_btnLFO2ShapeNoise("lfo2shapenoise", DrawableButton::ImageOnButtonBackground),
    m_imgShapeSine(Image::ARGB, PIXWIDTH, PIXHEIGHT, true),
    m_imgShapeSawUp(Image::ARGB, PIXWIDTH, PIXHEIGHT, true),
    m_imgShapeSawDown(Image::ARGB, PIXWIDTH, PIXHEIGHT, true),
    m_imgShapeSquare(Image::ARGB, PIXWIDTH, PIXHEIGHT, true),
    m_imgShapeNoise(Image::ARGB, PIXWIDTH, PIXHEIGHT, true)
{
    setSize(1080, 740);

    //m_openGlContext.attachTo(*this);

    // Init configuration 
    // -------------------------------------------------------------------------------------------
    PropertiesFile::Options storageParams;
    storageParams.applicationName = "Tunefish4";
    storageParams.filenameSuffix = ".settings";
    storageParams.folderName = "Brain Control";
    storageParams.osxLibrarySubFolder = "Application Support";
    storageParams.doNotSave = false;

    m_appProperties.setStorageParameters(storageParams);

    _addTextToggleButton(this, m_btnAnimationsOn, "Animations on", "", 10, 710, 100, 20);
    _addTextToggleButton(this, m_btnFastAnimations, "Fast animations", "", 120, 710, 100, 20);
    _addTextToggleButton(this, m_btnMovingWaveforms, "Moving waveforms", "", 230, 710, 100, 20);
    _addTextToggleButton(this, m_btnRecord, "Record", "", 900, 710, 100, 20);

    m_btnAnimationsOn.setToggleState(_configAreAnimationsOn(), dontSendNotification);
    m_btnFastAnimations.setToggleState(_configAreAnimationsFast(), dontSendNotification);
    m_btnMovingWaveforms.setToggleState(_configAreWaveformsMoving(), dontSendNotification);

    _addLabel(this, m_lblVersion, JucePlugin_VersionString, 1020, 710, 60, 20);
    m_lblVersion.setColour(Label::textColourId, Colour::fromRGB(100, 100, 100));
       

    // Init rest of UI
    // -------------------------------------------------------------------------------------------
    tfLookAndFeel.setColour(Slider::rotarySliderOutlineColourId, Colour::fromRGB(0, 0, 0));
    tfLookAndFeel.setColour(Slider::trackColourId, Colour::fromRGB(40, 40, 40));
    tfLookAndFeel.setColour(Slider::rotarySliderFillColourId, Colour::fromRGB(240, 240, 200));
    tfLookAndFeel.setColour(Slider::thumbColourId, Colour::fromRGB(255, 255, 255));
    tfLookAndFeel.setColour(Label::textColourId, Colour::fromRGB(255, 255, 255));
    tfLookAndFeel.setColour(GroupComponent::textColourId, Colour::fromRGB(255, 255, 255));
    tfLookAndFeel.setColour(GroupComponent::outlineColourId, Colour::fromRGB(255, 255, 255));
    tfLookAndFeel.setColour(ToggleButton::textColourId, Colour::fromRGB(255, 255, 255));
    tfLookAndFeel.setColour(ComboBox::textColourId, Colour::fromRGB(128, 150, 128));
    tfLookAndFeel.setColour(ComboBox::backgroundColourId, Colour::fromRGB(60, 60, 60));
    tfLookAndFeel.setColour(ComboBox::arrowColourId, Colour::fromRGB(80, 80, 80));
    tfLookAndFeel.setColour(ComboBox::outlineColourId, Colour::fromRGB(128, 128, 128));
    tfLookAndFeel.setColour(TextButton::textColourOffId, Colour::fromRGB(220, 220, 220));
    tfLookAndFeel.setColour(TextButton::textColourOnId, Colour::fromRGB(200, 230, 200));
    tfLookAndFeel.setColour(TextButton::buttonColourId, Colour::fromRGB(128, 128, 128));
    tfLookAndFeel.setColour(TextButton::buttonOnColourId, Colour::fromRGB(40, 40, 40));

    Colour colGlobal = Colour::fromRGB(240, 240, 240);
    Colour colGen = Colour::fromRGB(240, 200, 200);
    Colour colFilter = Colour::fromRGB(200, 200, 240);
    Colour colLfo = Colour::fromRGB(240, 240, 200);
    Colour colEffects = Colour::fromRGB(200, 240, 200);

    _createIcons();

    // -------------------------------------
    //  GLOBAL GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpGlobal, "GLOBAL", 10, 0, 590, 100);
    _addComboBox(&m_grpGlobal, m_cmbPolyphony, "1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16", 65, 25, 45, 20);
    _addComboBox(&m_grpGlobal, m_cmbPitchBendUp, "1|2|3|4|5|6|7|8|9|10|11|12", 65, 47, 45, 20);
    _addComboBox(&m_grpGlobal, m_cmbPitchBendDown, "1|2|3|4|5|6|7|8|9|10|11|12", 65, 69, 45, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPolyphony, "Polyphony:", 5, 25, 50, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPitchBendUp, "Pitch up:", 5, 47, 50, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPitchBendDown, "Pitch down:", 5, 69, 50, 20);
    _addRotarySlider(&m_grpGlobal, m_sldGlobVolume, m_lblGlobVolume, "Volume", 110, 25, colGlobal);
    _addRotarySlider(&m_grpGlobal, m_sldGlobFrequency, m_lblGlobFrequency, "Frequency", 170, 25, colGlobal);
    _addRotarySlider(&m_grpGlobal, m_sldGlobDetune, m_lblGlobDetune, "Detune", 230, 25, colGlobal);
    _addRotarySlider(&m_grpGlobal, m_sldGlobSlop, m_lblGlobSlop, "Slop", 290, 25, colGlobal);
    _addRotarySlider(&m_grpGlobal, m_sldGlobGlide, m_lblGlobGlide, "Glide", 350, 25, colGlobal);
    _addEditableComboBox(&m_grpGlobal, m_cmbInstrument, "", 410, 25, 160, 20);
    _addTextButton(&m_grpGlobal, m_btnSave, "Save", 410, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnRestore, "Restore", 465, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnPrev, "Prev", 520, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnCopy, "Copy", 410, 75, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnPaste, "Paste", 465, 75, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnNext, "Next", 520, 75, 50, 20);

    // -------------------------------------
    //  GENERATOR GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpGenerator, "GENERATOR", 10, 100, 590, 330);
    _addRotarySlider(&m_grpGenerator, m_sldGenVolume, m_lblGenVolume, "Volume", 10, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenPanning, m_lblGenPanning, "Panning", 70, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenSpread, m_lblGenSpread, "Spread", 130, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenBandwidth, m_lblGenBandwidth, "Bandwidth", 190, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenDamp, m_lblGenDamp, "Damp", 250, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenHarmonics, m_lblGenHarmonics, "Harmonics", 310, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenDrive, m_lblGenDrive, "Drive", 370, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenScale, m_lblGenScale, "Scale", 430, 25, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldGenModulation, m_lblGenModulation, "Modulation", 490, 25, colGen);

    _addRotarySlider(&m_grpGenerator, m_sldNoiseAmount, m_lblNoiseAmount, "Noise", 370, 85, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldNoiseFreq, m_lblNoiseFreq, "Noise Freq", 430, 85, colGen);
    _addRotarySlider(&m_grpGenerator, m_sldNoiseBandwidth, m_lblNoiseBandwidth, "Noise BW", 490, 85, colGen);

    _addLabel(&m_grpGenerator, m_lblUnisono, "Unisono:", 10, 95, 80, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono1, "1", "unisono", 90, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono2, "2", "unisono", 115, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono3, "3", "unisono", 140, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono4, "4", "unisono", 165, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono5, "5", "unisono", 190, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono6, "6", "unisono", 215, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono7, "7", "unisono", 240, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono8, "8", "unisono", 265, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono9, "9", "unisono", 290, 95, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenUnisono10, "10", "unisono", 315, 95, 25, 20);

    _addLabel(&m_grpGenerator, m_lblOctave, "Octave:", 10, 115, 80, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave1, "-4", "octave", 90, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave2, "-3", "octave", 115, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave3, "-2", "octave", 140, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave4, "-1", "octave", 165, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave5, "0", "octave", 190, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave6, "1", "octave", 215, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave7, "2", "octave", 240, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave8, "3", "octave", 265, 115, 25, 20);
    _addTextToggleButton(&m_grpGenerator, m_btnGenOctave9, "4", "octave", 290, 115, 25, 20);

    m_grpGenerator.addChildComponent(&m_freqView);
    m_freqView.setVisible(true);
    m_freqView.setBounds(10, 150, 570, 170);
    m_freqView.setSynth(ownerFilter, synth, synth->instr[0]);

    // -------------------------------------
    //  FILTER GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpLPFilter, "LOWPASS", 10, 430, 140, 110);
    _addTextToggleButton(&m_grpLPFilter, m_btnLPOn, "On", "", 5, 20, 25, 20);
    _addRotarySlider(&m_grpLPFilter, m_sldLPFrequency, m_lblLPFrequency, "Frequency", 10, 40, colFilter);
    _addRotarySlider(&m_grpLPFilter, m_sldLPResonance, m_lblLPResonance, "Resonance", 70, 40, colFilter);

    _addGroupBox(this, m_grpHPFilter, "HIGHPASS", 160, 430, 140, 110);
    _addTextToggleButton(&m_grpHPFilter, m_btnHPOn, "On", "", 5, 20, 25, 20);
    _addRotarySlider(&m_grpHPFilter, m_sldHPFrequency, m_lblHPFrequency, "Frequency", 10, 40, colFilter);
    _addRotarySlider(&m_grpHPFilter, m_sldHPResonance, m_lblHPResonance, "Resonance", 70, 40, colFilter);

    _addGroupBox(this, m_grpBPFilter, "BANDPASS", 310, 430, 140, 110);
    _addTextToggleButton(&m_grpBPFilter, m_btnBPOn, "On", "", 5, 20, 25, 20);
    _addRotarySlider(&m_grpBPFilter, m_sldBPFrequency, m_lblBPFrequency, "Frequency", 10, 40, colFilter);
    _addRotarySlider(&m_grpBPFilter, m_sldBPQ, m_lblBPQ, "Q", 70, 40, colFilter);

    _addGroupBox(this, m_grpNTFilter, "NOTCH", 460, 430, 140, 110);
    _addTextToggleButton(&m_grpNTFilter, m_btnNTOn, "On", "", 5, 20, 25, 20);
    _addRotarySlider(&m_grpNTFilter, m_sldNTFrequency, m_lblNTFrequency, "Frequency", 10, 40, colFilter);
    _addRotarySlider(&m_grpNTFilter, m_sldNTQ, m_lblNTQ, "Q", 70, 40, colFilter);

    // -------------------------------------
    //  LFO/ADSR GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpLFO1, "LFO1", 10, 540, 140, 160);
    _addRotarySlider(&m_grpLFO1, m_sldLFO1Rate, m_lblLFO1Rate, "Rate", 10, 25, colLfo);
    _addRotarySlider(&m_grpLFO1, m_sldLFO1Depth, m_lblLFO1Depth, "Depth", 70, 25, colLfo);
    _addTextToggleButton(&m_grpLFO1, m_btnLFO1Sync, "Synchronized", "", 10, 130, 125, 20);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSine, m_dimgShapeSine, "lfo1type", 10, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSawUp, m_dimgShapeSawUp, "lfo1type", 35, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSawDown, m_dimgShapeSawDown, "lfo1type", 60, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSquare, m_dimgShapeSquare, "lfo1type", 85, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeNoise, m_dimgShapeNoise, "lfo1type", 110, 95, 25, 25);

    _addGroupBox(this, m_grpLFO2, "LFO2", 160, 540, 140, 160);
    _addRotarySlider(&m_grpLFO2, m_sldLFO2Rate, m_lblLFO2Rate, "Rate", 10, 25, colLfo);
    _addRotarySlider(&m_grpLFO2, m_sldLFO2Depth, m_lblLFO2Depth, "Depth", 70, 25, colLfo);
    _addTextToggleButton(&m_grpLFO2, m_btnLFO2Sync, "Synchronized", "", 10, 130, 125, 20);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSine, m_dimgShapeSine, "lfo2type", 10, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSawUp, m_dimgShapeSawUp, "lfo2type", 35, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSawDown, m_dimgShapeSawDown, "lfo2type", 60, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSquare, m_dimgShapeSquare, "lfo2type", 85, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeNoise, m_dimgShapeNoise, "lfo2type", 110, 95, 25, 25);

    _addGroupBox(this, m_grpADSR1, "ADSR1", 310, 540, 140, 160);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Attack, m_lblADSR1Attack, "A", 10, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Decay, m_lblADSR1Decay, "D", 35, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Sustain, m_lblADSR1Sustain, "S", 60, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Release, m_lblADSR1Release, "R", 85, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Slope, m_lblADSR1Slope, "Slp", 110, 25, 20, 120);

    _addGroupBox(this, m_grpADSR2, "ADSR2", 460, 540, 140, 160);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Attack, m_lblADSR2Attack, "A", 10, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Decay, m_lblADSR2Decay, "D", 35, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Sustain, m_lblADSR2Sustain, "S", 60, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Release, m_lblADSR2Release, "R", 85, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Slope, m_lblADSR2Slope, "Slp", 110, 25, 20, 120);

    // -------------------------------------
    //  FX GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpFxFlanger, "FLANGER", 610, 0, 270, 100);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerLFO, m_lblFlangerLFO, "LFO", 10, 25, colEffects);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerFrequency, m_lblFlangerFrequency, "Frequency", 70, 25, colEffects);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerAmplitude, m_lblFlangerAmplitude, "Amplitude", 130, 25, colEffects);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerWet, m_lblFlangerWet, "Wet", 190, 25, colEffects);

    _addGroupBox(this, m_grpFxReverb, "REVERB", 610, 100, 270, 100);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbRoomsize, m_lblReverbRoomsize, "Roomsize", 10, 25, colEffects);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbDamp, m_lblReverbDamp, "Damp", 70, 25, colEffects);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbWet, m_lblReverbWet, "Wet", 130, 25, colEffects);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbWidth, m_lblReverbWidth, "Width", 190, 25, colEffects);

    _addGroupBox(this, m_grpFxDelay, "DELAY", 610, 200, 270, 100);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayLeft, m_lblDelayLeft, "Left", 10, 25, colEffects);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayRight, m_lblDelayRight, "Right", 70, 25, colEffects);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayDecay, m_lblDelayDecay, "Decay", 130, 25, colEffects);

    _addGroupBox(this, m_grpFxEQ, "EQ", 610, 300, 270, 100);
    _addRotarySlider(&m_grpFxEQ, m_sldEqLow, m_lblEqLow, "-880hz", 10, 25, colEffects);
    _addRotarySlider(&m_grpFxEQ, m_sldEqMid, m_lblEqMid, "880hz-5khz", 70, 25, colEffects);
    _addRotarySlider(&m_grpFxEQ, m_sldEqHigh, m_lblEqHigh, "5khz-", 130, 25, colEffects);

    _addGroupBox(this, m_grpFxChorus, "CHORUS", 610, 400, 270, 100);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusFreq, m_lblChorusFreq, "Frequency", 10, 25, colEffects);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusDepth, m_lblChorusDepth, "Depth", 70, 25, colEffects);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusGain, m_lblChorusGain, "Gain", 130, 25, colEffects);

    _addGroupBox(this, m_grpFxFormant, "FORMANT", 610, 500, 270, 100);
    _addRotarySlider(&m_grpFxFormant, m_sldFormantWet, m_lblFormantWet, "Wet", 10, 25, colEffects);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantA, "A", "formant", 70, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantE, "E", "formant", 95, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantI, "I", "formant", 120, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantO, "O", "formant", 145, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantU, "U", "formant", 170, 45, 25, 25);

    _addGroupBox(this, m_grpFxDistortion, "DISTORTION", 610, 600, 270, 100);
    _addRotarySlider(&m_grpFxDistortion, m_sldDistortionAmount, m_lblDistortionAmount, "Amount", 10, 25, colEffects);

    // -------------------------------------
    //  MOD MATRIX GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpModMatrix, "MOD MATRIX", 890, 0, 180, 410);
    _addComboBox(&m_grpModMatrix, m_cmbMM1Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 25, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM1Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 45, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM1Mod, 130, 25, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM2Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 70, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM2Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 90, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM2Mod, 130, 70, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM3Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 115, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM3Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 135, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM3Mod, 130, 115, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM4Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 160, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM4Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 180, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM4Mod, 130, 160, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM5Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 205, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM5Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 225, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM5Mod, 130, 205, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM6Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 250, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM6Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 270, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM6Mod, 130, 250, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM7Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 295, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM7Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 315, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM7Mod, 130, 295, colGlobal);
    _addComboBox(&m_grpModMatrix, m_cmbMM8Src, "none|LFO1|LFO2|ADSR1|ADSR2", 10, 340, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM8Dest, "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8", 10, 360, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM8Mod, 130, 340, colGlobal);

    // -------------------------------------
    //  EFFECT STACK GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpEffectStack, "EFFECTS STACK", 890, 410, 180, 290);
    _addComboBox(&m_grpEffectStack, m_cmbEffect1, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 25, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect2, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 50, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect3, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 75, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect4, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 100, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect5, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 125, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect6, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 150, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect7, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 175, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect8, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 200, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect9, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 225, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect10, "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ", 10, 250, 160, 20);

    _fillProgramCombobox();
    _resetTimer();
}

Tunefish4AudioProcessorEditor::~Tunefish4AudioProcessorEditor()
{
}

void Tunefish4AudioProcessorEditor::_addRotarySlider(Component *parent, Slider &slider, Label &label, String text, eU32 x, eU32 y, Colour colour)
{
    parent->addChildComponent(&slider);

    slider.setVisible(true);
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    //slider.setRotaryParameters(-2.9f, 2.9f, true);
    slider.addListener (this);
    slider.setRange (0.0, 1.0, 0.0);
    slider.setBounds (x, y, 60, 40);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&tfLookAndFeel);
    slider.setColour(Slider::rotarySliderFillColourId, colour);

    _addLabel(parent, label, text, x, y+40, 60, 20);
    label.setJustificationType(Justification::centredBottom);
}

void Tunefish4AudioProcessorEditor::_addRotarySliderNoLabel(Component *parent, Slider &slider, eU32 x, eU32 y, Colour colour)
{
    parent->addChildComponent(&slider);

    slider.setVisible(true);
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    //rslider.setRotaryParameters(-2.9f, 2.9f, true);
    slider.addListener (this);
    slider.setRange (0.0, 1.0, 0.0);
    slider.setBounds (x, y, 40, 40);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&tfLookAndFeel);
    slider.setColour(Slider::rotarySliderFillColourId, colour);
}

void Tunefish4AudioProcessorEditor::_addLinearSlider(Component *parent, Slider &slider, Label &label, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&slider);

    slider.setVisible(true);
    slider.setSliderStyle (Slider::LinearVertical);
    slider.addListener (this);
    slider.setRange (0.0, 1.0, 0.0);
    slider.setBounds (x, y, w, h-20);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&tfLookAndFeel);

    _addLabel(parent, label, text, x, y+h-20, w, 20);
    label.setJustificationType(Justification::centredBottom);
}

void Tunefish4AudioProcessorEditor::_addGroupBox(Component *parent, GroupComponent &group, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&group);

    group.setVisible(true);
    group.setText(text);
    group.setBounds(x, y, w, h);
    group.setLookAndFeel(&tfLookAndFeel);
}

void Tunefish4AudioProcessorEditor::_addTextButton(Component *parent, TextButton &button, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&button);

    button.setVisible(true);
    button.addListener(this);
    button.setLookAndFeel(&tfLookAndFeel);
    button.setButtonText(text);
    button.setBounds(x, y, w, h);
}

void Tunefish4AudioProcessorEditor::_addTextToggleButton(Component *parent, TextButton &button, String text, String group, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&button);

    button.setVisible(true);
    button.addListener(this);
    button.setClickingTogglesState(true);
    button.setLookAndFeel(&tfLookAndFeel);
    button.setButtonText(text);
    button.setBounds(x, y, w, h);

    if (group.length()>0)
        button.setRadioGroupId(group.hashCode());
}

void Tunefish4AudioProcessorEditor::_addImageToggleButton(Component *parent, DrawableButton &button, DrawableImage &image, String group, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&button);

    button.setVisible(true);
    button.addListener(this);
    button.setRadioGroupId(group.hashCode());
    button.setClickingTogglesState(true);
    button.setLookAndFeel(&tfLookAndFeel);
    button.setImages(&image);
    button.setBounds(x, y, w, h);
}

void Tunefish4AudioProcessorEditor::_addComboBox(Component *parent, ComboBox &combobox, String items, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&combobox);

    StringArray itemArray;
    itemArray.addTokens(items, "|", String::empty);

    combobox.setVisible(true);
    combobox.addListener(this);
    combobox.setLookAndFeel(&tfLookAndFeel);
    combobox.setBounds(x, y, w, h);
    combobox.addItemList(itemArray, 1);
}

void Tunefish4AudioProcessorEditor::_addEditableComboBox(Component *parent, ComboBox &combobox, String items, eU32 x, eU32 y, eU32 w, eU32 h)
{
    _addComboBox(parent, combobox, items, x, y, w, h);
    combobox.setEditableText(true);
}

void Tunefish4AudioProcessorEditor::_addLabel(Component *parent, Label &label, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&label);

    label.setVisible(true);
    label.setBounds(x, y, w, h);
    label.setJustificationType(Justification::left);
    label.setFont (Font (11.0f));
    label.setText(text, NotificationType::dontSendNotification);
    label.setLookAndFeel(&tfLookAndFeel);
}

void Tunefish4AudioProcessorEditor::_fillProgramCombobox()
{
    m_cmbInstrument.clear(dontSendNotification);

    const eU32 programCount = getProcessor()->getNumPrograms();
    const eU32 currentProgram = getProcessor()->getCurrentProgram();

    for (eU32 i=0; i<programCount; i++)
    {
        m_cmbInstrument.addItem(getProcessor()->getProgramName(i), i+1);
    }

    m_cmbInstrument.setSelectedItemIndex(currentProgram, dontSendNotification);
}

//==============================================================================
void Tunefish4AudioProcessorEditor::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colour::fromRGB(60, 60, 60), 0, 0,
                                       Colour::fromRGB(50, 50, 50), 0, static_cast<float>(getHeight()), false));
    g.fillAll();
}

void Tunefish4AudioProcessorEditor::timerCallback()
{
    refreshUiFromSynth();
}

void Tunefish4AudioProcessorEditor::visibilityChanged()
{
    m_wasWindowHidden = isVisible();
}

void Tunefish4AudioProcessorEditor::refreshUiFromSynth()
{
    Tunefish4AudioProcessor * processor = getProcessor();
    bool animationsOn = _configAreAnimationsOn();
    bool waveformsMoving = _configAreWaveformsMoving();

    if (animationsOn || processor->wasProgramSwitched() || m_wasWindowHidden)
    {
        // if the window was previously hidden, we need to fill all those comboboxes again
        if (m_wasWindowHidden)
        {
            processor->resetParamDirty(eTRUE);
            m_wasWindowHidden = false;
        }
        
        if (processor->wasProgramSwitched())
            m_cmbInstrument.setSelectedItemIndex(processor->getCurrentProgram(), dontSendNotification);

        if (processor->isParamDirty(TF_GEN_POLYPHONY))
            m_cmbPolyphony.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_GEN_POLYPHONY) * (TF_MAXVOICES - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_PITCHWHEEL_UP))
            m_cmbPitchBendUp.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_PITCHWHEEL_UP) * (TF_MAXPITCHBEND / 2))), dontSendNotification);

        if (processor->isParamDirty(TF_PITCHWHEEL_DOWN))
            m_cmbPitchBendDown.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_PITCHWHEEL_DOWN) * (TF_MAXPITCHBEND / 2))), dontSendNotification);

        m_sldGlobVolume.setValue(processor->getParameter(TF_GLOBAL_GAIN), dontSendNotification);
        m_sldGlobFrequency.setValue(processor->getParameter(TF_GEN_FREQ), dontSendNotification);
        m_sldGlobDetune.setValue(processor->getParameter(TF_GEN_DETUNE), dontSendNotification);
        m_sldGlobSlop.setValue(processor->getParameter(TF_GEN_SLOP), dontSendNotification);
        m_sldGlobGlide.setValue(processor->getParameter(TF_GEN_GLIDE), dontSendNotification);

        m_sldGenVolume.setValue(processor->getParameter(TF_GEN_VOLUME), dontSendNotification);
        m_sldGenPanning.setValue(processor->getParameter(TF_GEN_PANNING), dontSendNotification);
        m_sldGenSpread.setValue(processor->getParameter(TF_GEN_SPREAD), dontSendNotification);
        m_sldGenBandwidth.setValue(processor->getParameter(TF_GEN_BANDWIDTH), dontSendNotification);
        m_sldGenDamp.setValue(processor->getParameter(TF_GEN_DAMP), dontSendNotification);
        m_sldGenHarmonics.setValue(processor->getParameter(TF_GEN_NUMHARMONICS), dontSendNotification);
        m_sldGenDrive.setValue(processor->getParameter(TF_GEN_DRIVE), dontSendNotification);
        m_sldGenScale.setValue(processor->getParameter(TF_GEN_SCALE), dontSendNotification);
        m_sldGenModulation.setValue(processor->getParameter(TF_GEN_MODULATION), dontSendNotification);

        m_sldNoiseAmount.setValue(processor->getParameter(TF_NOISE_AMOUNT), dontSendNotification);
        m_sldNoiseFreq.setValue(processor->getParameter(TF_NOISE_FREQ), dontSendNotification);
        m_sldNoiseBandwidth.setValue(processor->getParameter(TF_NOISE_BW), dontSendNotification);

        m_btnLPOn.setToggleState(processor->getParameter(TF_LP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldLPFrequency.setValue(processor->getParameter(TF_LP_FILTER_CUTOFF), dontSendNotification);
        m_sldLPResonance.setValue(processor->getParameter(TF_LP_FILTER_RESONANCE), dontSendNotification);
        m_btnHPOn.setToggleState(processor->getParameter(TF_HP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldHPFrequency.setValue(processor->getParameter(TF_HP_FILTER_CUTOFF), dontSendNotification);
        m_sldHPResonance.setValue(processor->getParameter(TF_HP_FILTER_RESONANCE), dontSendNotification);
        m_btnBPOn.setToggleState(processor->getParameter(TF_BP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldBPFrequency.setValue(processor->getParameter(TF_BP_FILTER_CUTOFF), dontSendNotification);
        m_sldBPQ.setValue(processor->getParameter(TF_BP_FILTER_Q), dontSendNotification);
        m_btnNTOn.setToggleState(processor->getParameter(TF_NT_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldNTFrequency.setValue(processor->getParameter(TF_NT_FILTER_CUTOFF), dontSendNotification);
        m_sldNTQ.setValue(processor->getParameter(TF_NT_FILTER_Q), dontSendNotification);

        m_sldLFO1Rate.setValue(processor->getParameter(TF_LFO1_RATE), dontSendNotification);
        m_sldLFO1Depth.setValue(processor->getParameter(TF_LFO1_DEPTH), dontSendNotification);
        m_sldLFO2Rate.setValue(processor->getParameter(TF_LFO2_RATE), dontSendNotification);
        m_sldLFO2Depth.setValue(processor->getParameter(TF_LFO2_DEPTH), dontSendNotification);

        m_sldADSR1Attack.setValue(processor->getParameter(TF_ADSR1_ATTACK), dontSendNotification);
        m_sldADSR1Decay.setValue(processor->getParameter(TF_ADSR1_DECAY), dontSendNotification);
        m_sldADSR1Sustain.setValue(processor->getParameter(TF_ADSR1_SUSTAIN), dontSendNotification);
        m_sldADSR1Release.setValue(processor->getParameter(TF_ADSR1_RELEASE), dontSendNotification);
        m_sldADSR1Slope.setValue(processor->getParameter(TF_ADSR1_SLOPE), dontSendNotification);

        m_sldADSR2Attack.setValue(processor->getParameter(TF_ADSR2_ATTACK), dontSendNotification);
        m_sldADSR2Decay.setValue(processor->getParameter(TF_ADSR2_DECAY), dontSendNotification);
        m_sldADSR2Sustain.setValue(processor->getParameter(TF_ADSR2_SUSTAIN), dontSendNotification);
        m_sldADSR2Release.setValue(processor->getParameter(TF_ADSR2_RELEASE), dontSendNotification);
        m_sldADSR2Slope.setValue(processor->getParameter(TF_ADSR2_SLOPE), dontSendNotification);

        m_sldFlangerFrequency.setValue(processor->getParameter(TF_FLANGER_FREQUENCY), dontSendNotification);
        m_sldFlangerAmplitude.setValue(processor->getParameter(TF_FLANGER_AMPLITUDE), dontSendNotification);
        m_sldFlangerLFO.setValue(processor->getParameter(TF_FLANGER_LFO), dontSendNotification);
        m_sldFlangerWet.setValue(processor->getParameter(TF_FLANGER_WET), dontSendNotification);

        m_sldReverbRoomsize.setValue(processor->getParameter(TF_REVERB_ROOMSIZE), dontSendNotification);
        m_sldReverbDamp.setValue(processor->getParameter(TF_REVERB_DAMP), dontSendNotification);
        m_sldReverbWet.setValue(processor->getParameter(TF_REVERB_WET), dontSendNotification);
        m_sldReverbWidth.setValue(processor->getParameter(TF_REVERB_WIDTH), dontSendNotification);

        m_sldDelayLeft.setValue(processor->getParameter(TF_DELAY_LEFT), dontSendNotification);
        m_sldDelayRight.setValue(processor->getParameter(TF_DELAY_RIGHT), dontSendNotification);
        m_sldDelayDecay.setValue(processor->getParameter(TF_DELAY_DECAY), dontSendNotification);

        m_sldChorusFreq.setValue(processor->getParameter(TF_CHORUS_RATE), dontSendNotification);
        m_sldChorusDepth.setValue(processor->getParameter(TF_CHORUS_DEPTH), dontSendNotification);
        m_sldChorusGain.setValue(processor->getParameter(TF_CHORUS_GAIN), dontSendNotification);

        m_sldEqLow.setValue(processor->getParameter(TF_EQ_LOW), dontSendNotification);
        m_sldEqMid.setValue(processor->getParameter(TF_EQ_MID), dontSendNotification);
        m_sldEqHigh.setValue(processor->getParameter(TF_EQ_HIGH), dontSendNotification);

        m_sldFormantWet.setValue(processor->getParameter(TF_FORMANT_WET), dontSendNotification);

        m_sldDistortionAmount.setValue(processor->getParameter(TF_DISTORT_AMOUNT), dontSendNotification);

        // MOD Matrix Slot 1 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM1_SOURCE))
            m_cmbMM1Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM1_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM1_TARGET))
            m_cmbMM1Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM1_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM1Mod.setValue(processor->getParameter(TF_MM1_MOD), dontSendNotification);

        // MOD Matrix Slot 2 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM2_SOURCE))
            m_cmbMM2Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM2_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM2_TARGET))
            m_cmbMM2Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM2_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM2Mod.setValue(processor->getParameter(TF_MM2_MOD), dontSendNotification);

        // MOD Matrix Slot 3 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM3_SOURCE))
            m_cmbMM3Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM3_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM3_TARGET))
            m_cmbMM3Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM3_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM3Mod.setValue(processor->getParameter(TF_MM3_MOD), dontSendNotification);

        // MOD Matrix Slot 4 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM4_SOURCE))
            m_cmbMM4Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM4_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM4_TARGET))
            m_cmbMM4Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM4_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM4Mod.setValue(processor->getParameter(TF_MM4_MOD), dontSendNotification);

        // MOD Matrix Slot 5 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM5_SOURCE))
            m_cmbMM5Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM5_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM5_TARGET))
            m_cmbMM5Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM5_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM5Mod.setValue(processor->getParameter(TF_MM5_MOD), dontSendNotification);

        // MOD Matrix Slot 6 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM6_SOURCE))
            m_cmbMM6Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM6_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM6_TARGET))
            m_cmbMM6Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM6_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM6Mod.setValue(processor->getParameter(TF_MM6_MOD), dontSendNotification);

        // MOD Matrix Slot 7 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM7_SOURCE))
            m_cmbMM7Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM7_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM7_TARGET))
            m_cmbMM7Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM7_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM7Mod.setValue(processor->getParameter(TF_MM7_MOD), dontSendNotification);

        // MOD Matrix Slot 8 --------------------------------------------------------------
        if (processor->isParamDirty(TF_MM8_SOURCE))
            m_cmbMM8Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM8_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (processor->isParamDirty(TF_MM8_TARGET))
            m_cmbMM8Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_MM8_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM8Mod.setValue(processor->getParameter(TF_MM8_MOD), dontSendNotification);

        // EFFECTS Section ----------------------------------------------------------------
        if (processor->isParamDirty(TF_EFFECT_1))
            m_cmbEffect1.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_1) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_2))
            m_cmbEffect2.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_2) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_3))
            m_cmbEffect3.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_3) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_4))
            m_cmbEffect4.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_4) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_5))
            m_cmbEffect5.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_5) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_6))
            m_cmbEffect6.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_6) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_7))
            m_cmbEffect7.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_7) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_8))
            m_cmbEffect8.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_8) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_9))
            m_cmbEffect9.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_9) * TF_MAXEFFECTS)), dontSendNotification);

        if (processor->isParamDirty(TF_EFFECT_10))
            m_cmbEffect10.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(processor->getParameter(TF_EFFECT_10) * TF_MAXEFFECTS)), dontSendNotification);

        switch (static_cast<eU32>(eRoundNearest(processor->getParameter(TF_GEN_UNISONO) * (TF_MAXUNISONO - 1))))
        {
        case 0: m_btnGenUnisono1.setToggleState(true, dontSendNotification); break;
        case 1: m_btnGenUnisono2.setToggleState(true, dontSendNotification); break;
        case 2: m_btnGenUnisono3.setToggleState(true, dontSendNotification); break;
        case 3: m_btnGenUnisono4.setToggleState(true, dontSendNotification); break;
        case 4: m_btnGenUnisono5.setToggleState(true, dontSendNotification); break;
        case 5: m_btnGenUnisono6.setToggleState(true, dontSendNotification); break;
        case 6: m_btnGenUnisono7.setToggleState(true, dontSendNotification); break;
        case 7: m_btnGenUnisono8.setToggleState(true, dontSendNotification); break;
        case 8: m_btnGenUnisono9.setToggleState(true, dontSendNotification); break;
        case 9: m_btnGenUnisono10.setToggleState(true, dontSendNotification); break;
        }

        switch (static_cast<eU32>(eRoundNearest(processor->getParameter(TF_GEN_OCTAVE) * (TF_MAXOCTAVES - 1))))
        {
        case 8: m_btnGenOctave1.setToggleState(true, dontSendNotification); break;
        case 7: m_btnGenOctave2.setToggleState(true, dontSendNotification); break;
        case 6: m_btnGenOctave3.setToggleState(true, dontSendNotification); break;
        case 5: m_btnGenOctave4.setToggleState(true, dontSendNotification); break;
        case 4: m_btnGenOctave5.setToggleState(true, dontSendNotification); break;
        case 3: m_btnGenOctave6.setToggleState(true, dontSendNotification); break;
        case 2: m_btnGenOctave7.setToggleState(true, dontSendNotification); break;
        case 1: m_btnGenOctave8.setToggleState(true, dontSendNotification); break;
        case 0: m_btnGenOctave9.setToggleState(true, dontSendNotification); break;
        }

        switch (static_cast<eU32>(eRoundNearest(processor->getParameter(TF_LFO1_SHAPE) * (TF_LFOSHAPECOUNT - 1))))
        {
        case 0: m_btnLFO1ShapeSine.setToggleState(true, dontSendNotification); break;
        case 1: m_btnLFO1ShapeSawDown.setToggleState(true, dontSendNotification); break;
        case 2: m_btnLFO1ShapeSawUp.setToggleState(true, dontSendNotification); break;
        case 3: m_btnLFO1ShapeSquare.setToggleState(true, dontSendNotification); break;
        case 4: m_btnLFO1ShapeNoise.setToggleState(true, dontSendNotification); break;
        }

        switch (static_cast<eU32>(eRoundNearest(processor->getParameter(TF_LFO2_SHAPE) * (TF_LFOSHAPECOUNT - 1))))
        {
        case 0: m_btnLFO2ShapeSine.setToggleState(true, dontSendNotification); break;
        case 1: m_btnLFO2ShapeSawDown.setToggleState(true, dontSendNotification); break;
        case 2: m_btnLFO2ShapeSawUp.setToggleState(true, dontSendNotification); break;
        case 3: m_btnLFO2ShapeSquare.setToggleState(true, dontSendNotification); break;
        case 4: m_btnLFO2ShapeNoise.setToggleState(true, dontSendNotification); break;
        }

        switch (static_cast<eU32>(eRoundNearest(processor->getParameter(TF_FORMANT_MODE) * (TF_FORMANTCOUNT - 1))))
        {
        case 0: m_btnFormantA.setToggleState(true, dontSendNotification); break;
        case 1: m_btnFormantE.setToggleState(true, dontSendNotification); break;
        case 2: m_btnFormantI.setToggleState(true, dontSendNotification); break;
        case 3: m_btnFormantO.setToggleState(true, dontSendNotification); break;
        case 4: m_btnFormantU.setToggleState(true, dontSendNotification); break;
        }

        // -----------------------------------------------------
        //  Update modulation subvalues for all dials
        // -----------------------------------------------------

        m_sldGenVolume.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_VOLUME));
        m_sldGenSpread.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_SPREAD));
        m_sldGenScale.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_SCALE));
        m_sldGenPanning.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_PAN));
        m_sldGenHarmonics.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_NUMHARMONICS));
        m_sldNTQ.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_NT_FILTER_Q));
        m_sldNTFrequency.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_NT_FILTER_CUTOFF));
        m_sldNoiseAmount.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_NOISE_AMOUNT));
        m_sldMM8Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD8));
        m_sldMM7Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD7));
        m_sldMM6Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD6));
        m_sldMM5Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD5));
        m_sldMM4Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD4));
        m_sldMM3Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD3));
        m_sldMM2Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD2));
        m_sldMM1Mod.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_MOD1));
        m_sldLPResonance.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_LP_FILTER_RESONANCE));
        m_sldLPFrequency.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_LP_FILTER_CUTOFF));
        m_sldHPResonance.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_HP_FILTER_RESONANCE));
        m_sldHPFrequency.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_HP_FILTER_CUTOFF));
        m_sldGlobFrequency.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_FREQ));
        m_sldGenDrive.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_DRIVE));
        m_sldGlobDetune.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_DETUNE));
        m_sldGenDamp.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_DAMP));
        m_sldBPQ.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_BP_FILTER_Q));
        m_sldBPFrequency.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_BP_FILTER_CUTOFF));
        m_sldGenBandwidth.setModValue(processor->getParameterMod(eTfModMatrix::OUTPUT_BANDWIDTH));
    }

    if ((animationsOn && waveformsMoving) || processor->wasProgramSwitched())
    {
        m_freqView.repaint();
    }

    processor->resetParamDirty();
}

void Tunefish4AudioProcessorEditor::_setParameterNotifyingHost(Slider *slider, eTfParam param)
{
    getProcessor()->setParameterNotifyingHost(param, static_cast<float>(slider->getValue()));

    if (param == TF_GEN_BANDWIDTH ||
        param == TF_GEN_DAMP ||
        param == TF_GEN_DRIVE ||
        param == TF_GEN_MODULATION ||
        param == TF_GEN_NUMHARMONICS ||
        param == TF_GEN_SCALE)
    {
        m_freqView.repaint();
    }
}

void Tunefish4AudioProcessorEditor::_setParameterNotifyingHost(ComboBox *comboBox, eU32 maxIndex, eTfParam param) const
{
    getProcessor()->setParameterNotifyingHost(param, eF32(comboBox->getSelectedItemIndex()) / eF32(maxIndex));
}

void Tunefish4AudioProcessorEditor::_setParameterNotifyingHost(Button *button, eTfParam param) const
{
    getProcessor()->setParameterNotifyingHost(param, button->getToggleState() ? 1.0f : 0.0f);
}

void Tunefish4AudioProcessorEditor::_setParameterNotifyingHost(eF32 value, eTfParam param) const
{
    getProcessor()->setParameterNotifyingHost(param, value);
}

void Tunefish4AudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &m_sldGlobVolume)             _setParameterNotifyingHost(slider, TF_GLOBAL_GAIN);
    else if (slider == &m_sldGlobFrequency)     _setParameterNotifyingHost(slider, TF_GEN_FREQ);
    else if (slider == &m_sldGlobDetune)        _setParameterNotifyingHost(slider, TF_GEN_DETUNE);
    else if (slider == &m_sldGlobSlop)          _setParameterNotifyingHost(slider, TF_GEN_SLOP);
    else if (slider == &m_sldGlobGlide)         _setParameterNotifyingHost(slider, TF_GEN_GLIDE);

    else if (slider == &m_sldGenVolume)         _setParameterNotifyingHost(slider, TF_GEN_VOLUME);
    else if (slider == &m_sldGenPanning)        _setParameterNotifyingHost(slider, TF_GEN_PANNING);
    else if (slider == &m_sldGenSpread)         _setParameterNotifyingHost(slider, TF_GEN_SPREAD);
    else if (slider == &m_sldGenBandwidth)      _setParameterNotifyingHost(slider, TF_GEN_BANDWIDTH);
    else if (slider == &m_sldGenDamp)           _setParameterNotifyingHost(slider, TF_GEN_DAMP);
    else if (slider == &m_sldGenHarmonics)      _setParameterNotifyingHost(slider, TF_GEN_NUMHARMONICS);
    else if (slider == &m_sldGenDrive)          _setParameterNotifyingHost(slider, TF_GEN_DRIVE);
    else if (slider == &m_sldGenScale)          _setParameterNotifyingHost(slider, TF_GEN_SCALE);
    else if (slider == &m_sldGenModulation)     _setParameterNotifyingHost(slider, TF_GEN_MODULATION);

    else if (slider == &m_sldNoiseAmount)       _setParameterNotifyingHost(slider, TF_NOISE_AMOUNT);
    else if (slider == &m_sldNoiseFreq)         _setParameterNotifyingHost(slider, TF_NOISE_FREQ);
    else if (slider == &m_sldNoiseBandwidth)    _setParameterNotifyingHost(slider, TF_NOISE_BW);

    else if (slider == &m_sldLPFrequency)       _setParameterNotifyingHost(slider, TF_LP_FILTER_CUTOFF);
    else if (slider == &m_sldLPResonance)       _setParameterNotifyingHost(slider, TF_LP_FILTER_RESONANCE);
    else if (slider == &m_sldHPFrequency)       _setParameterNotifyingHost(slider, TF_HP_FILTER_CUTOFF);
    else if (slider == &m_sldHPResonance)       _setParameterNotifyingHost(slider, TF_HP_FILTER_RESONANCE);
    else if (slider == &m_sldBPFrequency)       _setParameterNotifyingHost(slider, TF_BP_FILTER_CUTOFF);
    else if (slider == &m_sldBPQ)               _setParameterNotifyingHost(slider, TF_BP_FILTER_Q);
    else if (slider == &m_sldNTFrequency)       _setParameterNotifyingHost(slider, TF_NT_FILTER_CUTOFF);
    else if (slider == &m_sldNTQ)               _setParameterNotifyingHost(slider, TF_NT_FILTER_Q);

    else if (slider == &m_sldLFO1Rate)          _setParameterNotifyingHost(slider, TF_LFO1_RATE);
    else if (slider == &m_sldLFO1Depth)         _setParameterNotifyingHost(slider, TF_LFO1_DEPTH);
    else if (slider == &m_sldLFO2Rate)          _setParameterNotifyingHost(slider, TF_LFO2_RATE);
    else if (slider == &m_sldLFO2Depth)         _setParameterNotifyingHost(slider, TF_LFO2_DEPTH);

    else if (slider == &m_sldADSR1Attack)       _setParameterNotifyingHost(slider, TF_ADSR1_ATTACK);
    else if (slider == &m_sldADSR1Decay)        _setParameterNotifyingHost(slider, TF_ADSR1_DECAY);
    else if (slider == &m_sldADSR1Sustain)      _setParameterNotifyingHost(slider, TF_ADSR1_SUSTAIN);
    else if (slider == &m_sldADSR1Release)      _setParameterNotifyingHost(slider, TF_ADSR1_RELEASE);
    else if (slider == &m_sldADSR1Slope)        _setParameterNotifyingHost(slider, TF_ADSR1_SLOPE);
    else if (slider == &m_sldADSR2Attack)       _setParameterNotifyingHost(slider, TF_ADSR2_ATTACK);
    else if (slider == &m_sldADSR2Decay)        _setParameterNotifyingHost(slider, TF_ADSR2_DECAY);
    else if (slider == &m_sldADSR2Sustain)      _setParameterNotifyingHost(slider, TF_ADSR2_SUSTAIN);
    else if (slider == &m_sldADSR2Release)      _setParameterNotifyingHost(slider, TF_ADSR2_RELEASE);
    else if (slider == &m_sldADSR2Slope)        _setParameterNotifyingHost(slider, TF_ADSR2_SLOPE);

    else if (slider == &m_sldFlangerLFO)        _setParameterNotifyingHost(slider, TF_FLANGER_LFO);
    else if (slider == &m_sldFlangerFrequency)  _setParameterNotifyingHost(slider, TF_FLANGER_FREQUENCY);
    else if (slider == &m_sldFlangerAmplitude)  _setParameterNotifyingHost(slider, TF_FLANGER_AMPLITUDE);
    else if (slider == &m_sldFlangerWet)        _setParameterNotifyingHost(slider, TF_FLANGER_WET);

    else if (slider == &m_sldReverbRoomsize)    _setParameterNotifyingHost(slider, TF_REVERB_ROOMSIZE);
    else if (slider == &m_sldReverbDamp)        _setParameterNotifyingHost(slider, TF_REVERB_DAMP);
    else if (slider == &m_sldReverbWet)         _setParameterNotifyingHost(slider, TF_REVERB_WET);
    else if (slider == &m_sldReverbWidth)       _setParameterNotifyingHost(slider, TF_REVERB_WIDTH);

    else if (slider == &m_sldDelayLeft)         _setParameterNotifyingHost(slider, TF_DELAY_LEFT);
    else if (slider == &m_sldDelayRight)        _setParameterNotifyingHost(slider, TF_DELAY_RIGHT);
    else if (slider == &m_sldDelayDecay)        _setParameterNotifyingHost(slider, TF_DELAY_DECAY);

    else if (slider == &m_sldEqLow)             _setParameterNotifyingHost(slider, TF_EQ_LOW);
    else if (slider == &m_sldEqMid)             _setParameterNotifyingHost(slider, TF_EQ_MID);
    else if (slider == &m_sldEqHigh)            _setParameterNotifyingHost(slider, TF_EQ_HIGH);

    else if (slider == &m_sldChorusFreq)        _setParameterNotifyingHost(slider, TF_CHORUS_RATE);
    else if (slider == &m_sldChorusDepth)       _setParameterNotifyingHost(slider, TF_CHORUS_DEPTH);
    else if (slider == &m_sldChorusGain)        _setParameterNotifyingHost(slider, TF_CHORUS_GAIN);

    else if (slider == &m_sldFormantWet)        _setParameterNotifyingHost(slider, TF_FORMANT_WET);

    else if (slider == &m_sldDistortionAmount)  _setParameterNotifyingHost(slider, TF_DISTORT_AMOUNT);

    else if (slider == &m_sldMM1Mod)            _setParameterNotifyingHost(slider, TF_MM1_MOD);
    else if (slider == &m_sldMM2Mod)            _setParameterNotifyingHost(slider, TF_MM2_MOD);
    else if (slider == &m_sldMM3Mod)            _setParameterNotifyingHost(slider, TF_MM3_MOD);
    else if (slider == &m_sldMM4Mod)            _setParameterNotifyingHost(slider, TF_MM4_MOD);
    else if (slider == &m_sldMM5Mod)            _setParameterNotifyingHost(slider, TF_MM5_MOD);
    else if (slider == &m_sldMM6Mod)            _setParameterNotifyingHost(slider, TF_MM6_MOD);
    else if (slider == &m_sldMM7Mod)            _setParameterNotifyingHost(slider, TF_MM7_MOD);
    else if (slider == &m_sldMM8Mod)            _setParameterNotifyingHost(slider, TF_MM8_MOD);
}

void Tunefish4AudioProcessorEditor::comboBoxChanged (ComboBox* comboBox)
{
    Tunefish4AudioProcessor *tfProcessor = static_cast<Tunefish4AudioProcessor *>(getProcessor());

    if (comboBox == &m_cmbPolyphony)            _setParameterNotifyingHost(comboBox, TF_MAXVOICES-1, TF_GEN_POLYPHONY);
    else if (comboBox == &m_cmbPitchBendUp)     _setParameterNotifyingHost(comboBox, TF_MAXPITCHBEND/2, TF_PITCHWHEEL_UP);
    else if (comboBox == &m_cmbPitchBendDown)   _setParameterNotifyingHost(comboBox, TF_MAXPITCHBEND/2, TF_PITCHWHEEL_DOWN);

    else if (comboBox == &m_cmbMM1Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM1_SOURCE);
    else if (comboBox == &m_cmbMM1Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM1_TARGET);
    else if (comboBox == &m_cmbMM2Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM2_SOURCE);
    else if (comboBox == &m_cmbMM2Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM2_TARGET);
    else if (comboBox == &m_cmbMM3Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM3_SOURCE);
    else if (comboBox == &m_cmbMM3Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM3_TARGET);
    else if (comboBox == &m_cmbMM4Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM4_SOURCE);
    else if (comboBox == &m_cmbMM4Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM4_TARGET);
    else if (comboBox == &m_cmbMM5Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM5_SOURCE);
    else if (comboBox == &m_cmbMM5Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM5_TARGET);
    else if (comboBox == &m_cmbMM6Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM6_SOURCE);
    else if (comboBox == &m_cmbMM6Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM6_TARGET);
    else if (comboBox == &m_cmbMM7Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM7_SOURCE);
    else if (comboBox == &m_cmbMM7Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM7_TARGET);
    else if (comboBox == &m_cmbMM8Src)          _setParameterNotifyingHost(comboBox, eTfModMatrix::INPUT_COUNT-1, TF_MM8_SOURCE);
    else if (comboBox == &m_cmbMM8Dest)         _setParameterNotifyingHost(comboBox, eTfModMatrix::OUTPUT_COUNT-1, TF_MM8_TARGET);

    else if (comboBox == &m_cmbEffect1)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_1);
    else if (comboBox == &m_cmbEffect2)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_2);
    else if (comboBox == &m_cmbEffect3)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_3);
    else if (comboBox == &m_cmbEffect4)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_4);
    else if (comboBox == &m_cmbEffect5)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_5);
    else if (comboBox == &m_cmbEffect6)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_6);
    else if (comboBox == &m_cmbEffect7)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_7);
    else if (comboBox == &m_cmbEffect8)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_8);
    else if (comboBox == &m_cmbEffect9)         _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_9);
    else if (comboBox == &m_cmbEffect10)        _setParameterNotifyingHost(comboBox, TF_MAXEFFECTS, TF_EFFECT_10);

    else if (comboBox == &m_cmbInstrument)
    {
        auto id = m_cmbInstrument.getSelectedId();
        auto index = m_cmbInstrument.getSelectedItemIndex();

        if (index >= 0)
        {
            tfProcessor->setCurrentProgram(index);
            tfProcessor->updateHostDisplay();
            m_freqView.repaint();
        }
        else
        {
            String newName = m_cmbInstrument.getText();
            tfProcessor->changeProgramName(tfProcessor->getCurrentProgram(), newName);
            tfProcessor->updateHostDisplay();
            _fillProgramCombobox();
        }
    }
}

static eF32 fromIndex(eU32 value, eU32 min, eU32 max)
{
    return static_cast<eF32>(value - min) / (max - min);
}

void Tunefish4AudioProcessorEditor::buttonClicked (Button *button)
{
    Tunefish4AudioProcessor *tfProcessor = static_cast<Tunefish4AudioProcessor *>(getProcessor());

    if (button == &m_btnLPOn)                   _setParameterNotifyingHost(button, TF_LP_FILTER_ON);
    else if (button == &m_btnHPOn)              _setParameterNotifyingHost(button, TF_HP_FILTER_ON);
    else if (button == &m_btnBPOn)              _setParameterNotifyingHost(button, TF_BP_FILTER_ON);
    else if (button == &m_btnNTOn)              _setParameterNotifyingHost(button, TF_NT_FILTER_ON);

    else if (button == &m_btnGenUnisono1)       _setParameterNotifyingHost(fromIndex(0, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono2)       _setParameterNotifyingHost(fromIndex(1, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono3)       _setParameterNotifyingHost(fromIndex(2, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono4)       _setParameterNotifyingHost(fromIndex(3, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono5)       _setParameterNotifyingHost(fromIndex(4, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono6)       _setParameterNotifyingHost(fromIndex(5, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono7)       _setParameterNotifyingHost(fromIndex(6, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono8)       _setParameterNotifyingHost(fromIndex(7, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono9)       _setParameterNotifyingHost(fromIndex(8, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);
    else if (button == &m_btnGenUnisono10)      _setParameterNotifyingHost(fromIndex(9, 0, TF_MAXUNISONO-1), TF_GEN_UNISONO);

    else if (button == &m_btnGenOctave1)        _setParameterNotifyingHost(fromIndex(8, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave2)        _setParameterNotifyingHost(fromIndex(7, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave3)        _setParameterNotifyingHost(fromIndex(6, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave4)        _setParameterNotifyingHost(fromIndex(5, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave5)        _setParameterNotifyingHost(fromIndex(4, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave6)        _setParameterNotifyingHost(fromIndex(3, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave7)        _setParameterNotifyingHost(fromIndex(2, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave8)        _setParameterNotifyingHost(fromIndex(1, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);
    else if (button == &m_btnGenOctave9)        _setParameterNotifyingHost(fromIndex(0, 0, TF_MAXOCTAVES-1), TF_GEN_OCTAVE);

    else if (button == &m_btnLFO1ShapeSine)     _setParameterNotifyingHost(fromIndex(0, 0, TF_LFOSHAPECOUNT-1), TF_LFO1_SHAPE);
    else if (button == &m_btnLFO1ShapeSawDown)  _setParameterNotifyingHost(fromIndex(1, 0, TF_LFOSHAPECOUNT-1), TF_LFO1_SHAPE);
    else if (button == &m_btnLFO1ShapeSawUp)    _setParameterNotifyingHost(fromIndex(2, 0, TF_LFOSHAPECOUNT-1), TF_LFO1_SHAPE);
    else if (button == &m_btnLFO1ShapeSquare)   _setParameterNotifyingHost(fromIndex(3, 0, TF_LFOSHAPECOUNT-1), TF_LFO1_SHAPE);
    else if (button == &m_btnLFO1ShapeNoise)    _setParameterNotifyingHost(fromIndex(4, 0, TF_LFOSHAPECOUNT-1), TF_LFO1_SHAPE);

    else if (button == &m_btnLFO2ShapeSine)     _setParameterNotifyingHost(fromIndex(0, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSawDown)  _setParameterNotifyingHost(fromIndex(1, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSawUp)    _setParameterNotifyingHost(fromIndex(2, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSquare)   _setParameterNotifyingHost(fromIndex(3, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeNoise)    _setParameterNotifyingHost(fromIndex(4, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);

    else if (button == &m_btnFormantA)          _setParameterNotifyingHost(fromIndex(0, 0, TF_FORMANTCOUNT-1), TF_FORMANT_MODE);
    else if (button == &m_btnFormantE)          _setParameterNotifyingHost(fromIndex(1, 0, TF_FORMANTCOUNT-1), TF_FORMANT_MODE);
    else if (button == &m_btnFormantI)          _setParameterNotifyingHost(fromIndex(2, 0, TF_FORMANTCOUNT-1), TF_FORMANT_MODE);
    else if (button == &m_btnFormantO)          _setParameterNotifyingHost(fromIndex(3, 0, TF_FORMANTCOUNT-1), TF_FORMANT_MODE);
    else if (button == &m_btnFormantU)          _setParameterNotifyingHost(fromIndex(4, 0, TF_FORMANTCOUNT-1), TF_FORMANT_MODE);

    else if (button == &m_btnSave)
    {
        tfProcessor->writeProgramToPresets();
        tfProcessor->saveProgram();
    }
    else if (button == &m_btnRestore)
    {
        tfProcessor->loadProgram();
        tfProcessor->loadProgramFromPresets();
        tfProcessor->updateHostDisplay();
        m_freqView.repaint();
    }
    else if (button == &m_btnPrev)
    {
        eU32 currentProgram = tfProcessor->getCurrentProgram();

        if (currentProgram > 0)
        {
            currentProgram--;
            m_cmbInstrument.setSelectedItemIndex(currentProgram);
            tfProcessor->setCurrentProgram(currentProgram);
            tfProcessor->updateHostDisplay();
            m_freqView.repaint();
        }
    }
    else if (button == &m_btnNext)
    {
        eU32 currentProgram = tfProcessor->getCurrentProgram();

        if (tfProcessor->getCurrentProgram() < tfProcessor->getNumPrograms()-1)
        {
            currentProgram++;
            m_cmbInstrument.setSelectedItemIndex(currentProgram);
            tfProcessor->setCurrentProgram(currentProgram);
            tfProcessor->updateHostDisplay();
            m_freqView.repaint();
        }
    }
    else if (button == &m_btnCopy)
    {
        tfProcessor->copyProgram();
    }
    else if (button == &m_btnPaste)
    {
        tfProcessor->pasteProgram();
        tfProcessor->updateHostDisplay();
        m_freqView.repaint();
    }
    else if (button == &m_btnAnimationsOn)
    {
        bool animationsOn = m_btnAnimationsOn.getToggleState();
        _configSetAnimationsOn(animationsOn);
    }
    else if (button == &m_btnFastAnimations)
    {
        bool animationsFast = m_btnFastAnimations.getToggleState();
        _configSetAnimationsFast(animationsFast);
        _resetTimer();
    }
    else if (button == &m_btnMovingWaveforms)
    {
        bool movingWaveforms = m_btnMovingWaveforms.getToggleState();
        _configSetWaveformsMoving(movingWaveforms);
    }
    else if (button == &m_btnRecord)
    {
        eTfRecorder &recorder = eTfRecorder::getInstance();

        if (recorder.isRecording())
        {
            recorder.stopRecording();

            FileChooser myChooser("Please select a file to save to", File::getSpecialLocation(File::userHomeDirectory), "*.tfm");
            if (myChooser.browseForFileToSave(true))
            {
                File file = myChooser.getResult();
                recorder.saveToFile(file);
            }       
        }
        else
        {
            recorder.reset();
            recorder.startRecording();
        }
    }
}

void Tunefish4AudioProcessorEditor::_createIcons()
{
    const auto MAX_X = PIXWIDTH-1;
    const auto MAX_Y = PIXHEIGHT-1;

    // sine
    // ------------------------------------------
    eU32 old = 0;
    Graphics gSine(m_imgShapeSine);
    gSine.setColour(Colours::white);
    for (eU32 i=0;i<PIXWIDTH;i++)
    {
        eU32 sine = eSin(static_cast<eF32>(i) / PIXWIDTH * ePI*2) * PIXHEIGHT/2;
        if (i>0)
            gSine.drawLine(i-1, old+PIXHEIGHT/2, i, sine+PIXHEIGHT/2);
        old = sine;
    }

    // saw down
    // ------------------------------------------
    Graphics gSawDown(m_imgShapeSawDown);
    gSawDown.setColour(Colours::white);
    gSawDown.drawLine(0, MAX_Y, MAX_X, 0);
    gSawDown.drawLine(MAX_X, 0, MAX_X, MAX_Y);

    // saw up
    // ------------------------------------------
    Graphics gSawUp(m_imgShapeSawUp);
    gSawUp.setColour(Colours::white);
    gSawUp.drawLine(0, 0, MAX_X, MAX_Y);
    gSawUp.drawLine(0, 0, 0, MAX_Y);

    // pulse
    // ------------------------------------------
    Graphics gSquare(m_imgShapeSquare);
    gSquare.setColour(Colours::white);
    gSquare.drawLine(0, 0, MAX_X/2, 0);
    gSquare.drawLine(MAX_X/2, MAX_Y, MAX_X, MAX_Y);
    gSquare.drawLine(MAX_X/2, 0, MAX_X/2, MAX_Y);

    // noise
    // ------------------------------------------
    Graphics gNoise(m_imgShapeNoise);
    gNoise.setColour(Colours::white);
    eRandom random;
    for (eU32 i=0;i<PIXWIDTH;i++)
    {
        gNoise.drawLine(i, MAX_Y/2, i, random.NextInt(0, MAX_Y));
    }

    m_dimgShapeSine.setImage(m_imgShapeSine);
    m_dimgShapeSawUp.setImage(m_imgShapeSawUp);
    m_dimgShapeSawDown.setImage(m_imgShapeSawDown);
    m_dimgShapeSquare.setImage(m_imgShapeSquare);
    m_dimgShapeNoise.setImage(m_imgShapeNoise);
}

void Tunefish4AudioProcessorEditor::_resetTimer()
{
   if (_configAreAnimationsFast())
   {
       startTimer(40);
   }
   else
   {
       startTimer(100);
   }
}

bool Tunefish4AudioProcessorEditor::_configAreAnimationsOn()
{
    return m_appProperties.getUserSettings()->getBoolValue("AnimationsOn", true);
}

bool Tunefish4AudioProcessorEditor::_configAreAnimationsFast()
{
    return m_appProperties.getUserSettings()->getBoolValue("FastAnimations", true);
}

bool Tunefish4AudioProcessorEditor::_configAreWaveformsMoving()
{
    return m_appProperties.getUserSettings()->getBoolValue("MovingWaveforms", true);
}

void Tunefish4AudioProcessorEditor::_configSetAnimationsOn(bool value)
{
    m_appProperties.getUserSettings()->setValue("AnimationsOn", value);
}

void Tunefish4AudioProcessorEditor::_configSetAnimationsFast(bool value)
{
    m_appProperties.getUserSettings()->setValue("FastAnimations", value);
}

void Tunefish4AudioProcessorEditor::_configSetWaveformsMoving(bool value)
{
    m_appProperties.getUserSettings()->setValue("MovingWaveforms", value);
}
