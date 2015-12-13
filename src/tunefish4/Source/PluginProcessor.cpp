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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Tunefish4AudioProcessor::Tunefish4AudioProcessor() :
tf(nullptr),
synth(nullptr),
currentProgramIndex(0),
adapterWriteOffset(0),
adapterDataAvailable(0)
{
    pluginLocation = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getFullPathName();
    
    adapterBuffer[0] = new eF32[TF_BUFFERSIZE];
    adapterBuffer[1] = new eF32[TF_BUFFERSIZE];
    
    synth = new eTfSynth();
    eTfSynthInit(*synth);
    synth->sampleRate = 44100;
    
    synth->instr[0] = tf = new eTfInstrument();
    eTfInstrumentInit(*synth, *tf);
    
    for (eU32 i=0; i<TF_PLUG_NUM_PROGRAMS; i++)
    {
        programs[i].loadDefault(i);
        
        for (eU32 j=0; j<TF_PARAM_COUNT; j++)
        {
            programs[i].setParam(j, TF_DEFAULTPROG[j]);
        }
    }
    
    loadProgramAll();
    
    for (eU32 j=0; j<TF_PARAM_COUNT; j++)
    {
        tf->params[j] = programs[0].getParam(j);
    }
    
    setCurrentProgram(0);
}

Tunefish4AudioProcessor::~Tunefish4AudioProcessor()
{
    eDelete(adapterBuffer[0]);
    eDelete(adapterBuffer[1]);
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
    
    eF32 value = eTfModMatrixGet(voice->modMatrix, (eTfModMatrix::Output)index);
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
}

const String Tunefish4AudioProcessor::getParameterName (int index)
{
    eASSERT(index >= 0 && index < TF_PARAM_COUNT);
    return TF_NAMES[index];
}

const String Tunefish4AudioProcessor::getParameterText (int index)
{
    return String::empty;
}

const String Tunefish4AudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Tunefish4AudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Tunefish4AudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Tunefish4AudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
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

bool Tunefish4AudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
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
    eASSERT(index >= 0 && index < TF_PLUG_NUM_PROGRAMS);
    
    // write program from tunefish to program list before switching
    eTfSynthProgram *ap = &programs[currentProgramIndex];
    for(int i=0;i<TF_PARAM_COUNT;i++)
        ap->setParam(i, tf->params[i]);
    
    currentProgramIndex = index;
    
    // load new program to into tunefish
    ap = &programs[currentProgramIndex];
    for(int i=0;i<TF_PARAM_COUNT;i++)
        tf->params[i] = ap->getParam(i);
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

//==============================================================================
void Tunefish4AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Tunefish4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Tunefish4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    MidiBuffer::Iterator it(midiMessages);
    MidiMessage midiMessage;
    eU32 messageOffset = 0;
    
    eU32 requestedLen = buffer.getNumSamples();
    
    eU32 sampleRate = (eU32)getSampleRate();
    if (sampleRate > 0)
        synth->sampleRate = sampleRate;
    
    for (int i = 0; i < getNumOutputChannels(); ++i)
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
}

void Tunefish4AudioProcessor::processEvents(MidiBuffer &midiMessages, eU32 messageOffset, eU32 frameSize)
{
    MidiBuffer::Iterator it(midiMessages);
    MidiMessage midiMessage;
    int samplePosition;
    
    it.setNextSamplePosition(messageOffset);
    
    while (it.getNextEvent(midiMessage, samplePosition))
    {
        if (samplePosition >= messageOffset + frameSize)
            break;
        
        if (midiMessage.isNoteOn())
        {
            eU8 velocity = midiMessage.getVelocity();
            eU8 note = midiMessage.getNoteNumber();
            
            eTfInstrumentNoteOn(*tf, note, velocity);
        }
        else if (midiMessage.isNoteOff())
        {
            eU8 note = midiMessage.getNoteNumber();
            
            eTfInstrumentNoteOff(*tf, note);
        }
        else if (midiMessage.isAllNotesOff())
        {
            eTfInstrumentAllNotesOff(*tf);
        }
        else if (midiMessage.isPitchWheel())
        {
            eS32 bend_lsb = midiMessage.getRawData()[1] & 0x7f;
            eS32 bend_msb = midiMessage.getRawData()[2] & 0x7f;
            
            eTfInstrumentPitchBend(*tf, ((eF32(bend_msb) / 127.0f) - 0.5f) * 2.0f,
                                   ((eF32(bend_lsb) / 127.0f) - 0.5f) * 2.0f);
        }
    }
}

