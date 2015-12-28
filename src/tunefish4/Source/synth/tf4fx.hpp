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

#ifndef TF4FX_HPP
#define TF4FX_HPP

struct eTfLfo;
struct eTfSynth;
struct eTfInstrument;

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT COMPONENTS
// ---------------------------------------------------------------------------------------------------------------------------

const eU32 TF_DELAY_MAXLEN   = 192000;
const eU32 TF_COMB_MAXLEN    = 4096;
const eU32 TF_ALLPASS_MAXLEN = 4096;

struct eTfDelay
{
    eBool    singleDelay;
    eF32     delayBuffer[TF_DELAY_MAXLEN];
    eU32     delayLen;
    eU32     readOffset;
    eU32     writeOffset;
};

struct eTfComb
{
    eF32    buffer[TF_COMB_MAXLEN];
    eInt    bufsize;
    eInt    bufidx;
	eF32    filterstore;
};

struct eTfAllpass
{
    eF32    buffer[TF_ALLPASS_MAXLEN];
    eInt    bufsize;
    eInt    bufidx;
};

void eTfDelayInit(eTfDelay &delay, eBool singleDelay);
void eTfDelayUpdate(eTfDelay &delay, eU32 sampleRate, eF32 ms);
void eTfDelayProcess(eTfDelay &delay, eF32 *signal, eU32 len, eF32 decay);

void eTfCombInit(eTfComb &comb, eU32 size);
void eTfCombProcess(eTfComb &comb1, eTfComb &comb2, eF32 damp1, eF32 damp2, eF32 feedback, eF32 gain, eF32 **signals_in, eF32 **signals_out, eU32 len);

