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

#ifndef TF4_HPP
#define TF4_HPP

const eU32 TF_BUFFERSIZE            = 256;
const eF32 TF_MASTER_VOLUME			= 2.0f;
const eU32 TF_FRAMESIZE             = 512;
const eU32 TF_MAXFRAMESIZE          = 4096;
const eU32 TF_IFFT_FRAMESIZE        = 512;
const eU32 TF_NOISETABLESIZE        = 65536;
const eU32 TF_NUMFREQS              = 128;
const eU32 TF_LFONOISETABLESIZE     = 256;
const eU32 TF_MODMATRIXENTRIES      = 8;
const eU32 TF_MAX_MODULATIONS       = 4;
const eF32 TF_MM_MODRANGE           = 10.0f;
const eU32 TF_MAX_HARMONICS         = 64;
const eU32 TF_MAXVOICES             = 16;
const eU32 TF_MAX_INSTR             = 32;
const eU32 TF_MAXEFFECTS            = 10;
const eU32 TF_MAXOCTAVES            = 9;
const eU32 TF_MAXUNISONO            = 10;
const eU32 TF_MAXPITCHBEND          = 24;
const eU32 TF_NUMGENPROFILES        = 4;
const eU32 TF_LFOSHAPECOUNT         = 5;
const eU32 TF_MAXMODULATIONTYPES    = 4;
const eU32 TF_FORMANTCOUNT          = 5;
const eF32 TF_EFFECT_SWITCHOFF_TIME = 2.0f;
const eF32 TF_12TH_ROOT_OF_2        = 1.059463094359f;

#include "tf4fx.hpp"

static const eF32 TF_OCTAVES[] =
{
    1.0f*16.0f,
    1.0f*8.0f,
    1.0f*4.0f,
    1.0f*2.0f,
    1.0f,
    1.0f/2.0f,
    1.0f/4.0f,
    1.0f/8.0f,
    1.0f/16.0f,
};

enum eTfFftType
{
    IFFT = 1,
    FFT = -1
};

enum eTfParam
{
    TF_GLOBAL_GAIN,

    TF_GEN_BANDWIDTH,
    TF_GEN_NUMHARMONICS,
    TF_GEN_DAMP,
    TF_GEN_MODULATION,
    TF_GEN_VOLUME,
    TF_GEN_PANNING,
    TF_GEN_SLOP,
    TF_GEN_OCTAVE,
    TF_GEN_GLIDE,
    TF_GEN_DETUNE,
    TF_GEN_FREQ,
    TF_GEN_POLYPHONY,
    TF_GEN_DRIVE,
    TF_GEN_UNISONO,
    TF_GEN_SPREAD,
    TF_GEN_SCALE,

    TF_NOISE_AMOUNT,
    TF_NOISE_FREQ,
    TF_NOISE_BW,

    TF_LP_FILTER_ON,
    TF_LP_FILTER_CUTOFF,
    TF_LP_FILTER_RESONANCE,

    TF_HP_FILTER_ON,
    TF_HP_FILTER_CUTOFF,
    TF_HP_FILTER_RESONANCE,

    TF_ADSR1_ATTACK,
    TF_ADSR1_DECAY,
    TF_ADSR1_SUSTAIN,
    TF_ADSR1_RELEASE,
    TF_ADSR1_SLOPE,

    TF_ADSR2_ATTACK,
    TF_ADSR2_DECAY,
    TF_ADSR2_SUSTAIN,
    TF_ADSR2_RELEASE,
    TF_ADSR2_SLOPE,

    TF_LFO1_RATE,
    TF_LFO1_DEPTH,
    TF_LFO1_SHAPE,
    TF_LFO1_SYNC,

    TF_LFO2_RATE,
    TF_LFO2_DEPTH,
    TF_LFO2_SHAPE,
    TF_LFO2_SYNC,

    TF_MM1_SOURCE,
    TF_MM1_MOD,
    TF_MM1_TARGET,
    TF_MM2_SOURCE,
    TF_MM2_MOD,
    TF_MM2_TARGET,
    TF_MM3_SOURCE,
    TF_MM3_MOD,
    TF_MM3_TARGET,
    TF_MM4_SOURCE,
    TF_MM4_MOD,
    TF_MM4_TARGET,
    TF_MM5_SOURCE,
    TF_MM5_MOD,
    TF_MM5_TARGET,
    TF_MM6_SOURCE,
    TF_MM6_MOD,
    TF_MM6_TARGET,
    TF_MM7_SOURCE,
    TF_MM7_MOD,
    TF_MM7_TARGET,
    TF_MM8_SOURCE,
    TF_MM8_MOD,
    TF_MM8_TARGET,
    TF_MM9_SOURCE,
    TF_MM9_MOD,
    TF_MM9_TARGET,
    TF_MM10_SOURCE,
    TF_MM10_MOD,
    TF_MM10_TARGET,

