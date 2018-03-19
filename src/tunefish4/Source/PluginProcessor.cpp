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

static File presetsDirectory()
{
    String folder(JucePlugin_Manufacturer);
    folder << File::getSeparatorString() << JucePlugin_Name;

#if JUCE_MAC
    // /home/<Username>/Library/Audio/Presets/<JucePlugin_Manufacturer>/<JucePlugin_Name>
    return File::getSpecialLocation(File::userHomeDirectory).getChildFile(String("Library/Audio/Presets/") + folder);
#elif JUCE_WINDOWS
    // C:\Users\<Username>\AppData\Roaming\<JucePlugin_Manufacturer>\<JucePlugin_Name>
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(folder);
#elif JUCE_LINUX
    // /home/<Username>/.config/<JucePlugin_Manufacturer>/<JucePlugin_Name>
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(folder);
#else
#error You must add support for your OS here!
#endif
}

//==============================================================================
Tunefish4AudioProcessor::Tunefish4AudioProcessor() :
    tf(nullptr),
    synth(nullptr),
    paramDirtyAny(false),
    currentProgramIndex(0),
    adapterWriteOffset(0),
    adapterDataAvailable(0)
{
    meterLevels[0] = 0;
    meterLevels[1] = 0;
    metering.set(0);
    
    pluginLocation = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getFullPathName();

    adapterBuffer[0] = new eF32[TF_BUFFERSIZE];
    adapterBuffer[1] = new eF32[TF_BUFFERSIZE];

    synth = new eTfSynth();
    eTfSynthInit(*synth);
    synth->sampleRate = 44100;

    synth->instr[0] = tf = new eTfInstrument();
    eTfInstrumentInit(*tf);

    for (eU32 i=0; i<TF_PLUG_NUM_PROGRAMS; i++)
        programs[i].loadFactory(i);

    loadProgramAll();
    programs[0].applyToSynth(tf);
    resetParamDirty(eTRUE);

    recorderIndex = eTfRecorder::getInstance().addSynth(this);
}

Tunefish4AudioProcessor::~Tunefish4AudioProcessor()
{
    eTfRecorder::getInstance().removeSynth(this);

    eDelete(adapterBuffer[0]);
    eDelete(adapterBuffer[1]);
    eTfInstrumentFree(*tf);
    eDelete(tf);
    eDelete(synth);
}

//==============================================================================
const String Tunefish4AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Tunefish4AudioProcessor::getNumParameters()
{
    return TF_PARAM_COUNT;
}

float Tunefish4AudioProcessor::getParameterMod(int index)
{
    eTfVoice *voice = tf->latestTriggeredVoice;
    if (!voice)
        return 0.0f;

    if (!voice->playing)
        return 0.0f;

    eF32 value = eTfModMatrixGet(voice->modMatrix, static_cast<eTfModMatrix::Output>(index));
    if (value == 1.0f)
        return 0.0f;

    return value;
}

float Tunefish4AudioProcessor::getParameter (int index)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    return tf->params[index];
}

void Tunefish4AudioProcessor::setParameter (int index, float newValue)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    tf->params[index] = newValue;
    paramDirty[index] = eTRUE;
    paramDirtyAny = eTRUE;
}

const String Tunefish4AudioProcessor::getParameterName (int index)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    return TF_NAMES[index];
}

const String Tunefish4AudioProcessor::getParameterText (int )
{
    return String::empty;
}

bool Tunefish4AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Tunefish4AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double Tunefish4AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Tunefish4AudioProcessor::getNumPrograms()
{
    return TF_PLUG_NUM_PROGRAMS;
}

int Tunefish4AudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void Tunefish4AudioProcessor::setCurrentProgram (int index)
{
    if (static_cast<eInt>(currentProgramIndex) == index)
        return;

    eASSERT(index >= 0 && index < TF_PLUG_NUM_PROGRAMS);

    // write program from tunefish to program list before switching
    programs[currentProgramIndex].loadFromSynth(tf);

    currentProgramIndex = index;
    resetParamDirty(eTRUE);

    // load new program to into tunefish
    programs[currentProgramIndex].applyToSynth(tf);

    updateHostDisplay();
}

