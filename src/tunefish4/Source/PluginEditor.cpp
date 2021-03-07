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

// this is used for recording songs to tf4m format only. Meant for Demos or Executable music
#define HAVE_RECORDING 

// this is used for writing all patches to a C++ header file for use as factory patches
#define HAVE_PATCH_WRITER

// this is used for reading patches from a .txt file
#define HAVE_PATCH_LOADER

const char* MOD_SOURCES = "none|LFO1|LFO2|ADSR1|ADSR2|ModWheel";
const char* MOD_TARGETS = "none|Bandwidth|Damp|Harmonics|Scale|Volume|Frequency|Panning|Detune|Spread|Drive|Noise|LP Cutoff|LP Resonance|HP Cutoff|HP Resonance|BP Cutoff|BP Q|NT Cutoff|NT Q|ADSR1 Decay|ADSR2 Decay|Mod1|Mod2|Mod3|Mod4|Mod5|Mod6|Mod7|Mod8|LFO1 Depth|LFO2 Depth";
const char* FX_SECTIONS = "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ";


const Colour COL_WINDOW_BG                  = Colour::fromRGB(40, 40, 40);

const Colour COL_GROUP_GRADIENT1            = Colour::fromRGB(80, 80, 80);
const Colour COL_GROUP_GRADIENT2            = Colour::fromRGB(70, 70, 70);
const Colour COL_GROUP_DISABLED_GRADIENT1   = Colour::fromRGB(60, 60, 60);
const Colour COL_GROUP_DISABLED_GRADIENT2   = Colour::fromRGB(50, 50, 50);
const Colour COL_GROUP_RECT                 = Colour::fromRGB(0, 0, 0);
const Colour COL_GROUP_RECT_LIGHT           = Colour::fromRGB(128, 128, 128);
const Colour COL_GROUP_DISABLED_RECT        = Colour::fromRGB(0, 0, 0);
const Colour COL_GROUP_DISABLED_RECT_LIGHT  = Colour::fromRGB(100, 100, 100);

const Colour COL_FREQVIEW_BG_GRADIENT1      = Colour::fromRGB(60, 30, 30);
const Colour COL_FREQVIEW_BG_GRADIENT2      = Colour::fromRGB(20, 0, 0);
const Colour COL_FREQVIEW_LINES             = Colour::fromRGB(255, 128, 128);
const Colour COL_FREQVIEW_LINES_ALT         = Colour::fromRGB(128, 64, 64);
const Colour COL_FREQVIEW_RECT_DARK         = Colour::fromRGB(0, 0, 0);
const Colour COL_FREQVIEW_RECT_LIGHT        = Colour::fromRGB(128, 128, 128);

const Colour COL_ROTARYSLIDER_GLOBAL        = Colour::fromRGB(240, 240, 240);
const Colour COL_ROTARYSLIDER_GEN           = Colour::fromRGB(240, 200, 200);
const Colour COL_ROTARYSLIDER_FILTER        = Colour::fromRGB(200, 200, 240);
const Colour COL_ROTARYSLIDER_LFO           = Colour::fromRGB(240, 240, 200);
const Colour COL_ROTARYSLIDER_EFFECTS       = Colour::fromRGB(200, 240, 200);

const eU32 PIXWIDTH = 14;
const eU32 PIXHEIGHT = 14;

void eTfGroupComponent::paint (Graphics& g)
{
    bool enabled = isEnabled();

    const Colour gradientCol1 = enabled ? COL_GROUP_GRADIENT1 : COL_GROUP_DISABLED_GRADIENT1;
    const Colour gradientCol2 = enabled ? COL_GROUP_GRADIENT2 : COL_GROUP_DISABLED_GRADIENT2;
    const Colour rectCol1 = enabled ? COL_GROUP_RECT : COL_GROUP_DISABLED_RECT;
    const Colour rectCol2 = enabled ? COL_GROUP_RECT_LIGHT : COL_GROUP_DISABLED_RECT_LIGHT;

    g.setGradientFill(ColourGradient(gradientCol1, 0, 0, gradientCol2, 0, static_cast<float>(getHeight()), false));
    g.fillRect(0, 15, getWidth(), getHeight() - 15);

    g.setColour(rectCol1);
    g.drawRect(0, 15, getWidth(), getHeight()-15);
    g.setColour(rectCol2);
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
    const float viewWidth = static_cast<float>(getWidth()) - 1.0f;
    const float viewHeight = static_cast<float>(getHeight()) - 1.0f;
    const float halfViewHeight = viewHeight / 2;
    const float quarterViewHeight = viewHeight / 4;

    g.setGradientFill (ColourGradient (COL_FREQVIEW_BG_GRADIENT1, 0, 0, COL_FREQVIEW_BG_GRADIENT2, static_cast<float>(getWidth()), static_cast<float>(getHeight())/2.0f, false));
    g.fillRect(0, 0, getWidth(), getHeight()/2);

    g.setGradientFill (ColourGradient (COL_FREQVIEW_BG_GRADIENT1, 0, static_cast<float>(getHeight())/2.0f, COL_FREQVIEW_BG_GRADIENT2, static_cast<float>(getWidth()), static_cast<float>(getHeight()), false));
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
            float fx = static_cast<eF32>(x);
            eF32 pos = fx / viewWidth;
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
                g.setColour(COL_FREQVIEW_LINES);
            else
                g.setColour(COL_FREQVIEW_LINES_ALT);

            g.drawLine(fx, halfViewHeight, fx, halfViewHeight - (value * halfViewHeight));
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
            float fx = static_cast<eF32>(x);
            eF32 pos = fx / viewWidth;

            eU32 offset = static_cast<eU32>(pos * TF_IFFT_FRAMESIZE);
            eF32 value = freqTable[offset*2];
            eF32 valueDrv = value * drive;

            value = eClamp<eF32>(-1.0f, value, 1.0f);
            valueDrv = eClamp<eF32>(-1.0f, valueDrv, 1.0f);

            g.setColour(COL_FREQVIEW_LINES_ALT);
            g.drawLine(fx -1, quarterViewHeight*3 - (lastValue * quarterViewHeight),
                        fx,   quarterViewHeight*3 - (value * quarterViewHeight));

            g.setColour(COL_FREQVIEW_LINES);
            g.drawLine(fx -1, quarterViewHeight*3 - (lastValueDrv * quarterViewHeight),
                        fx,   quarterViewHeight*3 - (valueDrv * quarterViewHeight));

            lastValue = value;
            lastValueDrv = valueDrv;
        }
    }

    g.setColour(COL_FREQVIEW_RECT_DARK);
    g.drawRect(0, 0, getWidth(), getHeight());
    g.setColour(COL_FREQVIEW_RECT_LIGHT);
    g.drawRect(1, 1, getWidth()-2, getHeight()-2);
}