    TF_EFFECT_1,
    TF_EFFECT_2,
    TF_EFFECT_3,
    TF_EFFECT_4,
    TF_EFFECT_5,
    TF_EFFECT_6,
    TF_EFFECT_7,
    TF_EFFECT_8,
    TF_EFFECT_9,
    TF_EFFECT_10,

    TF_DISTORT_AMOUNT,

    TF_CHORUS_RATE,
    TF_CHORUS_DEPTH,

    TF_DELAY_LEFT,
    TF_DELAY_RIGHT,
    TF_DELAY_DECAY,

    TF_REVERB_ROOMSIZE,
    TF_REVERB_DAMP,
    TF_REVERB_WET,
    TF_REVERB_WIDTH,

    TF_FLANGER_LFO,
    TF_FLANGER_FREQUENCY,
    TF_FLANGER_AMPLITUDE,
    TF_FLANGER_WET,

    TF_CHORUS_GAIN,

    TF_FORMANT_MODE,
    TF_FORMANT_WET,

    TF_EQ_LOW,
    TF_EQ_MID,
    TF_EQ_HIGH,

    TF_PITCHWHEEL_UP,
    TF_PITCHWHEEL_DOWN,

    TF_BP_FILTER_ON,
    TF_BP_FILTER_CUTOFF,
    TF_BP_FILTER_Q,

    TF_NT_FILTER_ON,
    TF_NT_FILTER_CUTOFF,
    TF_NT_FILTER_Q,

    TF_PARAM_COUNT
};

#ifdef eVSTI

static const eChar * TF_NAMES[] =
{
    "Gain",

    "GenBandw",
    "GenHarmn",
    "GenDamp",
    "GenMod",
    "GenVol",
    "GenPan",
    "GenSlop",
    "GenOct",
    "GenGlid",
    "GetDetn",
    "GenFreq",
    "GenPoly",
    "GenDriv",
    "GenUnis",
    "GenSprd",
    "GenScale",

    "NoiseOn",
    "NoiseFreq",
    "NoiseBw",

    "FltLPOn",
    "FltLPCut",
    "FltLPRes",

    "FltHPOn",
    "FltHPCut",
    "FltHPRes",

    "Env1Att",
    "Env1Dec",
    "Env1Sus",
    "Env1Rel",
    "Env1Slp",

    "Env2Att",
    "Env2Dec",
    "Env2Sus",
    "Env2Rel",
    "Env2Slp",

    "Lfo1Rate",
    "Lfo1Dep",
    "Lfo1Shp",
    "Lfo1Sync",

    "Lfo2Rate",
    "Lfo2Dep",
    "Lfo2Shp",
    "Lfo2Sync",

    "Mm1Source",
    "Mm1Mod",
    "Mm1Target",
    "Mm2Source",
    "Mm2Mod",
    "Mm2Target",
    "Mm3Source",
    "Mm3Mod",
    "Mm3Target",
    "Mm4Source",
    "Mm4Mod",
    "Mm4Target",
    "Mm5Source",
    "Mm5Mod",
    "Mm5Target",
    "Mm6Source",
    "Mm6Mod",
    "Mm6Target",
    "Mm7Source",
    "Mm7Mod",
    "Mm7Target",
    "Mm8Source",
    "Mm8Mod",
    "Mm8Target",
    "Mm9Source",
    "Mm9Mod",
    "Mm9Target",
    "Mm10Sourc",
    "Mm10Mod",
    "Mm10Targe",

    "Effect1",
    "Effect2",
    "Effect3",
    "Effect4",
    "Effect5",
    "Effect6",
    "Effect7",
    "Effect8",
    "Effect9",
    "Effect10",

    "DistAmnt",

    "ChrsRate",
    "ChrsDep",

    "DelayL",
    "DelayR",
    "DelayDc",

    "RevRoom",
    "RevDamp",
    "RevWet",
    "RevWidth",

    "FlangLfo",
    "FlangFreq",
    "FlangAmp",
    "FlangWet",

    "ChrsGain",

    "FrmtType",
    "FrmtWet",

    "EqLow",
    "EqMid",
    "EqHigh",

    "OscPWup",
    "OscPWdw",

    "FltBPOn",
    "FltBPCut",
    "FltBPQ",

    "FltNTOn",
    "FltNTCut",
    "FltNTQ",
};