const String Tunefish4AudioProcessor::getProgramName (int index)
{
    eASSERT(index >= 0 && index < TF_PLUG_NUM_PROGRAMS);
    return programs[index].getName();
}

void Tunefish4AudioProcessor::changeProgramName (int index, const String& newName)
{
    eASSERT(index >= 0 && index < TF_PLUG_NUM_PROGRAMS);
    programs[index].setName(newName.toRawUTF8());
}

const String Tunefish4AudioProcessor::getCurrentProgramName() const
{
	return programs[currentProgramIndex].getName();
}

eTfSynth* Tunefish4AudioProcessor::getSynth() const
{
    return synth;
}

CriticalSection & Tunefish4AudioProcessor::getSynthCriticalSection()
{
    return csSynth;
}

void Tunefish4AudioProcessor::setMetering (bool on)
{
    metering.set(on);
}

float Tunefish4AudioProcessor::getMeterLevel (int channel, int meter)
{
	switch (meter)
	{
	case 0: // main meters
		return meterLevels[channel].get();

	default:
		return 0.0f;
	}
}

//==============================================================================
void Tunefish4AudioProcessor::prepareToPlay (double sampleRate, int )
{
    if (sampleRate > 0)
        synth->sampleRate = static_cast<eU32>(sampleRate);
}

void Tunefish4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Tunefish4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    MidiBuffer::Iterator it(midiMessages);
    MidiMessage midiMessage;
    eU32 messageOffset = 0;

    eU32 requestedLen = buffer.getNumSamples();

    eU32 sampleRate = static_cast<eU32>(getSampleRate());
    if (sampleRate > 0)
        synth->sampleRate = sampleRate;

    for (int i = 0; i < getTotalNumOutputChannels(); ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    if (buffer.getNumChannels() == 2)
    {
        eU32 len = requestedLen;
        eF32 **signal = buffer.getArrayOfWritePointers();
        eF32 *destL = signal[0];
        eF32 *destR = signal[1];

        while(len)
        {
            if (!adapterDataAvailable)
            {
                csSynth.enter();
                eMemSet(adapterBuffer[0], 0, TF_BUFFERSIZE * sizeof(eF32));
                eMemSet(adapterBuffer[1], 0, TF_BUFFERSIZE * sizeof(eF32));
                processEvents(midiMessages, messageOffset, TF_BUFFERSIZE);
                eTfInstrumentProcess(*synth, *tf, adapterBuffer, TF_BUFFERSIZE);
                messageOffset += TF_BUFFERSIZE;
                adapterDataAvailable = TF_BUFFERSIZE;
                csSynth.exit();
            }

            eF32 *srcL = &adapterBuffer[0][TF_BUFFERSIZE - adapterDataAvailable];
            eF32 *srcR = &adapterBuffer[1][TF_BUFFERSIZE - adapterDataAvailable];

            while (len && adapterDataAvailable)
            {
                *destL++ += *srcL++;
                *destR++ += *srcR++;

                len--;
                adapterDataAvailable--;
            }
        }
    }

    processEvents(midiMessages, messageOffset, requestedLen);
    midiMessages.clear();
    
    // Master Volume & Pan, Metering
    if (buffer.getNumChannels() == 2)
    {
        if (metering.get())
        {
            meterLevels[0].set (buffer.getMagnitude (0, 0, buffer.getNumSamples()));
            meterLevels[1].set (buffer.getMagnitude (1, 0, buffer.getNumSamples()));
        }
    }
}