//==============================================================================
Tunefish4AudioProcessorEditor::Tunefish4AudioProcessorEditor (Tunefish4AudioProcessor* ownerFilter, eTfSynth *synth)
    : AudioProcessorEditor (ownerFilter),
    m_wasWindowHidden(true),
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
    m_imgShapeNoise(Image::ARGB, PIXWIDTH, PIXHEIGHT, true),
    m_meter(*ownerFilter, 2, 0),
    m_midiKeyboard(ownerFilter->keyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
	setLookAndFeel(TfLookAndFeel::getInstance());
    setSize(1080, 800);

    // Init configuration 
    // -------------------------------------------------------------------------------------------
    PropertiesFile::Options storageParams;
    storageParams.applicationName = JucePlugin_Name;
    storageParams.filenameSuffix = ".settings";
    storageParams.folderName = "Brain Control";
    storageParams.osxLibrarySubFolder = "Application Support";
    storageParams.doNotSave = false;

    m_appProperties.setStorageParameters(storageParams);
                   
    _createIcons();

    // -------------------------------------
    //  GLOBAL GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpGlobal, "GLOBAL", 10, 0, 1060, 80);
    
    m_grpGlobal.addChildComponent(m_meter);
    m_meter.setVisible(true);
    m_meter.setBounds(8, 28, 64, 40);
    getProcessor()->setMetering(true);

    _addComboBox(&m_grpGlobal, m_cmbPolyphony, "1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16", 75, 45, 40, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPolyphony, "Voices:", 70, 25, 40, 20);

    _addComboBox(&m_grpGlobal, m_cmbPitchBendUp, "1|2|3|4|5|6|7|8|9|10|11|12", 1005, 25, 45, 20);
    _addComboBox(&m_grpGlobal, m_cmbPitchBendDown, "1|2|3|4|5|6|7|8|9|10|11|12", 1005, 47, 45, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPitchBendUp, "Pitch up:", 955, 25, 50, 20);
    _addLabel(&m_grpGlobal, m_lblGlobPitchBendDown, "Pitch down:", 955, 47, 50, 20);

    _addTextToggleButton(this, m_btnAnimationsOn, "Animations on", "", 850, 20, 100, 17);
    _addTextToggleButton(this, m_btnFastAnimations, "Fast animations", "", 850, 38, 100, 17);
    _addTextToggleButton(this, m_btnMovingWaveforms, "Moving waveforms", "", 850, 56, 100, 17);

    m_btnAnimationsOn.setToggleState(_configAreAnimationsOn(), dontSendNotification);
    m_btnFastAnimations.setToggleState(_configAreAnimationsFast(), dontSendNotification);
    m_btnMovingWaveforms.setToggleState(_configAreWaveformsMoving(), dontSendNotification);

    _addRotarySlider(&m_grpGlobal, m_sldGlobVolume, m_lblGlobVolume, "Volume", 110, 25, COL_ROTARYSLIDER_GLOBAL);
    _addRotarySlider(&m_grpGlobal, m_sldGlobFrequency, m_lblGlobFrequency, "Frequency", 170, 25, COL_ROTARYSLIDER_GLOBAL);
    _addRotarySlider(&m_grpGlobal, m_sldGlobDetune, m_lblGlobDetune, "Detune", 230, 25, COL_ROTARYSLIDER_GLOBAL);
    _addRotarySlider(&m_grpGlobal, m_sldGlobSlop, m_lblGlobSlop, "Slop", 290, 25, COL_ROTARYSLIDER_GLOBAL);
    _addRotarySlider(&m_grpGlobal, m_sldGlobGlide, m_lblGlobGlide, "Glide", 350, 25, COL_ROTARYSLIDER_GLOBAL);
    _addEditableComboBox(&m_grpGlobal, m_cmbInstrument, "", 410, 25, 160, 20);
    _addTextButton(&m_grpGlobal, m_btnSave, "Save", 410, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnRestore, "Restore", 465, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnPrev, "Prev", 520, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnNext, "Next", 575, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnCopy, "Copy", 630, 50, 50, 20);
    _addTextButton(&m_grpGlobal, m_btnPaste, "Paste", 685, 50, 50, 20);

    _addTextButton(&m_grpGlobal, m_btnAbout, String("Tunefish ") + JucePlugin_VersionString, 575, 25, 160, 20);

#ifdef HAVE_RECORDING
    _addTextToggleButton(&m_grpGlobal, m_btnRecord, "Record", "", 745, 20, 90, 17);
#endif

#ifdef HAVE_PATCH_WRITER
    _addTextButton(&m_grpGlobal, m_btnFactoryWriter, "Factory writer", 745, 38, 90, 17);
#endif

#ifdef HAVE_PATCH_LOADER
	_addTextButton(&m_grpGlobal, m_btnPresetFileLoader, "Load preset", 745, 56, 90, 17);
#endif

    // -------------------------------------
    //  GENERATOR GROUP
    // -------------------------------------
    _addGroupBox(this, m_grpGenerator, "GENERATOR", 10, 80, 590, 370);
    _addRotarySlider(&m_grpGenerator, m_sldGenVolume, m_lblGenVolume, "Volume", 10, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenPanning, m_lblGenPanning, "Panning", 70, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenSpread, m_lblGenSpread, "Spread", 130, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenBandwidth, m_lblGenBandwidth, "Bandwidth", 190, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenDamp, m_lblGenDamp, "Damp", 250, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenHarmonics, m_lblGenHarmonics, "Harmonics", 310, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenDrive, m_lblGenDrive, "Drive", 370, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenScale, m_lblGenScale, "Scale", 430, 25, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldGenModulation, m_lblGenModulation, "Modulation", 490, 25, COL_ROTARYSLIDER_GEN);

    _addRotarySlider(&m_grpGenerator, m_sldNoiseAmount, m_lblNoiseAmount, "Noise", 370, 85, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldNoiseFreq, m_lblNoiseFreq, "Noise Freq", 430, 85, COL_ROTARYSLIDER_GEN);
    _addRotarySlider(&m_grpGenerator, m_sldNoiseBandwidth, m_lblNoiseBandwidth, "Noise BW", 490, 85, COL_ROTARYSLIDER_GEN);

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
    m_freqView.setBounds(10, 140, 570, 210);
    m_freqView.setSynth(ownerFilter, synth, synth->instr[0]);

    // -------------------------------------
    //  FILTER GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpLPFilter, "LOWPASS", 10, 450, 140, 100);
    _addTextToggleButton(nullptr, m_btnLPOn, "On", "", 15, 470, 25, 20);
    _addRotarySlider(&m_grpLPFilter, m_sldLPFrequency, m_lblLPFrequency, "Frequency", 10, 40, COL_ROTARYSLIDER_FILTER);
    _addRotarySlider(&m_grpLPFilter, m_sldLPResonance, m_lblLPResonance, "Resonance", 70, 40, COL_ROTARYSLIDER_FILTER);

    _addGroupBox(this, m_grpHPFilter, "HIGHPASS", 160, 450, 140, 100);
    _addTextToggleButton(nullptr, m_btnHPOn, "On", "", 165, 470, 25, 20);
    _addRotarySlider(&m_grpHPFilter, m_sldHPFrequency, m_lblHPFrequency, "Frequency", 10, 40, COL_ROTARYSLIDER_FILTER);
    _addRotarySlider(&m_grpHPFilter, m_sldHPResonance, m_lblHPResonance, "Resonance", 70, 40, COL_ROTARYSLIDER_FILTER);

    _addGroupBox(this, m_grpBPFilter, "BANDPASS", 310, 450, 140, 100);
    _addTextToggleButton(nullptr, m_btnBPOn, "On", "", 315, 470, 25, 20);
    _addRotarySlider(&m_grpBPFilter, m_sldBPFrequency, m_lblBPFrequency, "Frequency", 10, 40, COL_ROTARYSLIDER_FILTER);
    _addRotarySlider(&m_grpBPFilter, m_sldBPQ, m_lblBPQ, "Q", 70, 40, COL_ROTARYSLIDER_FILTER);

    _addGroupBox(this, m_grpNTFilter, "NOTCH", 460, 450, 140, 100);
    _addTextToggleButton(nullptr, m_btnNTOn, "On", "", 465, 470, 25, 20);
    _addRotarySlider(&m_grpNTFilter, m_sldNTFrequency, m_lblNTFrequency, "Frequency", 10, 40, COL_ROTARYSLIDER_FILTER);
    _addRotarySlider(&m_grpNTFilter, m_sldNTQ, m_lblNTQ, "Q", 70, 40, COL_ROTARYSLIDER_FILTER);

    // -------------------------------------
    //  LFO/ADSR GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpLFO1, "LFO1", 10, 550, 140, 160);
    _addRotarySlider(&m_grpLFO1, m_sldLFO1Rate, m_lblLFO1Rate, "Rate", 10, 25, COL_ROTARYSLIDER_LFO);
    _addRotarySlider(&m_grpLFO1, m_sldLFO1Depth, m_lblLFO1Depth, "Depth", 70, 25, COL_ROTARYSLIDER_LFO);
    _addTextToggleButton(&m_grpLFO1, m_btnLFO1Sync, "Synchronized", "", 10, 130, 125, 20);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSine, m_dimgShapeSine, "lfo1type", 10, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSawUp, m_dimgShapeSawUp, "lfo1type", 35, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSawDown, m_dimgShapeSawDown, "lfo1type", 60, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeSquare, m_dimgShapeSquare, "lfo1type", 85, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO1, m_btnLFO1ShapeNoise, m_dimgShapeNoise, "lfo1type", 110, 95, 25, 25);

    _addGroupBox(this, m_grpLFO2, "LFO2", 160, 550, 140, 160);
    _addRotarySlider(&m_grpLFO2, m_sldLFO2Rate, m_lblLFO2Rate, "Rate", 10, 25, COL_ROTARYSLIDER_LFO);
    _addRotarySlider(&m_grpLFO2, m_sldLFO2Depth, m_lblLFO2Depth, "Depth", 70, 25, COL_ROTARYSLIDER_LFO);
    _addTextToggleButton(&m_grpLFO2, m_btnLFO2Sync, "Synchronized", "", 10, 130, 125, 20);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSine, m_dimgShapeSine, "lfo2type", 10, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSawUp, m_dimgShapeSawUp, "lfo2type", 35, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSawDown, m_dimgShapeSawDown, "lfo2type", 60, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeSquare, m_dimgShapeSquare, "lfo2type", 85, 95, 25, 25);
    _addImageToggleButton(&m_grpLFO2, m_btnLFO2ShapeNoise, m_dimgShapeNoise, "lfo2type", 110, 95, 25, 25);

    _addGroupBox(this, m_grpADSR1, "ADSR1", 310, 550, 140, 160);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Attack, m_lblADSR1Attack, "A", 10, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Decay, m_lblADSR1Decay, "D", 35, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Sustain, m_lblADSR1Sustain, "S", 60, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Release, m_lblADSR1Release, "R", 85, 25, 20, 120);
    _addLinearSlider(&m_grpADSR1, m_sldADSR1Slope, m_lblADSR1Slope, "Slp", 110, 25, 20, 120);

    _addGroupBox(this, m_grpADSR2, "ADSR2", 460, 550, 140, 160);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Attack, m_lblADSR2Attack, "A", 10, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Decay, m_lblADSR2Decay, "D", 35, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Sustain, m_lblADSR2Sustain, "S", 60, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Release, m_lblADSR2Release, "R", 85, 25, 20, 120);
    _addLinearSlider(&m_grpADSR2, m_sldADSR2Slope, m_lblADSR2Slope, "Slp", 110, 25, 20, 120);

    // -------------------------------------
    //  FX GROUPS
    // -------------------------------------
    _addGroupBox(this, m_grpFxFlanger, "FLANGER", 610, 80, 270, 90);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerLFO, m_lblFlangerLFO, "LFO", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerFrequency, m_lblFlangerFrequency, "Frequency", 70, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerAmplitude, m_lblFlangerAmplitude, "Amplitude", 130, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxFlanger, m_sldFlangerWet, m_lblFlangerWet, "Wet", 190, 25, COL_ROTARYSLIDER_EFFECTS);

    _addGroupBox(this, m_grpFxReverb, "REVERB", 610, 170, 270, 90);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbRoomsize, m_lblReverbRoomsize, "Roomsize", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbDamp, m_lblReverbDamp, "Damp", 70, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbWet, m_lblReverbWet, "Wet", 130, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxReverb, m_sldReverbWidth, m_lblReverbWidth, "Width", 190, 25, COL_ROTARYSLIDER_EFFECTS);

    _addGroupBox(this, m_grpFxDelay, "DELAY", 610, 260, 270, 90);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayLeft, m_lblDelayLeft, "Left", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayRight, m_lblDelayRight, "Right", 70, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxDelay, m_sldDelayDecay, m_lblDelayDecay, "Decay", 130, 25, COL_ROTARYSLIDER_EFFECTS);

    _addGroupBox(this, m_grpFxEQ, "EQ", 610, 350, 270, 90);
    _addRotarySlider(&m_grpFxEQ, m_sldEqLow, m_lblEqLow, "-880hz", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxEQ, m_sldEqMid, m_lblEqMid, "880hz-5khz", 70, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxEQ, m_sldEqHigh, m_lblEqHigh, "5khz-", 130, 25, COL_ROTARYSLIDER_EFFECTS);

    _addGroupBox(this, m_grpFxChorus, "CHORUS", 610, 440, 270, 90);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusFreq, m_lblChorusFreq, "Frequency", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusDepth, m_lblChorusDepth, "Depth", 70, 25, COL_ROTARYSLIDER_EFFECTS);
    _addRotarySlider(&m_grpFxChorus, m_sldChorusGain, m_lblChorusGain, "Gain", 130, 25, COL_ROTARYSLIDER_EFFECTS);

    _addGroupBox(this, m_grpFxFormant, "FORMANT", 610, 530, 270, 90);
    _addRotarySlider(&m_grpFxFormant, m_sldFormantWet, m_lblFormantWet, "Wet", 10, 25, COL_ROTARYSLIDER_EFFECTS);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantA, "A", "formant", 70, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantE, "E", "formant", 95, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantI, "I", "formant", 120, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantO, "O", "formant", 145, 45, 25, 25);
    _addTextToggleButton(&m_grpFxFormant, m_btnFormantU, "U", "formant", 170, 45, 25, 25);

    _addGroupBox(this, m_grpFxDistortion, "DISTORTION", 610, 620, 270, 90);
    _addRotarySlider(&m_grpFxDistortion, m_sldDistortionAmount, m_lblDistortionAmount, "Amount", 10, 25, COL_ROTARYSLIDER_EFFECTS);

    // -------------------------------------
    //  MOD MATRIX GROUP
    // -------------------------------------
    
    _addGroupBox(this, m_grpModMatrix, "MOD MATRIX", 890, 80, 180, 360);
    _addComboBox(&m_grpModMatrix, m_cmbMM1Src, MOD_SOURCES, 10, 21, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM1Dest, MOD_TARGETS, 10, 41, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM1Mod, 130, 21, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM2Src, MOD_SOURCES, 10, 63, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM2Dest, MOD_TARGETS, 10, 83, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM2Mod, 130, 63, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM3Src, MOD_SOURCES, 10, 105, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM3Dest, MOD_TARGETS, 10, 125, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM3Mod, 130, 105, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM4Src, MOD_SOURCES, 10, 147, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM4Dest, MOD_TARGETS, 10, 167, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM4Mod, 130, 147, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM5Src, MOD_SOURCES, 10, 189, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM5Dest, MOD_TARGETS, 10, 209, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM5Mod, 130, 189, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM6Src, MOD_SOURCES, 10, 231, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM6Dest, MOD_TARGETS, 10, 251, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM6Mod, 130, 231, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM7Src, MOD_SOURCES, 10, 273, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM7Dest, MOD_TARGETS, 10, 293, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM7Mod, 130, 273, COL_ROTARYSLIDER_GLOBAL);
    _addComboBox(&m_grpModMatrix, m_cmbMM8Src, MOD_SOURCES, 10, 315, 110, 20);
    _addComboBox(&m_grpModMatrix, m_cmbMM8Dest, MOD_TARGETS, 10, 335, 110, 20);
    _addRotarySliderNoLabel(&m_grpModMatrix, m_sldMM8Mod, 130, 315, COL_ROTARYSLIDER_GLOBAL);

    // -------------------------------------
    //  EFFECT STACK GROUP
    // -------------------------------------
    
    _addGroupBox(this, m_grpEffectStack, "EFFECTS STACK", 890, 440, 180, 270);
    _addComboBox(&m_grpEffectStack, m_cmbEffect1, FX_SECTIONS, 10, 21, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect2, FX_SECTIONS, 10, 45, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect3, FX_SECTIONS, 10, 69, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect4, FX_SECTIONS, 10, 93, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect5, FX_SECTIONS, 10, 117, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect6, FX_SECTIONS, 10, 141, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect7, FX_SECTIONS, 10, 165, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect8, FX_SECTIONS, 10, 189, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect9, FX_SECTIONS, 10, 213, 160, 20);
    _addComboBox(&m_grpEffectStack, m_cmbEffect10, FX_SECTIONS, 10, 237, 160, 20);

    // -------------------------------------
    //  KEYBOARD
    // -------------------------------------
    addAndMakeVisible(&m_midiKeyboard);
    m_midiKeyboard.setBounds(10, getHeight() - 80, getWidth() - 20, 80);

    _fillProgramCombobox();
    _resetTimer();
}