void eTfAllpassInit(eTfAllpass &allpass, eU32 size);
void eTfAllpassProcess(eTfAllpass &allpass1, eTfAllpass &allpass2, eF32 feedback, eF32 **signals_in, eF32 **signals_out, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT INTERFACE
// ---------------------------------------------------------------------------------------------------------------------------

enum eTfFxMode
{
    FX_NONE = 0,
    FX_DISTORTION,
    FX_DELAY,
    FX_CHORUS,
    FX_FLANGER,
    FX_REVERB,
    FX_FORMANT,
    FX_EQ,
    FX_RESERVED6,
    FX_RESERVED7,
    FX_RESERVED8,

    FX_COUNT
};

typedef void        eTfEffect;
typedef eTfEffect * (*eTfEffectCreateProc)();
typedef void        (*eTfEffectDeleteProc)(eTfEffect *fx);
typedef void        (*eTfEffectProcessProc)(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT DELAY
// ---------------------------------------------------------------------------------------------------------------------------

const eU32      TF_FX_DELAY_MAX_MILLISECONDS = 1000;

struct eTfEffectDelay
{
    eTfDelay    delay[2];
};

eTfEffect *     eTfEffectDelayCreate();
void            eTfEffectDelayDelete(eTfEffect *fx);
void            eTfEffectDelayProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT REVERB
// ---------------------------------------------------------------------------------------------------------------------------

const eU32      TF_FX_REVERB_NUMCOMBS     = 8;
const eU32      TF_FX_REVERB_NUMALLPASSES = 4;

struct eTfEffectReverb
{
    eTfComb     comb[2][TF_FX_REVERB_NUMCOMBS];
    eTfAllpass  allpass[2][TF_FX_REVERB_NUMALLPASSES];
    eF32        combBuffers[TF_FX_REVERB_NUMCOMBS*2*TF_MAXFRAMESIZE];
    eF32        mixBuffers[TF_MAXFRAMESIZE*2];
};

eTfEffect *     eTfEffectReverbCreate();
void            eTfEffectReverbDelete(eTfEffect *fx);
void            eTfEffectReverbProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT DISTORTION
// ---------------------------------------------------------------------------------------------------------------------------

const eU32        TF_FX_DISTORTION_TABLESIZE = 32768;

struct eTfEffectDistortion
{
    eF32        generatedAmount;
    eF32        powTable[TF_FX_DISTORTION_TABLESIZE];
};

eTfEffect *     eTfEffectDistortionCreate();
void            eTfEffectDistortionDelete(eTfEffect *fx);
void            eTfEffectDistortionProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT FORMANT
// ---------------------------------------------------------------------------------------------------------------------------

const eU32      TF_FX_FORMANT_MEMSIZE = 10;

struct eTfEffectFormant
{
    eF64        memoryL[TF_FX_FORMANT_MEMSIZE];
    eF64        memoryR[TF_FX_FORMANT_MEMSIZE];
};

eTfEffect *     eTfEffectFormantCreate();
void            eTfEffectFormantDelete(eTfEffect *fx);
void            eTfEffectFormantProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT EQ
// ---------------------------------------------------------------------------------------------------------------------------

struct eTfEffectEq
{
    // Filter #1 (Low band)
    eF32x2      m_f1p0;     // Poles ...
    eF32x2      m_f1p1;
    eF32x2      m_f1p2;
    eF32x2      m_f1p3;

    // Filter #2 (High band)
    eF32x2      m_f2p0;     // Poles ...
    eF32x2      m_f2p1;
    eF32x2      m_f2p2;
    eF32x2      m_f2p3;

    // Sample history buffer
    eF32x2      m_sdm1;     // Sample data minus 1
    eF32x2      m_sdm2;     //                   2
    eF32x2      m_sdm3;     //                   3
};

eTfEffect *     eTfEffectEqCreate();
void            eTfEffectEqDelete(eTfEffect *fx);
void            eTfEffectEqProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT CHORUS
// ---------------------------------------------------------------------------------------------------------------------------

const eU32      TF_FX_CHORUS_DELAYCOUNT = 3;
const eF32      TF_FX_CHORUS_DELAY_MAX = 10.0f;
const eF32      TF_FX_CHORUS_DELAY_MIN = 1.0f;

struct eTfEffectChorus
{
    eTfDelay    delay[2*TF_FX_CHORUS_DELAYCOUNT];
    eF32        lfoPhase[2*TF_FX_CHORUS_DELAYCOUNT];
};

eTfEffect *     eTfEffectChorusCreate();
void            eTfEffectChorusDelete(eTfEffect *fx);
void            eTfEffectChorusProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT FLANGER
// ---------------------------------------------------------------------------------------------------------------------------

const eU32  TF_FX_FLANGERBUFFSIZE      = 4096;

struct eTfEffectFlanger
{
    eInt        buffpos;
    eInt        bidi;
    eF32        buffleft[TF_FX_FLANGERBUFFSIZE*2];
    eF32        buffright[TF_FX_FLANGERBUFFSIZE*2];
    eInt        depth;
    eInt        targetDepth;
    eInt        volume;
    eInt        targetVolume;
    eInt        angle;
    eF32        angle0;
    eF32        angle1;
    eF32        lfocount;
    eF32        lastBpm;
};

eTfEffect *     eTfEffectFlangerCreate();
void            eTfEffectFlangerDelete(eTfEffect *fx);
void            eTfEffectFlangerProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len);

// ---------------------------------------------------------------------------------------------------------------------------
//  FUNCTION POINTERS
// ---------------------------------------------------------------------------------------------------------------------------

static eTfEffectCreateProc s_effectCreate[] =
{
    nullptr,
#ifndef eCFG_NO_TF_FX_DISTORTION
    eTfEffectDistortionCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_DELAY
    eTfEffectDelayCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_CHORUS
    eTfEffectChorusCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FLANGER
    eTfEffectFlangerCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_REVERB
    eTfEffectReverbCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FORMANT
    eTfEffectFormantCreate,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_EQ
    eTfEffectEqCreate,
#else
    nullptr,
#endif
    nullptr,   // FX_RESERVED6
    nullptr,   // FX_RESERVED7
    nullptr,   // FX_RESERVED8
};

static eTfEffectDeleteProc s_effectDelete[] =
{
    nullptr,
#ifndef eCFG_NO_TF_FX_DISTORTION
    eTfEffectDistortionDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_DELAY
    eTfEffectDelayDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_CHORUS
    eTfEffectChorusDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FLANGER
    eTfEffectFlangerDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_REVERB
    eTfEffectReverbDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FORMANT
    eTfEffectFormantDelete,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_EQ
    eTfEffectEqDelete,
#else
    nullptr,
#endif
    nullptr,   // FX_RESERVED6
    nullptr,   // FX_RESERVED7
    nullptr,   // FX_RESERVED8
};

static eTfEffectProcessProc s_effectProcess[] =
{
    nullptr,
#ifndef eCFG_NO_TF_FX_DISTORTION
    eTfEffectDistortionProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_DELAY
    eTfEffectDelayProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_CHORUS
    eTfEffectChorusProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FLANGER
    eTfEffectFlangerProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_REVERB
    eTfEffectReverbProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_FORMANT
    eTfEffectFormantProcess,
#else
    nullptr,
#endif
#ifndef eCFG_NO_TF_FX_EQ
    eTfEffectEqProcess,
#else
    nullptr,
#endif
    nullptr,   // FX_RESERVED6
    nullptr,   // FX_RESERVED7
    nullptr,   // FX_RESERVED8
};

#endif