void Tunefish4AudioProcessor::processEvents(MidiBuffer &midiMessages, eU32 messageOffset, eU32 frameSize)
{
    MidiBuffer::Iterator it(midiMessages);
    MidiMessage midiMessage;
    int samplePosition;

    // get the samplerate
    eF32 sampleRate = static_cast<eF32>(getSampleRate());

    // get the current tempo
    juce::AudioPlayHead::CurrentPositionInfo cpi;
    getPlayHead()->getCurrentPosition(cpi);
    eU32 tempo = static_cast<eU32>(cpi.bpm);
    eTfRecorder::getInstance().setTempo(static_cast<eU16>(tempo));

    it.setNextSamplePosition(messageOffset);

    while (it.getNextEvent(midiMessage, samplePosition))
    {
        if (samplePosition >= static_cast<int>(messageOffset + frameSize))
            break;

        eF32 time = static_cast<eF32>(cpi.timeInSeconds) + (static_cast<eF32>(samplePosition) / sampleRate);
        
        if (midiMessage.isNoteOn())
        {
            eU8 velocity = static_cast<eU8>(midiMessage.getVelocity());
            eU8 note = static_cast<eU8>(midiMessage.getNoteNumber());

            eTfInstrumentNoteOn(*tf, note, velocity);

            eTfRecorder::getInstance().recordEvent(eTfEvent(time, static_cast<eU8>(recorderIndex), note, velocity));
        }
        else if (midiMessage.isNoteOff())
        {
            eU8 note = static_cast<eU8>(midiMessage.getNoteNumber());

            if (eTfInstrumentNoteOff(*tf, note))
            {
                eTfRecorder::getInstance().recordEvent(eTfEvent(time, static_cast<eU8>(recorderIndex), note, 0));                
            }
        }
        else if (midiMessage.isAllNotesOff())
        {
            eTfInstrumentAllNotesOff(*tf);
        }
        else if (midiMessage.isPitchWheel())
        {
            eS32 bend_lsb = midiMessage.getRawData()[1] & 0x7f;
            eS32 bend_msb = midiMessage.getRawData()[2] & 0x7f;

            auto semitones = ((eF32(bend_msb) / 127.0f) - 0.5f) * 2.0f;
            auto cents = ((eF32(bend_lsb) / 127.0f) - 0.5f) * 2.0f;

            eTfInstrumentPitchBend(*tf, semitones, cents);
        }
    }
}

void Tunefish4AudioProcessor::writeProgramToPresets()
{
    programs[currentProgramIndex].loadFromSynth(tf);
}

void Tunefish4AudioProcessor::loadProgramFromPresets() const
{
    programs[currentProgramIndex].applyToSynth(tf);
}

bool Tunefish4AudioProcessor::loadProgram()
{
    return loadProgram(currentProgramIndex);
}

bool Tunefish4AudioProcessor::loadProgram(eU32 index)
{
    File file = presetsDirectory().getChildFile(String("program") + String(index) + String(".txt"));

    if (!file.existsAsFile())
    {
        programs[index].loadFactory(index);
        return true;
    }
    
	return loadPresetFile(file, false, index);
}

bool Tunefish4AudioProcessor::loadProgramAll()
{
    for(auto i=0;i<TF_PLUG_NUM_PROGRAMS;i++)
    {
        loadProgram(i);
    }

    return true;
}

bool Tunefish4AudioProcessor::saveProgram() const
{
    return saveProgram(currentProgramIndex);
}

bool Tunefish4AudioProcessor::saveProgram(eU32 index) const
{
    File file = presetsDirectory().getChildFile(String("program") + String(index) + String(".txt"));

    file.getParentDirectory().createDirectory();
    file.deleteFile();

    ScopedPointer<FileOutputStream> stream = file.createOutputStream();
    if (stream == nullptr)
    {
        NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::WarningIcon,
            "Error",
            "Failed writing " + file.getFullPathName());
        return false;
    }

    stream->writeText(programs[index].getName(), false, false);
    stream->writeText("\r\n", false, false);

    for(eU32 i=0;i<TF_PARAM_COUNT;i++)
    {
        stream->writeText(TF_NAMES[i], false, false);
        stream->writeText(";", false, false);
        stream->writeText(String(programs[index].getParam(i)), false, false);
        stream->writeText("\r\n", false, false);
    }

    return true;
}

bool Tunefish4AudioProcessor::saveProgramAll() const
{
    for(int i=0;i<TF_PLUG_NUM_PROGRAMS;i++)
    {
        if (!saveProgram(i))
            return false;
    }

    return true;
}

bool Tunefish4AudioProcessor::copyProgram()
{
    copiedProgram.loadFromSynth(tf);
    copiedProgram.setName(programs[currentProgramIndex].getName());
    return true;
}

bool Tunefish4AudioProcessor::pasteProgram()
{
    programs[currentProgramIndex] = copiedProgram;
    programs[currentProgramIndex].applyToSynth(tf);
    saveProgram();
    return true;
}