Tunefish4AudioProcessorEditor::~Tunefish4AudioProcessorEditor()
{
	setLookAndFeel(nullptr);
	getProcessor()->setMetering(false);
    TfLookAndFeel::deleteInstance();
    Fonts::deleteInstance();
}

void Tunefish4AudioProcessorEditor::_addRotarySlider(Component *parent, Slider &slider, Label &label, String text, eU32 x, eU32 y, Colour colour)
{
    parent->addChildComponent(&slider);

    slider.setVisible(true);
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.addListener (this);
    slider.setRange (0.0, 1.0, 0.0);
    slider.setBounds (x, y, 60, 40);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    slider.setColour(Slider::rotarySliderFillColourId, colour);

    _addLabel(parent, label, text, x, y+30, 60, 20);
    label.setJustificationType(Justification::centredBottom);
}

void Tunefish4AudioProcessorEditor::_addRotarySliderNoLabel(Component *parent, Slider &slider, eU32 x, eU32 y, Colour colour)
{
    parent->addChildComponent(&slider);

    slider.setVisible(true);
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.addListener (this);
    slider.setRange (0.0, 1.0, 0.0);
    slider.setBounds (x, y, 40, 40);
    slider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
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

    _addLabel(parent, label, text, x, y+h-20, w, 20);
    label.setJustificationType(Justification::centredBottom);
}