void Tunefish4AudioProcessor::writeProgramToPresets()
{
    eTfSynthProgram *ap = &programs[currentProgramIndex];
    for(int i=0;i<TF_PARAM_COUNT;i++)
        ap->setParam(i, tf->params[i]);
}

void Tunefish4AudioProcessor::loadProgramFromPresets()
{
    eTfSynthProgram *ap = &programs[currentProgramIndex];
    for(int i=0;i<TF_PARAM_COUNT;i++)
        tf->params[i] = ap->getParam(i);
}

bool Tunefish4AudioProcessor::loadProgram()
{
    return loadProgram(currentProgramIndex);
}

bool Tunefish4AudioProcessor::loadProgram(eU32 index)
{
    String path = pluginLocation +
    File::separatorString + String("tf4programs") + File::separatorString +
    String("program") + String(index) + String(".txt");
    
    File file(path);
    FileInputStream *stream = file.createInputStream();
    if (!stream)
        return false;
    
    String name = stream->readNextLine();
    programs[index].setName(name.toRawUTF8());
    
    while(true)
    {
        String line = stream->readNextLine();
        
        if (line.length() == 0)
        {
            eDelete(stream);
            return true;
        }
        
        StringArray parts;
        parts.addTokens(line, ";", String::empty);
        
        if (parts.size() == 2)
        {
            String key = parts[0];
            eF32 value = parts[1].getFloatValue();
            
            for(eU32 i=0;i<TF_PARAM_COUNT;i++)
            {
                if (key == TF_NAMES[i])
                {
                    programs[index].setParam(i, value);
                    break;
                }
            }
        }
    }
    
    return true;
}

bool Tunefish4AudioProcessor::loadProgramAll()
{
    for(int i=0;i<TF_PLUG_NUM_PROGRAMS;i++)
    {
        if (!loadProgram(i))
            return false;
    }
    
    return true;
}

bool Tunefish4AudioProcessor::saveProgram()
{
    return saveProgram(currentProgramIndex);
}

bool Tunefish4AudioProcessor::saveProgram(eU32 index)
{
    String path = pluginLocation +
    File::separatorString + String("tf4programs") + File::separatorString +
    String("program") + String(index) + String(".txt");
    
    File file(path);
    file.deleteFile();
    FileOutputStream *stream = file.createOutputStream();
    if (!stream)
        return false;
    
    stream->writeText(programs[index].getName(), false, false);
    stream->writeText("\r\n", false, false);
    
    for(eU32 i=0;i<TF_PARAM_COUNT;i++)
    {
        stream->writeText(TF_NAMES[i], false, false);
        stream->writeText(";", false, false);
        stream->writeText(String(programs[index].getParam(i)), false, false);
        stream->writeText("\r\n", false, false);
    }
    
    eDelete(stream);
    return true;
}

bool Tunefish4AudioProcessor::saveProgramAll()
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
    eTfSynthProgram *ap = &copiedProgram;
    for(int i=0;i<TF_PARAM_COUNT;i++)
        ap->setParam(i, tf->params[i]);
    
    eStrCopy(ap->getName(), programs[currentProgramIndex].getName());
    
    return true;
}

bool Tunefish4AudioProcessor::pasteProgram()
{
    eTfSynthProgram *ap = &copiedProgram;
    for(int i=0;i<TF_PARAM_COUNT;i++)
        tf->params[i] = ap->getParam(i);
    
    eStrCopy(programs[currentProgramIndex].getName(), ap->getName());
    
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
                tf->params[i] = (float) xmlState->getDoubleAttribute (TF_NAMES[i], tf->params[i]);
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

CriticalSection & Tunefish4AudioProcessor::getSynthCriticalSection()
{
    return csSynth;
}