bool Tunefish4AudioProcessor::isParamDirty(eU32 index)
{
    return paramDirty[index];
}

bool Tunefish4AudioProcessor::isParamDirtyAny() const
{
    return paramDirtyAny;
}

bool Tunefish4AudioProcessor::wasProgramSwitched() const
{
    return programSwitched;
}

void Tunefish4AudioProcessor::resetParamDirty(eBool dirty)
{
    for (eU32 j = 0; j<TF_PARAM_COUNT; j++)
    {
        paramDirty[j] = dirty;
    }

    programSwitched = dirty;
    paramDirtyAny = dirty;
}

bool Tunefish4AudioProcessor::writeFactoryPatchHeader(File headerFile) const
{
    if (!headerFile.deleteFile())
        return false;

    FileOutputStream *out = headerFile.createOutputStream();
    if (!out)
        return false;

    out->writeText("const int TF_FACTORY_PATCH_COUNT = " + String(TF_PLUG_NUM_PROGRAMS) + ";\r\n", false, false);
    out->writeText("const int TF_FACTORY_PATCH_PARAMCOUNT = " + String(TF_PARAM_COUNT) + ";\r\n", false, false);
    out->writeText("const double TF_FACTORY_PATCHES[TF_FACTORY_PATCH_COUNT][TF_FACTORY_PATCH_PARAMCOUNT] = {\r\n", false, false);
    
    for (auto i=0; i<TF_PLUG_NUM_PROGRAMS; i++)
    {
        auto &program = programs[i];

        out->writeText("\t{\r\n\t\t", false, false);

        for (auto j=0; j<TF_PARAM_COUNT; j++)
        {
            auto value = program.getParam(j);            
            out->writeText(String(value) + ", ", false, false);            
        }

        out->writeText("\r\n\t},\r\n", false, false);
    }

    out->writeText("};\r\n\r\n", false, false);

    out->writeText("const char * TF_FACTORY_PATCH_NAMES[TF_FACTORY_PATCH_COUNT] = {\r\n", false, false);

    for (auto i = 0; i<TF_PLUG_NUM_PROGRAMS; i++)
    {
        auto &program = programs[i];
        out->writeText("\t\"" + program.getName() + "\",\r\n", false, false);
    }

    out->writeText("};\r\n\r\n", false, false);
           
    delete out;
    return true;
}

bool Tunefish4AudioProcessor::loadPresetFile(File file, bool applyToSynth, int index)
{
	if (index == -1)
		index = currentProgramIndex;
	
	ScopedPointer<FileInputStream> stream = file.createInputStream();
	if (stream == nullptr)
	{
		NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::WarningIcon,
			"Error",
			"Failed opening " + file.getFullPathName());
		return false;
	}

	programs[index].setName(stream->readNextLine());

	while (true)
	{
		String line = stream->readNextLine();

		if (line.length() == 0)
			break;

		StringArray parts;
		parts.addTokens(line, ";", String::empty);

		if (parts.size() == 2)
		{
			String key = parts[0];
			eF32 value = parts[1].getFloatValue();

			for (eU32 i = 0; i<TF_PARAM_COUNT; i++)
			{
				if (key == TF_NAMES[i])
				{
					programs[index].setParam(i, value);
					break;
				}
			}
		}
	}

	if (applyToSynth)
	{
		programs[index].applyToSynth(tf);
		saveProgram();
		resetParamDirty(true);
	}

	return true;
}

//==============================================================================
bool Tunefish4AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Tunefish4AudioProcessor::createEditor()
{
    return new Tunefish4AudioProcessorEditor (this, synth);
}

//==============================================================================
void Tunefish4AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml ("TF4SETTINGS");

    for (eU32 i=0; i<TF_PARAM_COUNT; i++)
    {
        xml.setAttribute (TF_NAMES[i], tf->params[i]);
    }

    copyXmlToBinary (xml, destData);
}

void Tunefish4AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("TF4SETTINGS"))
        {
            for (eU32 i=0; i<TF_PARAM_COUNT; i++)
            {
                tf->params[i] = static_cast<float>(xmlState->getDoubleAttribute (TF_NAMES[i], tf->params[i]));
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Tunefish4AudioProcessor();
}