void Tunefish4AudioProcessorEditor::_addGroupBox(Component *parent, GroupComponent &group, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&group);

    group.setVisible(true);
    group.setText(text);
    group.setBounds(x, y, w, h);
}

void Tunefish4AudioProcessorEditor::_addTextButton(Component *parent, TextButton &button, String text, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&button);

    button.setVisible(true);
    button.addListener(this);
    button.setButtonText(text);
    button.setBounds(x, y, w, h);
}

void Tunefish4AudioProcessorEditor::_addTextToggleButton(Component *parent, TextButton &button, String text, String group, eU32 x, eU32 y, eU32 w, eU32 h)
{
    if (parent)
        parent->addChildComponent(&button);
    else
        addAndMakeVisible(&button);

    button.setVisible(true);
    button.addListener(this);
    button.setClickingTogglesState(true);
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
    button.setImages(&image);
    button.setBounds(x, y, w, h);
}

void Tunefish4AudioProcessorEditor::_addComboBox(Component *parent, ComboBox &combobox, String items, eU32 x, eU32 y, eU32 w, eU32 h)
{
    parent->addChildComponent(&combobox);

    StringArray itemArray;
    itemArray.addTokens(items, "|", "");

    combobox.setVisible(true);
    combobox.addListener(this);
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
}