#endif

struct eTfLfo
{
    eF32            phase;
    eF32            result;
};

struct eTfEnvelope
{
    enum Phase
    {
        ATTACK = 0,
        DECAY,
        SUSTAIN,
        RELEASE,
        FINISHED,
    };

    eF32			volume;
    Phase           phase;
};

struct eTfGenerator
{
    enum ModulationType
    {
        MODULATION_NONE,
        MODULATION_FREQ,
        MODULATION_PHASE,
        MODULATION_RING,
        MODULATION_AMPLITUDE
    };

    eF32            modulation;
    eF32            phase[2*TF_MAXUNISONO];
    eF32            freq1;
    eF32            freq2;
    eF32            freqTable[TF_IFFT_FRAMESIZE*2];
    eF32            freqModTable[TF_IFFT_FRAMESIZE*2];
    eF32            resultTable[TF_IFFT_FRAMESIZE*2];
    eU32            writeOffset;
    eU32            minReadOffset;
    eU32            availableData;

    eU32            activeGenSize;
    eU32            activeNumHarmonics;
    eF32            activeScale;
    eF32            activeDamp;
    eF32            activeBandwidth;
};

struct eTfModMatrix
{
    enum Input
    {
        INPUT_NONE = 0,

        INPUT_LFO1,
        INPUT_LFO2,
        INPUT_ADSR1,
        INPUT_ADSR2,

        INPUT_RESERVED1,
        INPUT_RESERVED2,
        INPUT_RESERVED3,
        INPUT_RESERVED4,
        INPUT_RESERVED5,
        INPUT_RESERVED6,
        INPUT_RESERVED7,
        INPUT_RESERVED8,
        INPUT_RESERVED9,
        INPUT_RESERVED10,

        INPUT_COUNT
    };

    enum Output
    {
        OUTPUT_NONE = 0,

        OUTPUT_BANDWIDTH,
        OUTPUT_DAMP,
        OUTPUT_NUMHARMONICS,
        OUTPUT_SCALE,
        OUTPUT_VOLUME,
        OUTPUT_FREQ,
        OUTPUT_PAN,
        OUTPUT_DETUNE,
        OUTPUT_SPREAD,
        OUTPUT_DRIVE,
        OUTPUT_NOISE_AMOUNT,
        OUTPUT_LP_FILTER_CUTOFF,
        OUTPUT_LP_FILTER_RESONANCE,
        OUTPUT_HP_FILTER_CUTOFF,
        OUTPUT_HP_FILTER_RESONANCE,
        OUTPUT_BP_FILTER_CUTOFF,
        OUTPUT_BP_FILTER_Q,
        OUTPUT_NT_FILTER_CUTOFF,
        OUTPUT_NT_FILTER_Q,
        OUTPUT_ADSR1_DECAY,
        OUTPUT_ADSR2_DECAY,
        OUTPUT_MOD1,
        OUTPUT_MOD2,
        OUTPUT_MOD3,
        OUTPUT_MOD4,
        OUTPUT_MOD5,
        OUTPUT_MOD6,
        OUTPUT_MOD7,
        OUTPUT_MOD8,

        OUTPUT_RESERVED1,
        OUTPUT_RESERVED2,
        OUTPUT_RESERVED3,
        OUTPUT_RESERVED4,
        OUTPUT_RESERVED5,
        OUTPUT_RESERVED6,
        OUTPUT_RESERVED7,
        OUTPUT_RESERVED8,
        OUTPUT_RESERVED9,
        OUTPUT_RESERVED10,

        OUTPUT_COUNT
    };

    struct Entry
    {
        Input       src;
        Output      dst;
        eF32        mod;
        eF32        result;
    };

    eTfEnvelope     envState[2];
    eTfLfo          lfoState[2];
    eF32            values[INPUT_COUNT];
    Entry           entries[TF_MODMATRIXENTRIES];
    eF32            modulation[TF_MODMATRIXENTRIES];
};

struct eTfFilter
{
    enum Type
    {
        FILTER_LP,
        FILTER_HP,
        FILTER_BP,
        FILTER_NT
    };

