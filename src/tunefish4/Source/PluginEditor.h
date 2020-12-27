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

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "tflookandfeel.h"

class eTfGroupComponent : public GroupComponent
{
public:
    void paint (Graphics& g);
};

class eTfFreqView : public GroupComponent
{
public:
    eTfFreqView();
    ~eTfFreqView();

    void paint (Graphics& g);
    void setSynth(Tunefish4AudioProcessor *processor, eTfSynth *synth, eTfInstrument *instr);

private:
    eTfSynth *                  m_synth;
    eTfInstrument *             m_instr;
    eTfVoice *                  m_voice;
    Tunefish4AudioProcessor *   m_processor;
};

class eTfSlider : public Slider
{
public:
    eTfSlider() : Slider(), modValue(0.0f) { }

    eF32 getModValue() { return modValue; }
    void setModValue(eF32 value) { if (modValue != value) { modValue = value; this->repaint(); } }
private:
    eF32 modValue;
};

//==============================================================================
/**
 */
class Tunefish4AudioProcessorEditor  :
public AudioProcessorEditor,
public Slider::Listener,
public ComboBox::Listener,
public Button::Listener,
public Timer
{
public:
    Tunefish4AudioProcessorEditor (Tunefish4AudioProcessor* ownerFilter, eTfSynth *synth);
    ~Tunefish4AudioProcessorEditor();

    //==============================================================================
    // This is just a standard Juce paint method...
    void paint (Graphics& g) override;
    void sliderValueChanged (Slider* slider) override;
    void comboBoxChanged (ComboBox* comboBox) override;
    void buttonClicked (Button *button) override;
    void timerCallback() override;
    void visibilityChanged() override;
    void refreshUiFromSynth();

private:
    Tunefish4AudioProcessor * getProcessor() const
    {
        return static_cast <Tunefish4AudioProcessor*> (getAudioProcessor());
    }

    void _addRotarySlider(Component *parent, Slider &slider, Label &label, String text, eU32 x, eU32 y, Colour colour);
    void _addRotarySliderNoLabel(Component *parent, Slider &slider, eU32 x, eU32 y, Colour colour);
    void _addLinearSlider(Component *parent, Slider &slider, Label &label, String text, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addGroupBox(Component *parent, GroupComponent &group, String text, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addTextButton(Component *parent, TextButton &button, String text, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addTextToggleButton(Component *parent, TextButton &button, String text, String group, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addImageToggleButton(Component *parent, DrawableButton &button, DrawableImage &image, String group, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addComboBox(Component *parent, ComboBox &combobox, String items, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addEditableComboBox(Component *parent, ComboBox &combobox, String items, eU32 x, eU32 y, eU32 w, eU32 h);
    void _addLabel(Component *parent, Label &label, String text, eU32 x, eU32 y, eU32 w, eU32 h);

    void _setParameterNotifyingHost(Slider *slider, eTfParam param);
    void _setParameterNotifyingHost(ComboBox *comboBox, eU32 maxIndex, eTfParam param) const;
    void _setParameterNotifyingHost(Button *button, eTfParam param) const;
    void _setParameterNotifyingHost(eF32 value, eTfParam param) const;

    void _fillProgramCombobox();
    bool _isModulatorUsed(eU32 mod) const;
    bool _isEffectUsed(eU32 effectNum) const;
    void _createIcons();
    void _resetTimer();

    bool _configAreAnimationsOn();
    bool _configAreAnimationsFast();
    bool _configAreWaveformsMoving();
    void _configSetAnimationsOn(bool value);
    void _configSetAnimationsFast(bool value);
    void _configSetWaveformsMoving(bool value);

    bool m_wasWindowHidden;
    ApplicationProperties m_appProperties;
    TextButton m_btnAnimationsOn;
    TextButton m_btnFastAnimations;
    TextButton m_btnMovingWaveforms;
    TextButton m_btnRecord;
    TextButton m_btnFactoryWriter;
	TextButton m_btnPresetFileLoader;

    // -------------------------------------
    //  COMPONENT GROUPS
    // -------------------------------------
    eTfGroupComponent m_grpGlobal;
    eTfGroupComponent m_grpGenerator;
    eTfGroupComponent m_grpLPFilter;
    eTfGroupComponent m_grpHPFilter;
    eTfGroupComponent m_grpBPFilter;
    eTfGroupComponent m_grpNTFilter;
    eTfGroupComponent m_grpLFO1;
    eTfGroupComponent m_grpLFO2;
    eTfGroupComponent m_grpADSR1;
    eTfGroupComponent m_grpADSR2;
    eTfGroupComponent m_grpFxFlanger;
    eTfGroupComponent m_grpFxReverb;
    eTfGroupComponent m_grpFxDelay;
    eTfGroupComponent m_grpFxEQ;
    eTfGroupComponent m_grpFxChorus;
    eTfGroupComponent m_grpFxFormant;
    eTfGroupComponent m_grpFxDistortion;
    eTfGroupComponent m_grpModMatrix;
    eTfGroupComponent m_grpEffectStack;

    // -------------------------------------
    //  GLOBAL GROUP
    // -------------------------------------
    Label m_lblGlobVolume;
    Label m_lblGlobFrequency;
    Label m_lblGlobDetune;
    Label m_lblGlobSlop;
    Label m_lblGlobGlide;
    Label m_lblGlobPolyphony;
    Label m_lblGlobPitchBendUp;
    Label m_lblGlobPitchBendDown;

    LevelMeter m_meter;
    eTfSlider m_sldGlobVolume;
    eTfSlider m_sldGlobFrequency;
    eTfSlider m_sldGlobDetune;
    eTfSlider m_sldGlobSlop;
    eTfSlider m_sldGlobGlide;
    ComboBox m_cmbInstrument;
    ComboBox m_cmbPolyphony;
    ComboBox m_cmbPitchBendUp;
    ComboBox m_cmbPitchBendDown;
    TextButton m_btnRestore;
    TextButton m_btnSave;
    TextButton m_btnPrev;
    TextButton m_btnNext;
    TextButton m_btnCopy;
    TextButton m_btnPaste;

    // -------------------------------------
    //  GENERATOR GROUP
    // -------------------------------------
    eTfSlider m_sldGenVolume;
    eTfSlider m_sldGenPanning;
    eTfSlider m_sldGenSpread;
    eTfSlider m_sldGenBandwidth;
    eTfSlider m_sldGenDamp;
    eTfSlider m_sldGenHarmonics;
    eTfSlider m_sldGenDrive;
    eTfSlider m_sldGenScale;
    eTfSlider m_sldGenModulation;

    Label m_lblGenVolume;
    Label m_lblGenPanning;
    Label m_lblGenSpread;
    Label m_lblGenBandwidth;
    Label m_lblGenDamp;
    Label m_lblGenHarmonics;
    Label m_lblGenDrive;
    Label m_lblGenScale;
    Label m_lblGenModulation;

    eTfSlider m_sldNoiseAmount;
    eTfSlider m_sldNoiseFreq;
    eTfSlider m_sldNoiseBandwidth;

    Label m_lblNoiseAmount;
    Label m_lblNoiseFreq;
    Label m_lblNoiseBandwidth;

    Label m_lblUnisono;
    TextButton m_btnGenUnisono1;
    TextButton m_btnGenUnisono2;
    TextButton m_btnGenUnisono3;
    TextButton m_btnGenUnisono4;
    TextButton m_btnGenUnisono5;
    TextButton m_btnGenUnisono6;
    TextButton m_btnGenUnisono7;
    TextButton m_btnGenUnisono8;
    TextButton m_btnGenUnisono9;
    TextButton m_btnGenUnisono10;

    Label m_lblOctave;
    TextButton m_btnGenOctave1;
    TextButton m_btnGenOctave2;
    TextButton m_btnGenOctave3;
    TextButton m_btnGenOctave4;
    TextButton m_btnGenOctave5;
    TextButton m_btnGenOctave6;
    TextButton m_btnGenOctave7;
    TextButton m_btnGenOctave8;
    TextButton m_btnGenOctave9;

    eTfFreqView m_freqView;

    // -------------------------------------
    //  FILTER GROUP
    // -------------------------------------
    TextButton m_btnLPOn;
    eTfSlider m_sldLPFrequency;
    eTfSlider m_sldLPResonance;
    Label m_lblLPFrequency;
    Label m_lblLPResonance;

    TextButton m_btnHPOn;
    eTfSlider m_sldHPFrequency;
    eTfSlider m_sldHPResonance;
    Label m_lblHPFrequency;
    Label m_lblHPResonance;

    TextButton m_btnBPOn;
    eTfSlider m_sldBPFrequency;
    eTfSlider m_sldBPQ;
    Label m_lblBPFrequency;
    Label m_lblBPQ;

    TextButton m_btnNTOn;
    eTfSlider m_sldNTFrequency;
    eTfSlider m_sldNTQ;
    Label m_lblNTFrequency;
    Label m_lblNTQ;

    // -------------------------------------
    //  LGO/ADSR GROUP
    // -------------------------------------
    TextButton m_btnLFO1Sync;
    eTfSlider m_sldLFO1Rate;
    eTfSlider m_sldLFO1Depth;
    Label m_lblLFO1Rate;
    Label m_lblLFO1Depth;
    DrawableButton m_btnLFO1ShapeSine;
    DrawableButton m_btnLFO1ShapeSawUp;
    DrawableButton m_btnLFO1ShapeSawDown;
    DrawableButton m_btnLFO1ShapeSquare;
    DrawableButton m_btnLFO1ShapeNoise;

    TextButton m_btnLFO2Sync;
    eTfSlider m_sldLFO2Rate;
    eTfSlider m_sldLFO2Depth;
    Label m_lblLFO2Rate;
    Label m_lblLFO2Depth;
    DrawableButton m_btnLFO2ShapeSine;
    DrawableButton m_btnLFO2ShapeSawUp;
    DrawableButton m_btnLFO2ShapeSawDown;
    DrawableButton m_btnLFO2ShapeSquare;
    DrawableButton m_btnLFO2ShapeNoise;

    Image m_imgShapeSine;
    Image m_imgShapeSawUp;
    Image m_imgShapeSawDown;
    Image m_imgShapeSquare;
    Image m_imgShapeNoise;
    DrawableImage m_dimgShapeSine;
    DrawableImage m_dimgShapeSawUp;
    DrawableImage m_dimgShapeSawDown;
    DrawableImage m_dimgShapeSquare;
    DrawableImage m_dimgShapeNoise;

    Slider m_sldADSR1Attack;
    Slider m_sldADSR1Decay;
    Slider m_sldADSR1Sustain;
    Slider m_sldADSR1Release;
    Slider m_sldADSR1Slope;
    Label m_lblADSR1Attack;
    Label m_lblADSR1Decay;
    Label m_lblADSR1Sustain;
    Label m_lblADSR1Release;
    Label m_lblADSR1Slope;

    Slider m_sldADSR2Attack;
    Slider m_sldADSR2Decay;
    Slider m_sldADSR2Sustain;
    Slider m_sldADSR2Release;
    Slider m_sldADSR2Slope;
    Label m_lblADSR2Attack;
    Label m_lblADSR2Decay;
    Label m_lblADSR2Sustain;
    Label m_lblADSR2Release;
    Label m_lblADSR2Slope;

    // -------------------------------------
    //  FX GROUPS
    // -------------------------------------
    eTfSlider m_sldFlangerLFO;
    eTfSlider m_sldFlangerFrequency;
    eTfSlider m_sldFlangerAmplitude;
    eTfSlider m_sldFlangerWet;
    Label m_lblFlangerLFO;
    Label m_lblFlangerFrequency;
    Label m_lblFlangerAmplitude;
    Label m_lblFlangerWet;

    eTfSlider m_sldReverbRoomsize;
    eTfSlider m_sldReverbDamp;
    eTfSlider m_sldReverbWet;
    eTfSlider m_sldReverbWidth;
    Label m_lblReverbRoomsize;
    Label m_lblReverbDamp;
    Label m_lblReverbWet;
    Label m_lblReverbWidth;

    eTfSlider m_sldDelayLeft;
    eTfSlider m_sldDelayRight;
    eTfSlider m_sldDelayDecay;
    Label m_lblDelayLeft;
    Label m_lblDelayRight;
    Label m_lblDelayDecay;

    eTfSlider m_sldEqLow;
    eTfSlider m_sldEqMid;
    eTfSlider m_sldEqHigh;
    Label m_lblEqLow;
    Label m_lblEqMid;
    Label m_lblEqHigh;

    eTfSlider m_sldChorusFreq;
    eTfSlider m_sldChorusDepth;
    eTfSlider m_sldChorusGain;
    Label m_lblChorusFreq;
    Label m_lblChorusDepth;
    Label m_lblChorusGain;

    eTfSlider m_sldFormantWet;
    Label m_lblFormantWet;
    TextButton m_btnFormantA;
    TextButton m_btnFormantE;
    TextButton m_btnFormantI;
    TextButton m_btnFormantO;
    TextButton m_btnFormantU;

    eTfSlider m_sldDistortionAmount;
    Label m_lblDistortionAmount;

    // -------------------------------------
    //  MOD MATRIX GROUP
    // -------------------------------------
    ComboBox m_cmbMM1Src;
    ComboBox m_cmbMM1Dest;
    eTfSlider m_sldMM1Mod;
    ComboBox m_cmbMM2Src;
    ComboBox m_cmbMM2Dest;
    eTfSlider m_sldMM2Mod;
    ComboBox m_cmbMM3Src;
    ComboBox m_cmbMM3Dest;
    eTfSlider m_sldMM3Mod;
    ComboBox m_cmbMM4Src;
    ComboBox m_cmbMM4Dest;
    eTfSlider m_sldMM4Mod;
    ComboBox m_cmbMM5Src;
    ComboBox m_cmbMM5Dest;
    eTfSlider m_sldMM5Mod;
    ComboBox m_cmbMM6Src;
    ComboBox m_cmbMM6Dest;
    eTfSlider m_sldMM6Mod;
    ComboBox m_cmbMM7Src;
    ComboBox m_cmbMM7Dest;
    eTfSlider m_sldMM7Mod;
    ComboBox m_cmbMM8Src;
    ComboBox m_cmbMM8Dest;
    eTfSlider m_sldMM8Mod;

    // -------------------------------------
    //  EFFECT STACK GROUP
    // -------------------------------------
    ComboBox m_cmbEffect1;
    ComboBox m_cmbEffect2;
    ComboBox m_cmbEffect3;
    ComboBox m_cmbEffect4;
    ComboBox m_cmbEffect5;
    ComboBox m_cmbEffect6;
    ComboBox m_cmbEffect7;
    ComboBox m_cmbEffect8;
    ComboBox m_cmbEffect9;
    ComboBox m_cmbEffect10;

    // -------------------------------------
    // Other
    // -------------------------------------
    TextButton m_btnAbout;
    MidiKeyboardComponent m_midiKeyboard;
};

class AboutComponent : public Component
{
public:

    static void openAboutWindow(Component* parent = nullptr);

    AboutComponent();
    void paint(Graphics& g);
    void resized();

private:
    HyperlinkButton  link1, link2;
    AttributedString text1, text2, text3;
};



#endif  // PLUGINEDITOR_H_INCLUDED