void Tunefish4AudioProcessorEditor::_fillProgramCombobox()
{
    m_cmbInstrument.clear(dontSendNotification);

    const eU32 programCount = getProcessor()->getNumPrograms();
    const eU32 currentProgram = getProcessor()->getCurrentProgram();

    for (eU32 i=0; i<programCount; i++)
    {
        auto name = getProcessor()->getProgramName(i);
        if (name.isEmpty())
            name = "Untitled";
        
        m_cmbInstrument.addItem(name, i+1);
    }

    m_cmbInstrument.setSelectedItemIndex(currentProgram, dontSendNotification);
}

//==============================================================================
void Tunefish4AudioProcessorEditor::paint (Graphics& g)
{
    g.setColour(COL_WINDOW_BG);
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
    Tunefish4AudioProcessor * tfprocessor = getProcessor();
    bool animationsOn = _configAreAnimationsOn();
    bool waveformsMoving = _configAreWaveformsMoving();

    bool parametersChanged = tfprocessor->wasProgramSwitched() || tfprocessor->isParamDirtyAny();

    m_meter.refreshDisplayIfNeeded();
    
    if (animationsOn || parametersChanged || m_wasWindowHidden)
    {
        // if the window was previously hidden, we need to fill all those comboboxes again
        if (m_wasWindowHidden)
        {
            tfprocessor->resetParamDirty(eTRUE);
        }
        
        if (tfprocessor->wasProgramSwitched())
            m_cmbInstrument.setSelectedItemIndex(tfprocessor->getCurrentProgram(), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_GEN_POLYPHONY))
            m_cmbPolyphony.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_GEN_POLYPHONY) * (TF_MAXVOICES - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_PITCHWHEEL_UP))
            m_cmbPitchBendUp.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_PITCHWHEEL_UP) * (TF_MAXPITCHBEND / 2))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_PITCHWHEEL_DOWN))
            m_cmbPitchBendDown.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_PITCHWHEEL_DOWN) * (TF_MAXPITCHBEND / 2))), dontSendNotification);

        m_sldGlobVolume.setValue(tfprocessor->getParameter(TF_GLOBAL_GAIN), dontSendNotification);
        m_sldGlobFrequency.setValue(tfprocessor->getParameter(TF_GEN_FREQ), dontSendNotification);
        m_sldGlobDetune.setValue(tfprocessor->getParameter(TF_GEN_DETUNE), dontSendNotification);
        m_sldGlobSlop.setValue(tfprocessor->getParameter(TF_GEN_SLOP), dontSendNotification);
        m_sldGlobGlide.setValue(tfprocessor->getParameter(TF_GEN_GLIDE), dontSendNotification);

        m_sldGenVolume.setValue(tfprocessor->getParameter(TF_GEN_VOLUME), dontSendNotification);
        m_sldGenPanning.setValue(tfprocessor->getParameter(TF_GEN_PANNING), dontSendNotification);
        m_sldGenSpread.setValue(tfprocessor->getParameter(TF_GEN_SPREAD), dontSendNotification);
        m_sldGenBandwidth.setValue(tfprocessor->getParameter(TF_GEN_BANDWIDTH), dontSendNotification);
        m_sldGenDamp.setValue(tfprocessor->getParameter(TF_GEN_DAMP), dontSendNotification);
        m_sldGenHarmonics.setValue(tfprocessor->getParameter(TF_GEN_NUMHARMONICS), dontSendNotification);
        m_sldGenDrive.setValue(tfprocessor->getParameter(TF_GEN_DRIVE), dontSendNotification);
        m_sldGenScale.setValue(tfprocessor->getParameter(TF_GEN_SCALE), dontSendNotification);
        m_sldGenModulation.setValue(tfprocessor->getParameter(TF_GEN_MODULATION), dontSendNotification);

        m_sldNoiseAmount.setValue(tfprocessor->getParameter(TF_NOISE_AMOUNT), dontSendNotification);
        m_sldNoiseFreq.setValue(tfprocessor->getParameter(TF_NOISE_FREQ), dontSendNotification);
        m_sldNoiseBandwidth.setValue(tfprocessor->getParameter(TF_NOISE_BW), dontSendNotification);

        m_btnLPOn.setToggleState(tfprocessor->getParameter(TF_LP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldLPFrequency.setValue(tfprocessor->getParameter(TF_LP_FILTER_CUTOFF), dontSendNotification);
        m_sldLPResonance.setValue(tfprocessor->getParameter(TF_LP_FILTER_RESONANCE), dontSendNotification);
        m_btnHPOn.setToggleState(tfprocessor->getParameter(TF_HP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldHPFrequency.setValue(tfprocessor->getParameter(TF_HP_FILTER_CUTOFF), dontSendNotification);
        m_sldHPResonance.setValue(tfprocessor->getParameter(TF_HP_FILTER_RESONANCE), dontSendNotification);
        m_btnBPOn.setToggleState(tfprocessor->getParameter(TF_BP_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldBPFrequency.setValue(tfprocessor->getParameter(TF_BP_FILTER_CUTOFF), dontSendNotification);
        m_sldBPQ.setValue(tfprocessor->getParameter(TF_BP_FILTER_Q), dontSendNotification);
        m_btnNTOn.setToggleState(tfprocessor->getParameter(TF_NT_FILTER_ON) > 0.5f, dontSendNotification);
        m_sldNTFrequency.setValue(tfprocessor->getParameter(TF_NT_FILTER_CUTOFF), dontSendNotification);
        m_sldNTQ.setValue(tfprocessor->getParameter(TF_NT_FILTER_Q), dontSendNotification);

        m_sldLFO1Rate.setValue(tfprocessor->getParameter(TF_LFO1_RATE), dontSendNotification);
        m_sldLFO1Depth.setValue(tfprocessor->getParameter(TF_LFO1_DEPTH), dontSendNotification);
        m_sldLFO2Rate.setValue(tfprocessor->getParameter(TF_LFO2_RATE), dontSendNotification);
        m_sldLFO2Depth.setValue(tfprocessor->getParameter(TF_LFO2_DEPTH), dontSendNotification);

        m_sldADSR1Attack.setValue(tfprocessor->getParameter(TF_ADSR1_ATTACK), dontSendNotification);
        m_sldADSR1Decay.setValue(tfprocessor->getParameter(TF_ADSR1_DECAY), dontSendNotification);
        m_sldADSR1Sustain.setValue(tfprocessor->getParameter(TF_ADSR1_SUSTAIN), dontSendNotification);
        m_sldADSR1Release.setValue(tfprocessor->getParameter(TF_ADSR1_RELEASE), dontSendNotification);
        m_sldADSR1Slope.setValue(tfprocessor->getParameter(TF_ADSR1_SLOPE), dontSendNotification);

        m_sldADSR2Attack.setValue(tfprocessor->getParameter(TF_ADSR2_ATTACK), dontSendNotification);
        m_sldADSR2Decay.setValue(tfprocessor->getParameter(TF_ADSR2_DECAY), dontSendNotification);
        m_sldADSR2Sustain.setValue(tfprocessor->getParameter(TF_ADSR2_SUSTAIN), dontSendNotification);
        m_sldADSR2Release.setValue(tfprocessor->getParameter(TF_ADSR2_RELEASE), dontSendNotification);
        m_sldADSR2Slope.setValue(tfprocessor->getParameter(TF_ADSR2_SLOPE), dontSendNotification);

        m_sldFlangerFrequency.setValue(tfprocessor->getParameter(TF_FLANGER_FREQUENCY), dontSendNotification);
        m_sldFlangerAmplitude.setValue(tfprocessor->getParameter(TF_FLANGER_AMPLITUDE), dontSendNotification);
        m_sldFlangerLFO.setValue(tfprocessor->getParameter(TF_FLANGER_LFO), dontSendNotification);
        m_sldFlangerWet.setValue(tfprocessor->getParameter(TF_FLANGER_WET), dontSendNotification);

        m_sldReverbRoomsize.setValue(tfprocessor->getParameter(TF_REVERB_ROOMSIZE), dontSendNotification);
        m_sldReverbDamp.setValue(tfprocessor->getParameter(TF_REVERB_DAMP), dontSendNotification);
        m_sldReverbWet.setValue(tfprocessor->getParameter(TF_REVERB_WET), dontSendNotification);
        m_sldReverbWidth.setValue(tfprocessor->getParameter(TF_REVERB_WIDTH), dontSendNotification);

        m_sldDelayLeft.setValue(tfprocessor->getParameter(TF_DELAY_LEFT), dontSendNotification);
        m_sldDelayRight.setValue(tfprocessor->getParameter(TF_DELAY_RIGHT), dontSendNotification);
        m_sldDelayDecay.setValue(tfprocessor->getParameter(TF_DELAY_DECAY), dontSendNotification);

        m_sldChorusFreq.setValue(tfprocessor->getParameter(TF_CHORUS_RATE), dontSendNotification);
        m_sldChorusDepth.setValue(tfprocessor->getParameter(TF_CHORUS_DEPTH), dontSendNotification);
        m_sldChorusGain.setValue(tfprocessor->getParameter(TF_CHORUS_GAIN), dontSendNotification);

        m_sldEqLow.setValue(tfprocessor->getParameter(TF_EQ_LOW), dontSendNotification);
        m_sldEqMid.setValue(tfprocessor->getParameter(TF_EQ_MID), dontSendNotification);
        m_sldEqHigh.setValue(tfprocessor->getParameter(TF_EQ_HIGH), dontSendNotification);

        m_sldFormantWet.setValue(tfprocessor->getParameter(TF_FORMANT_WET), dontSendNotification);

        m_sldDistortionAmount.setValue(tfprocessor->getParameter(TF_DISTORT_AMOUNT), dontSendNotification);

        // MOD Matrix Slot 1 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM1_SOURCE))
            m_cmbMM1Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM1_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM1_TARGET))
            m_cmbMM1Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM1_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM1Mod.setValue(tfprocessor->getParameter(TF_MM1_MOD), dontSendNotification);

        // MOD Matrix Slot 2 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM2_SOURCE))
            m_cmbMM2Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM2_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM2_TARGET))
            m_cmbMM2Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM2_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM2Mod.setValue(tfprocessor->getParameter(TF_MM2_MOD), dontSendNotification);

        // MOD Matrix Slot 3 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM3_SOURCE))
            m_cmbMM3Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM3_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM3_TARGET))
            m_cmbMM3Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM3_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM3Mod.setValue(tfprocessor->getParameter(TF_MM3_MOD), dontSendNotification);

        // MOD Matrix Slot 4 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM4_SOURCE))
            m_cmbMM4Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM4_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM4_TARGET))
            m_cmbMM4Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM4_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM4Mod.setValue(tfprocessor->getParameter(TF_MM4_MOD), dontSendNotification);

        // MOD Matrix Slot 5 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM5_SOURCE))
            m_cmbMM5Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM5_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM5_TARGET))
            m_cmbMM5Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM5_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM5Mod.setValue(tfprocessor->getParameter(TF_MM5_MOD), dontSendNotification);

        // MOD Matrix Slot 6 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM6_SOURCE))
            m_cmbMM6Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM6_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM6_TARGET))
            m_cmbMM6Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM6_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM6Mod.setValue(tfprocessor->getParameter(TF_MM6_MOD), dontSendNotification);

        // MOD Matrix Slot 7 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM7_SOURCE))
            m_cmbMM7Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM7_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM7_TARGET))
            m_cmbMM7Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM7_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM7Mod.setValue(tfprocessor->getParameter(TF_MM7_MOD), dontSendNotification);

        // MOD Matrix Slot 8 --------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_MM8_SOURCE))
            m_cmbMM8Src.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM8_SOURCE) * (eTfModMatrix::INPUT_COUNT - 1))), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_MM8_TARGET))
            m_cmbMM8Dest.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_MM8_TARGET) * (eTfModMatrix::OUTPUT_COUNT - 1))), dontSendNotification);

        m_sldMM8Mod.setValue(tfprocessor->getParameter(TF_MM8_MOD), dontSendNotification);

        // EFFECTS Section ----------------------------------------------------------------
        if (tfprocessor->isParamDirty(TF_EFFECT_1))
            m_cmbEffect1.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_1) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_2))
            m_cmbEffect2.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_2) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_3))
            m_cmbEffect3.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_3) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_4))
            m_cmbEffect4.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_4) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_5))
            m_cmbEffect5.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_5) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_6))
            m_cmbEffect6.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_6) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_7))
            m_cmbEffect7.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_7) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_8))
            m_cmbEffect8.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_8) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_9))
            m_cmbEffect9.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_9) * TF_MAXEFFECTS)), dontSendNotification);

        if (tfprocessor->isParamDirty(TF_EFFECT_10))
            m_cmbEffect10.setSelectedItemIndex(static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_EFFECT_10) * TF_MAXEFFECTS)), dontSendNotification);

        switch (static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_GEN_UNISONO) * (TF_MAXUNISONO - 1))))
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

        switch (static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_GEN_OCTAVE) * (TF_MAXOCTAVES - 1))))
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

        switch (static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_LFO1_SHAPE) * (TF_LFOSHAPECOUNT - 1))))
        {
        case 0: m_btnLFO1ShapeSine.setToggleState(true, dontSendNotification); break;
        case 1: m_btnLFO1ShapeSawDown.setToggleState(true, dontSendNotification); break;
        case 2: m_btnLFO1ShapeSawUp.setToggleState(true, dontSendNotification); break;
        case 3: m_btnLFO1ShapeSquare.setToggleState(true, dontSendNotification); break;
        case 4: m_btnLFO1ShapeNoise.setToggleState(true, dontSendNotification); break;
        }
        m_btnLFO1Sync.setToggleState(tfprocessor->getParameter(TF_LFO1_SYNC) > 0.5, dontSendNotification);

        switch (static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_LFO2_SHAPE) * (TF_LFOSHAPECOUNT - 1))))
        {
        case 0: m_btnLFO2ShapeSine.setToggleState(true, dontSendNotification); break;
        case 1: m_btnLFO2ShapeSawDown.setToggleState(true, dontSendNotification); break;
        case 2: m_btnLFO2ShapeSawUp.setToggleState(true, dontSendNotification); break;
        case 3: m_btnLFO2ShapeSquare.setToggleState(true, dontSendNotification); break;
        case 4: m_btnLFO2ShapeNoise.setToggleState(true, dontSendNotification); break;
        }
        m_btnLFO2Sync.setToggleState(tfprocessor->getParameter(TF_LFO2_SYNC) > 0.5, dontSendNotification);

        switch (static_cast<eU32>(eRoundNearest(tfprocessor->getParameter(TF_FORMANT_MODE) * (TF_FORMANTCOUNT - 1))))
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

        m_sldGenVolume.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_VOLUME));
        m_sldGenSpread.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_SPREAD));
        m_sldGenScale.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_SCALE));
        m_sldGenPanning.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_PAN));
        m_sldGenHarmonics.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_NUMHARMONICS));
        m_sldNTQ.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_NT_FILTER_Q));
        m_sldNTFrequency.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_NT_FILTER_CUTOFF));
        m_sldNoiseAmount.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_NOISE_AMOUNT));
        m_sldMM8Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD8));
        m_sldMM7Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD7));
        m_sldMM6Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD6));
        m_sldMM5Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD5));
        m_sldMM4Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD4));
        m_sldMM3Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD3));
        m_sldMM2Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD2));
        m_sldMM1Mod.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_MOD1));
        m_sldLPResonance.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_LP_FILTER_RESONANCE));
        m_sldLPFrequency.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_LP_FILTER_CUTOFF));
        m_sldHPResonance.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_HP_FILTER_RESONANCE));
        m_sldHPFrequency.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_HP_FILTER_CUTOFF));
        m_sldGlobFrequency.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_FREQ));
        m_sldGenDrive.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_DRIVE));
        m_sldGlobDetune.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_DETUNE));
        m_sldGenDamp.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_DAMP));
        m_sldBPQ.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_BP_FILTER_Q));
        m_sldBPFrequency.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_BP_FILTER_CUTOFF));
        m_sldGenBandwidth.setModValue(tfprocessor->getParameterMod(eTfModMatrix::OUTPUT_BANDWIDTH));
    }

    if ((animationsOn && waveformsMoving) || tfprocessor->wasProgramSwitched())
    {
        m_freqView.repaint();
    }

    if (parametersChanged || m_wasWindowHidden)
    {
        m_grpLPFilter.setEnabled(tfprocessor->getParameter(TF_LP_FILTER_ON) > 0.5);
        m_grpHPFilter.setEnabled(tfprocessor->getParameter(TF_HP_FILTER_ON) > 0.5);
        m_grpBPFilter.setEnabled(tfprocessor->getParameter(TF_BP_FILTER_ON) > 0.5);
        m_grpNTFilter.setEnabled(tfprocessor->getParameter(TF_NT_FILTER_ON) > 0.5);

        m_grpFxDistortion.setEnabled(_isEffectUsed(1));
        m_grpFxDelay.setEnabled(_isEffectUsed(2));
        m_grpFxChorus.setEnabled(_isEffectUsed(3));
        m_grpFxFlanger.setEnabled(_isEffectUsed(4));
        m_grpFxReverb.setEnabled(_isEffectUsed(5));
        m_grpFxFormant.setEnabled(_isEffectUsed(6));
        m_grpFxEQ.setEnabled(_isEffectUsed(7));

        m_grpLFO1.setEnabled(_isModulatorUsed(1));
        m_grpLFO2.setEnabled(_isModulatorUsed(2));
        m_grpADSR1.setEnabled(_isModulatorUsed(3));
        m_grpADSR2.setEnabled(_isModulatorUsed(4));
    }

    tfprocessor->resetParamDirty();
    m_wasWindowHidden = false;
}