    // lowpass memory
    eF32x2            oldx;
    eF32x2            oldy1, y1;
    eF32x2            oldy2, y2;
    eF32x2            oldy3, y3;
    eF32x2            y4;
    // highpass memory
    eF32x2            in0, in1, in2;
    eF32x2            out1, out2;
    // lowpass coefficients
    eF32            k, p, r;
    // highpass coefficients
    eF32            a0, a1, a2;
    eF32            b0, b1, b2;
};

struct eTfNoise
{
    eTfNoise()
    {
        filterLP = static_cast<eTfFilter*>(eAllocAligned(sizeof(eTfFilter), 16));
        filterHP = static_cast<eTfFilter*>(eAllocAligned(sizeof(eTfFilter), 16));
        eMemSet(filterLP, 0, sizeof(eTfFilter));
        eMemSet(filterHP, 0, sizeof(eTfFilter));

    }

    ~eTfNoise()
    {
        eFreeAligned(filterLP);
        eFreeAligned(filterHP);
    }

    eU32            offset1;
    eU32            offset2;
    eBool           filterOn;
    eTfFilter *     filterHP;
    eTfFilter *     filterLP;
    eF32            amount;
};

struct eTfVoice
{
    eTfVoice(eBool allocFilters = eTRUE)
    {
        if (allocFilters) 
        {
            filterLP = (eTfFilter*)eAllocAligned(sizeof(eTfFilter), 16);
            filterHP = (eTfFilter*)eAllocAligned(sizeof(eTfFilter), 16);
            filterBP = (eTfFilter*)eAllocAligned(sizeof(eTfFilter), 16);
            filterNT = (eTfFilter*)eAllocAligned(sizeof(eTfFilter), 16);
            eMemSet(filterLP, 0, sizeof(eTfFilter));
            eMemSet(filterHP, 0, sizeof(eTfFilter));
            eMemSet(filterBP, 0, sizeof(eTfFilter));
            eMemSet(filterNT, 0, sizeof(eTfFilter));
        } 
        else
        {
            filterLP = nullptr;
            filterHP = nullptr;
            filterBP = nullptr;
            filterNT = nullptr;
        }
    }

    ~eTfVoice()
    {
        eFreeAligned(filterLP);
        eFreeAligned(filterHP);
        eFreeAligned(filterBP);
        eFreeAligned(filterNT);
    }

    eBool           noteIsOn;
    eBool           playing;
    eU32            time;

    eF32            currentFreq;
    eS32            currentNote;
    eS32            currentVelocity;
    eF32            currentSlop;

    eF32            pitchBendSemitones;
    eF32            pitchBendCents;

	eF32			lastVolL;
	eF32			lastVolR;

    eTfModMatrix    modMatrix;
    eTfNoise        noiseGen;
    eTfFilter *     filterLP;
    eTfFilter *     filterHP;
    eTfFilter *     filterBP;
    eTfFilter *     filterNT;
    eTfGenerator    generator;
};

struct eTfInstrument
{
    eF32            params[TF_PARAM_COUNT];
    eS16            output[TF_MAXFRAMESIZE*2];
    eF32            lfo1Phase;
    eF32            lfo2Phase;
    eTfVoice        voice[TF_MAXVOICES];
    eTfVoice *      latestTriggeredVoice;
    eF32            tempBuffers[2][TF_MAXFRAMESIZE];
    eTfEffect *     effects[TF_MAXEFFECTS];
    eU32            effectIndex[TF_MAXEFFECTS];
    eF32            effectsInactiveTime;
};

struct eTfSynth
{
    eU32            sampleRate;
    eF32            randomBuffer[TF_MAXFRAMESIZE];
    eF32            sinBuffer[TF_MAXFRAMESIZE];
    eF32            expBuffer[TF_MAXFRAMESIZE];
    eF32            freqTable[TF_NUMFREQS];
    eF32            lfoNoiseTable[TF_LFONOISETABLESIZE];
    eF32            whiteNoiseTable[TF_NOISETABLESIZE];
    eTfInstrument * instr[TF_MAX_INSTR];
};

struct eTfEvent
{
	eTfEvent(eF32 time, eU8	instr, eU8 note, eU8 velocity)
	{
		this->time = time;
		this->instr = instr;
		this->note = note;
		this->velocity = velocity;
	}

	eF32			time;
	eU8				instr;
	eU8				note;
	eU8				velocity;
};

struct eTfSong
{
	eArray<eTfEvent>	events[TF_MAX_INSTR];
    eU32                instrCount;
	eU32				tempo;
};

void	eTfSignalMix16(eS16 *master, eS16 *in, eU32 length);
eBool   eTfSignalMix(eF32 **master, eF32 **in, eU32 length, eF32 volume);
void    eTfSignalToS16(eF32 **sig, eS16 *out, const eF32 gain, eU32 length);
void    eTfSignalToPeak(eF32 **sig, eF32 *peak_left, eF32 *peak_right, eU32 length);

void    eTfEnvelopeReset(eTfEnvelope &state);
eBool   eTfEnvelopeIsEnd(eTfEnvelope &state);
void    eTfEnvelopeNoteOn(eTfEnvelope &state);
void    eTfEnvelopeNoteOff(eTfEnvelope &state);
eF32    eTfEnvelopeProcess(eTfSynth &synth, eTfInstrument &instr, eTfEnvelope &envState, eF32 decayMod, eU32 paramOffset, eU32 frameSize);

void    eTfLfoReset(eTfLfo &state, eF32 phase);
eF32    eTfLfoProcess(eTfSynth &synth, eTfInstrument &instr, eTfLfo &lfoState, eU32 paramOffset, eU32 frameSize);

void    eTfModMatrixReset(eTfModMatrix &state);
void    eTfModMatrixNoteOn(eTfModMatrix &state, eF32 lfoPhase1, eF32 lfoPhase2);
void    eTfModMatrixNoteOff(eTfModMatrix &state);
void    eTfModMatrixPanic(eTfModMatrix &state);
eBool   eTfModMatrixIsActive(eTfModMatrix &state);
eBool   eTfModMatrixProcess(eTfSynth &synth, eTfInstrument &instr, eTfModMatrix &state, eU32 frameSize);
eF32    eTfModMatrixGet(eTfModMatrix &state, eTfModMatrix::Output output);

void    eTfGeneratorReset(eTfGenerator &state);
void    eTfGeneratorFft(eTfFftType type, eU32 frameSize, eF32 *buffer);
void    eTfGeneratorNormalize(eF32 *buffer, eU32 frameSize);
void    eTfGeneratorUpdate(eTfSynth &synth, eTfInstrument &instr, eTfVoice &voice, eTfGenerator &generator, eF32 frequencyRange);
eBool   eTfGeneratorModulate(eTfSynth &synth, eTfInstrument &instr, eTfGenerator &generator);
eBool   eTfGeneratorProcess(eTfSynth &synth, eTfInstrument &instr, eTfVoice &voice, eTfGenerator &generator, eF32 velocity, eF32 **signal, eU32 frameSize);

void    eTfNoiseReset(eTfNoise &state);
void    eTfNoiseUpdate(eTfSynth &synth, eTfInstrument &instr, eTfNoise &state, eTfModMatrix &modMatrix, eF32 velocity);
eBool   eTfNoiseProcess(eTfSynth &synth, eTfNoise &state, eF32 **signal, eU32 frameSize);

void    eTfFilterUpdate(eTfSynth &synth, eTfFilter &state, eF32 f, eF32 q, eTfFilter::Type type);
void    eTfFilterProcess(eTfFilter &state, eTfFilter::Type type, eF32 **signal, eU32 frameSize);

void    eTfVoiceReset(eTfVoice &state);
void    eTfVoiceNoteOn(eTfVoice &state, eS32 note, eS32 velocity, eF32 lfoPhase1, eF32 lfoPhase2);
void    eTfVoiceNoteOff(eTfVoice &state);
void    eTfVoicePitchBend(eTfVoice &state, eF32 semitones, eF32 cents);
void    eTfVoicePanic(eTfVoice &state);

void    eTfInstrumentInit(eTfInstrument &instr);
void    eTfInstrumentFree(eTfInstrument &instr);
eF32    eTfInstrumentProcess(eTfSynth &synth, eTfInstrument &instr, eF32 **outputs, eU32 sampleFrames);
void    eTfInstrumentNoteOn(eTfInstrument &instr, eS32 note, eS32 velocity);
eBool   eTfInstrumentNoteOff(eTfInstrument &instr, eS32 note);
void    eTfInstrumentAllNotesOff(eTfInstrument &instr);
void    eTfInstrumentPitchBend(eTfInstrument &instr, eF32 semitones, eF32 cents);
void    eTfInstrumentPanic(eTfInstrument &instr);
eU32    eTfInstrumentGetPolyphony(eTfInstrument &instr);
eU32    eTfInstrumentAllocateVoice(eTfInstrument &instr);

void    eTfSynthInit(eTfSynth &synth);


#endif