void Tunefish4AudioProcessorEditor::_setParameterNotifyingHost(Slider *slider, eTfParam param)
{
    getProcessor()->setParameterNotifyingHost(param, static_cast<float>(slider->getValue()));

    if (param == TF_GEN_BANDWIDTH ||
        param == TF_GEN_DAMP ||
        param == TF_GEN_SPREAD ||
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
    else if (button == &m_btnLFO1Sync)          _setParameterNotifyingHost((button->getToggleState() ? 1.0f : 0.0f), TF_LFO1_SYNC);

    else if (button == &m_btnLFO2ShapeSine)     _setParameterNotifyingHost(fromIndex(0, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSawDown)  _setParameterNotifyingHost(fromIndex(1, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSawUp)    _setParameterNotifyingHost(fromIndex(2, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeSquare)   _setParameterNotifyingHost(fromIndex(3, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2ShapeNoise)    _setParameterNotifyingHost(fromIndex(4, 0, TF_LFOSHAPECOUNT-1), TF_LFO2_SHAPE);
    else if (button == &m_btnLFO2Sync)          _setParameterNotifyingHost((button->getToggleState() ? 1.0f : 0.0f), TF_LFO2_SYNC);

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
        _fillProgramCombobox();
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
            if (AlertWindow::showOkCancelBox(AlertWindow::InfoIcon, "Recorder", "This will record all Tunefish MIDI events. Clicking again will stop recording and allow you to save the recording to Tunefish native .TFM file format. Do you want to start recording?"))
            {
                recorder.reset();
                recorder.startRecording();
            }  
            else
            {
                m_btnRecord.setToggleState(false, dontSendNotification);
            }
        }
    }
    else if (button == &m_btnFactoryWriter)
    {
		if (AlertWindow::showOkCancelBox(AlertWindow::InfoIcon, "FactoryWriter", "This will write all current presets into a C/C++ header file. This is for developent purposes only! Do you want to continue?"))
		{
			FileChooser myChooser("Please select a file to save to", File::getSpecialLocation(File::userHomeDirectory), "*.hpp");
			if (myChooser.browseForFileToSave(true))
			{
				File file = myChooser.getResult();
				tfProcessor->writeFactoryPatchHeader(file);
			}
		}
    }
	else if (button == &m_btnPresetFileLoader)
	{
		FileChooser myChooser("Please select a preset file to load", File::getSpecialLocation(File::userHomeDirectory), "*.txt");
		if (myChooser.browseForFileToOpen())
		{
			File file = myChooser.getResult();
			if (tfProcessor->loadPresetFile(file, true))
			{
				auto selectedId = m_cmbInstrument.getSelectedId();
				m_cmbInstrument.changeItemText(selectedId, tfProcessor->getCurrentProgramName());
				refreshUiFromSynth();
			}
		}
	}
    else if (button == &m_btnAbout)
    {
        AboutComponent::openAboutWindow(this);
    }
}

bool Tunefish4AudioProcessorEditor::_isModulatorUsed(eU32 mod) const
{
    Tunefish4AudioProcessor * tfprocessor = getProcessor();

    for (int slot = TF_MM1_SOURCE; slot <= TF_MM10_SOURCE; slot+=3)
    {
        // "none|LFO1|LFO2|ADSR1|ADSR2"
        auto value = tfprocessor->getParameter(slot);
        auto intValue = static_cast<eU32>(round(value * eTfModMatrix::INPUT_COUNT));

        if (intValue == mod)
            return true;
    }

    return false;
}

bool Tunefish4AudioProcessorEditor::_isEffectUsed(eU32 effectNum) const
{
    Tunefish4AudioProcessor * tfprocessor = getProcessor();

    for (int fxSlot = TF_EFFECT_1; fxSlot <= TF_EFFECT_10; ++fxSlot)
    {
        // "none|Distortion|Delay|Chorus|Flanger|Reverb|Formant|EQ"
        auto value = tfprocessor->getParameter(fxSlot);
        auto intValue = static_cast<eU32>(round(value * TF_MAXEFFECTS));

        if (intValue == effectNum)
            return true;
    }

    return false;
}

void Tunefish4AudioProcessorEditor::_createIcons()
{
    const float MAX_X = static_cast<float>(PIXWIDTH-1);
    const float MAX_Y = static_cast<float>(PIXHEIGHT-1);

    // sine
    // ------------------------------------------
    float old = 0.0f;
    Graphics gSine(m_imgShapeSine);
    gSine.setColour(Colours::white);
    for (eU32 i=0;i<PIXWIDTH;i++)
    {
        float fi = static_cast<float>(i);
        float sine = eSin(fi / PIXWIDTH * ePI*2) * PIXHEIGHT/2;
        if (i>0)
            gSine.drawLine(fi - 1.0f, old+PIXHEIGHT/2, fi, sine+PIXHEIGHT/2);
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
        float fi = static_cast<float>(i);
        gNoise.drawLine(fi, MAX_Y/2.0f, fi, random.NextFloat(0.0f, MAX_Y));
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



/**************************************************************************************
*      AboutComponent
**************************************************************************************/


AboutComponent::AboutComponent() :
    link1("braincontrol.org", URL("http://www.braincontrol.org")),
    link2("tunefish-synth.com", URL("https://www.tunefish-synth.com"))
{
    String pluginName(JucePlugin_Name);

    text1.setJustification(Justification::centred);
    text1.append(pluginName + "\n",
        Fonts::getInstance()->title(),
        Colours::black);
    text1.append(String("Version ") + JucePlugin_VersionString,
        Fonts::getInstance()->normal(),
        Colours::black);

    text2.setJustification(Justification::centred);
    text2.append(String::fromUTF8("© 2018 Brain Control"),
        Fonts::getInstance()->normal(),
        Colours::black);

    text3.setJustification(Justification::left);
    text3.append(pluginName + " is free software under GPLv3. If you want to support the developer of this synth, please make a donation at tunefish-synth.com. " + pluginName + " comes with no warranty of any kind. Use at your own risk. Sources are available on GitHub. Please regard included license files.",
        Fonts::getInstance()->normal(),
        Colour(0xff555555));

    addAndMakeVisible(&link1);
    link1.setFont(Font(10.0f, Font::underlined), true);
    addAndMakeVisible(&link2);
    link2.setFont(Font(10.0f, Font::underlined), true);
}

void AboutComponent::paint(Graphics& g)
{
    g.fillAll(Colour(0xffebebeb));
    text1.draw(g, Rectangle<int>(0, 0, getWidth(), 32).toFloat());
    text2.draw(g, Rectangle<int>(0, 40, getWidth(), 32).toFloat());
    text3.draw(g, Rectangle<int>(0, 98, getWidth(), getHeight() - 100).toFloat());
}

void AboutComponent::resized()
{
    link2.setSize(100, 22);
    link2.changeWidthToFitText();
    link2.setTopLeftPosition((getWidth() - link2.getWidth()) / 2, getHeight() - link2.getHeight() - 10);

    link1.setSize(100, 22);
    link1.changeWidthToFitText();
    link1.setTopLeftPosition((getWidth() - link1.getWidth()) / 2, getHeight() - link1.getHeight() - 10 - link2.getHeight());
}

void AboutComponent::openAboutWindow(Component* parent)
{
    AlertWindow window("", "", AlertWindow::AlertIconType::NoIcon);
    AboutComponent comp;
    comp.setSize(300, 260);
    window.setLookAndFeel(TfLookAndFeel::getInstance());
    window.setUsingNativeTitleBar(true);
    window.addCustomComponent(&comp);
    window.addButton("Close", 1);

    if (parent)
    {
        Rectangle<int> bounds = parent->getScreenBounds();
        window.setCentrePosition(bounds.getCentreX(), bounds.getCentreY() - 90);
    }
    window.toFront(true);
    window.setVisible(true);

    window.setEscapeKeyCancels(true);
    window.runModalLoop();
}



