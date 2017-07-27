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

#ifdef eENIGMA
#include "synth.hpp"
#else
#include "../runtime/system.hpp"
#include "tf4.hpp"
#endif

#define LEFT 0
#define RIGHT 1

// ---------------------------------------------------------------------------------------------------------------------------
//  DELAY
// ---------------------------------------------------------------------------------------------------------------------------

void eTfDelayInit(eTfDelay &delay, eBool singleDelay)
{
    eMemZero(delay);
    delay.singleDelay = singleDelay;
}

void eTfDelayUpdate(eTfDelay &delay, eU32 sampleRate, eF32 ms)
{
    delay.delayLen = eFtoL((eF32)sampleRate * ms / 1000.0f);
    delay.delayLen = eClamp<eU32>(1, delay.delayLen, TF_DELAY_MAXLEN);
    if (delay.singleDelay) {
        eS32 readOffset = delay.writeOffset - delay.delayLen;
        if (readOffset < 0) readOffset += TF_DELAY_MAXLEN;
        delay.readOffset = (eU32)readOffset;
    } else {
        delay.writeOffset = eClamp<eU32>(0, delay.writeOffset, delay.delayLen-1);
        delay.readOffset = eClamp<eU32>(0, delay.readOffset, delay.delayLen-1);
    }
}

void eTfDelayProcess(eTfDelay &delay, eF32 *signal, eU32 len, eF32 decay)
{
    eU32 *read_pos      = &delay.readOffset;
    eU32 *write_pos     = &delay.writeOffset;
    eF32 *buffer_src    = &delay.delayBuffer[*read_pos];
    eF32 *buffer_dest   = &delay.delayBuffer[*write_pos];
    eU32 wrapPosition   = delay.singleDelay ? TF_DELAY_MAXLEN : delay.delayLen;
    eF32 mulDest        = delay.singleDelay ? 0.0f : 1.0f;

    while (len--) {
        // fetch one sample
        eF32 sample = *signal;

        // write it to buffer
        *buffer_dest = (sample + (*buffer_dest * mulDest)) * decay;
        eUndenormalise((*buffer_dest));
        buffer_dest++;

        // read from buffer and add to sample
        sample += *buffer_src++;

        // increase write position and verify
        (*write_pos)++;
        if ((*write_pos) >= wrapPosition) {
            (*write_pos) = 0;
            buffer_dest = delay.delayBuffer;
        }

        // increase read position and verify
        (*read_pos)++;
        if ((*read_pos) >= wrapPosition) {
            (*read_pos) = 0;
            buffer_src = delay.delayBuffer;
        }

        // write to output
        *signal++ = sample;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  COMB
// ---------------------------------------------------------------------------------------------------------------------------

void eTfCombInit(eTfComb &comb, eU32 size)
{
    eASSERT(size+3 < TF_COMB_MAXLEN);
    eMemZero(comb);
    comb.bufsize = size;
}

void eTfCombProcess(eTfComb &comb1, eTfComb &comb2, eF32 damp1, eF32 damp2, eF32 feedback, eF32 gain, eF32 **signals_in, eF32 **signals_out, eU32 len)
{
    eF32 *inputL = signals_in[LEFT];
    eF32 *inputR = signals_in[RIGHT];
    eF32 *outputL = signals_out[LEFT];
    eF32 *outputR = signals_out[RIGHT];

    eF32x2 damp2x2 = eSimdSetAll(damp2);
    eF32x2 damp1x2 = eSimdSetAll(damp1);
    eF32x2 feedbackx2 = eSimdSetAll(feedback);

    while(len--)
    {
        eF32 input = (*inputL++ + *inputR++) * gain;
        eF32x2 minput = eSimdSetAll(input);

        eF32x2 output = eSimdSet2(comb1.buffer[comb1.bufidx], comb2.buffer[comb2.bufidx]);
        eF32x2 filterstore = eSimdSet2(comb1.filterstore, comb2.filterstore);
        filterstore = eSimdAdd(eSimdMul(output, damp2x2), eSimdMul(filterstore, damp1x2));

        eF32x2 buffer = eSimdAdd(minput, eSimdMul(filterstore, feedbackx2));

        eSimdStore2(buffer, comb1.buffer[comb1.bufidx], comb2.buffer[comb2.bufidx]);
        eSimdStore2(filterstore, comb1.filterstore, comb2.filterstore);
        eSimdStore2(output, *outputL++, *outputR++);

        if (++comb1.bufidx >= comb1.bufsize) comb1.bufidx = 0;
        if (++comb2.bufidx >= comb2.bufsize) comb2.bufidx = 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  ALLPASS
// ---------------------------------------------------------------------------------------------------------------------------

void eTfAllpassInit(eTfAllpass &allpass, eU32 size)
{
    eASSERT(size+3 < TF_ALLPASS_MAXLEN);
    eMemZero(allpass);
    allpass.bufsize = size;
}

void eTfAllpassProcess(eTfAllpass &allpass1, eTfAllpass &allpass2, eF32 feedback, eF32 **signals_in, eF32 **signals_out, eU32 len)
{
    eF32 *inputL = signals_in[0];
    eF32 *inputR = signals_in[1];
    eF32 *outputL = signals_out[0];
    eF32 *outputR = signals_out[1];

    eF32x2 feedbackx2 = eSimdSetAll(feedback);

    while(len--)
    {
        eF32x2 output = eSimdSet2(*inputL++, *inputR++);
        eF32x2 bufout = eSimdSet2(allpass1.buffer[allpass1.bufidx], allpass2.buffer[allpass2.bufidx]);
        eF32x2 buffer = eSimdAdd(output, eSimdMul(bufout, feedbackx2));
        output = eSimdSub(bufout, output);
        eSimdStore2(buffer, allpass1.buffer[allpass1.bufidx], allpass2.buffer[allpass2.bufidx]);
        eSimdStore2(output, *outputL++, *outputR++);

        if (++allpass1.bufidx >= allpass1.bufsize) allpass1.bufidx = 0;
        if (++allpass2.bufidx >= allpass2.bufsize) allpass2.bufidx = 0;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT DELAY
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectDelayCreate()
{
    eTfEffectDelay *delay = static_cast<eTfEffectDelay *>(eAllocAligned(sizeof(eTfEffectDelay), 16));
    eMemSet(delay, 0, sizeof(eTfEffectDelay));

    eTfDelayInit(delay->delay[LEFT], eFALSE);
    eTfDelayInit(delay->delay[RIGHT], eFALSE);
    return delay;
}

void eTfEffectDelayDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectDelayProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eASSERT_ALIGNED16(fx);
    eTfEffectDelay *delay = static_cast<eTfEffectDelay *>(fx);

    eU32 delayLeft = eFtoL(instr.params[TF_DELAY_LEFT] * TF_FX_DELAY_MAX_MILLISECONDS);
    eU32 delayRight = eFtoL(instr.params[TF_DELAY_RIGHT] * TF_FX_DELAY_MAX_MILLISECONDS);
    eF32 decay = instr.params[TF_DELAY_DECAY];

    eTfDelayUpdate(delay->delay[LEFT], synth.sampleRate, static_cast<eF32>(delayLeft));
    eTfDelayUpdate(delay->delay[RIGHT], synth.sampleRate, static_cast<eF32>(delayRight));

    eTfDelayProcess(delay->delay[LEFT], signal[LEFT], len, decay);
    eTfDelayProcess(delay->delay[RIGHT], signal[RIGHT], len, decay);
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT REVERB
// ---------------------------------------------------------------------------------------------------------------------------

const eF32 FIXEDGAIN    = 0.015f;
const eF32 SCALEWET     = 3.0f;
const eF32 SCALEDRY     = 2.0f;
const eF32 SCALEDAMP    = 0.4f;
const eF32 SCALEROOM    = 0.28f;
const eF32 OFFSETROOM   = 0.7f;
const eInt STEREOSPREAD = 23;

const eInt COMBTUNINGS[]    = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
const eInt ALLPASSTUNINGS[] = { 556, 441, 341, 225 };

eTfEffect * eTfEffectReverbCreate()
{
    eTfEffectReverb *reverb = static_cast<eTfEffectReverb *>(eAllocAligned(sizeof(eTfEffectReverb), 16));
    eMemSet(reverb, 0, sizeof(eTfEffectReverb));

    for (int i=0; i<TF_FX_REVERB_NUMCOMBS; i++)
    {
        eTfCombInit(reverb->comb[LEFT][i], COMBTUNINGS[i]);
        eTfCombInit(reverb->comb[RIGHT][i], COMBTUNINGS[i]);
    }

    for (int i=0; i<TF_FX_REVERB_NUMALLPASSES; i++)
    {
        eTfAllpassInit(reverb->allpass[LEFT][i], ALLPASSTUNINGS[i]);
        eTfAllpassInit(reverb->allpass[RIGHT][i], ALLPASSTUNINGS[i] + STEREOSPREAD);
    }

    return reverb;
}

void eTfEffectReverbDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectReverbProcess(eTfEffect *fx, eTfSynth &, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eASSERT_ALIGNED16(fx);
    eTfEffectReverb *reverb = static_cast<eTfEffectReverb *>(fx);

    eF32 roomsize          = instr.params[TF_REVERB_ROOMSIZE] * SCALEROOM + OFFSETROOM;
    eF32 damp              = instr.params[TF_REVERB_DAMP] * SCALEDAMP;
    eF32 wet               = instr.params[TF_REVERB_WET] * SCALEWET;
    eF32 dry               = (1.0f - instr.params[TF_REVERB_WET]) * SCALEDRY;
    eF32 width             = instr.params[TF_REVERB_WIDTH];
    eF32 wet1              = wet * (width / 2.0f + 0.5f);
    eF32 wet2              = wet * ((1.0f - width) / 2.0f);
    eF32 dry0              = dry;
    eF32 cmbFeedback       = roomsize;
    eF32 apsFeedback	   = 0.5f;
    eF32 damp1             = damp;
    eF32 gain              = FIXEDGAIN;
    eF32 damp2             = 1.0f - damp;

	//eSwap<eF32*>(signal[LEFT], signal[RIGHT]);

	if (len > TF_MAXFRAMESIZE)
		return;

    // erase mixbuffer
    eF32 *signals_mix[2];
    signals_mix[LEFT] = &reverb->mixBuffers[0];
    signals_mix[RIGHT] = &reverb->mixBuffers[TF_MAXFRAMESIZE];
    eMemSet(signals_mix[LEFT], 0, sizeof(eF32)*len);
    eMemSet(signals_mix[RIGHT], 0, sizeof(eF32)*len);

    // run comb filters in parallel
    for (eU32 i=0;i<TF_FX_REVERB_NUMCOMBS; i++)
    {
        eF32 *signals_out[2];
        signals_out[LEFT] = &reverb->combBuffers[TF_MAXFRAMESIZE * 2 * i];
        signals_out[RIGHT] = &reverb->combBuffers[TF_MAXFRAMESIZE * 2 * i + 1];
        eTfCombProcess(reverb->comb[LEFT][i], reverb->comb[RIGHT][i], damp1, damp2, cmbFeedback, gain, signal, signals_out, len);
        eTfSignalMix(signals_mix, signals_out, len, 0.5f);
    }

    // run allpass filters in serial
    for (eU32 i=0;i<TF_FX_REVERB_NUMALLPASSES; i++)
    {
        eTfAllpassProcess(reverb->allpass[LEFT][i], reverb->allpass[RIGHT][i], apsFeedback, signals_mix, signals_mix, len);
    }

    // create final signal
    eF32x2 wet1x2 = eSimdSetAll(wet1);
    eF32x2 wet2x2 = eSimdSetAll(wet2);
    eF32x2 dry0x2 = eSimdSetAll(dry0);

	//eSwap<eF32*>(signal[LEFT], signal[RIGHT]);

    eF32 *dryL = signal[LEFT];
    eF32 *dryR = signal[RIGHT];
    eF32 *wetL = signals_mix[LEFT];
    eF32 *wetR = signals_mix[RIGHT];

    while(len--)
    {
        eF32x2 in = eSimdSet2(*dryL, *dryR);
        eF32x2 lwet1 = eSimdSet2(*wetL, *wetR);
		eF32x2 lwet2 = eSimdSet2(*wetR++, *wetL++);
        eF32x2 out = eSimdSetAll(0.0f);

        out = eSimdAdd(
                eSimdAdd(
                    eSimdMul(lwet1, wet1x2),
                    eSimdMul(lwet2, wet2x2)),
                eSimdMul(in, dry0x2));

        eSimdStore2(out, *dryL++, *dryR++);
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT DISTORTION
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectDistortionCreate()
{
    eTfEffectDistortion *dist = static_cast<eTfEffectDistortion *>(eAllocAligned(sizeof(eTfEffectDistortion), 16));
    eMemSet(dist, 0, sizeof(eTfEffectDistortion));

    dist->generatedAmount = -1.0f;
    return dist;
}

void eTfEffectDistortionDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectDistortionProcess(eTfEffect *fx, eTfSynth &, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eASSERT_ALIGNED16(fx);
    eTfEffectDistortion *dist = static_cast<eTfEffectDistortion *>(fx);

    eF32 amount = 1.0f - instr.params[TF_DISTORT_AMOUNT];
    if (amount != dist->generatedAmount)
    {
        dist->generatedAmount = amount;
        for (eU32 base = 0; base<32768; base++)
            dist->powTable[base] = ePow(base/32768.f, amount);
    }

    for(eU32 i=0;i<2;i++)
    {
        eF32 *in = signal[i];
        eU32 len2 = len;

        while(len2--)
        {
            eF32 val = *in;
            eF32 sign = eSign(val);
            eF32 abs  = eAbs(val);
            if (abs > 1.0f) abs = 1.0f;
            eU32 offs = eFtoL(abs * 32767.0f);
            *in++ = sign * dist->powTable[offs];
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT FORMANT
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectFormantCreate()
{
    eTfEffectFormant *formant = static_cast<eTfEffectFormant *>(eAllocAligned(sizeof(eTfEffectFormant), 16));
    eMemSet(formant, 0, sizeof(eTfEffectFormant));
    return formant;
}

void eTfEffectFormantDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectFormantProcess(eTfEffect *fx, eTfSynth &, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eASSERT_ALIGNED16(fx);
    eTfEffectFormant *formant = static_cast<eTfEffectFormant *>(fx);

    const eF64 coeff[5][11]= {
        { 3.11044e-06,
            8.943665402,    -36.83889529,    92.01697887,    -154.337906,    181.6233289,
            -151.8651235,   89.09614114,    -35.10298511,    8.388101016,    -0.923313471  ///A
        },
        {4.36215e-06,
        8.90438318,    -36.55179099,    91.05750846,    -152.422234,    179.1170248,  ///E
        -149.6496211,87.78352223,    -34.60687431,    8.282228154,    -0.914150747
        },
        { 3.33819e-06,
        8.893102966,    -36.49532826,    90.96543286,    -152.4545478,    179.4835618,
        -150.315433,    88.43409371,    -34.98612086,    8.407803364,    -0.932568035  ///I
        },
        {1.13572e-06,
        8.994734087,    -37.2084849,    93.22900521,    -156.6929844,    184.596544,   ///O
        -154.3755513,    90.49663749,    -35.58964535,    8.478996281,    -0.929252233
        },
        {4.09431e-07,
        8.997322763,    -37.20218544,    93.11385476,    -156.2530937,    183.7080141,  ///U
        -153.2631681,    89.59539726,    -35.12454591,    8.338655623,    -0.910251753
        }
    };

    eU32 mode          = eFtoL(instr.params[TF_FORMANT_MODE] * 4.0f);
    eF32 wet           = instr.params[TF_FORMANT_WET];
    eF32 wet_inv    = 1.0f - wet;

    const eF64 *co_vow = coeff[mode];
    eF64 res = 0.0f;

    for(eU32 i=0;i<2;i++)
    {
        eF32 *in = signal[i];
        eF64 *mem = i==0 ? formant->memoryL : formant->memoryR;

        eU32 len2 = len;
        while (len2--)
        {
            res = co_vow[0] * (eF64)*in;

            for (eU32 j=0;j<TF_FX_FORMANT_MEMSIZE;j++)
                res += co_vow[j+1] * mem[j];

            for(eU32 j=TF_FX_FORMANT_MEMSIZE-1;j>0;j--)
                mem[j] = mem[j-1];

            mem[0] = res;

            *in *= wet_inv;
            *in++ += (eF32)res * wet;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT EQ
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectEqCreate()
{
    eTfEffectEq *eq = static_cast<eTfEffectEq *>(eAllocAligned(sizeof(eTfEffectEq), 16));
    eMemSet(eq, 0, sizeof(eTfEffectEq));
    return eq;
}

void eTfEffectEqDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectEqProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eASSERT_ALIGNED16(fx);
    eTfEffectEq *eq = static_cast<eTfEffectEq *>(fx);

    eF32 gain[3];
    for(eU32 i=0;i<3;i++)
    {
        gain[i] = instr.params[TF_EQ_LOW + i];
        if (gain[i] <= 0.5f)    gain[i] *= 2.0f;
        else                    gain[i] = ePow((gain[i] - 0.5f) * 2.0f, 2.0f) * 10.0f + 1.0f;
    }

    // Calculate filter cutoff frequencies
    eF32 m_lf = 2.0f * eSin(ePI * (880.0f / synth.sampleRate));
    eF32 m_hf = 2.0f * eSin(ePI * (5000.0f / synth.sampleRate));

    eF32 *in1 = signal[0];
    eF32 *in2 = signal[1];

    eF32x2 lf = eSimdSetAll(m_lf);
    eF32x2 hf = eSimdSetAll(m_hf);
    eF32x2 lg = eSimdSetAll(gain[0]);
    eF32x2 mg = eSimdSetAll(gain[1]);
    eF32x2 hg = eSimdSetAll(gain[2]);

    while(len--)
    {
        eF32x2 val = eSimdSet2(*in1, *in2);

        // Locals
        eF32x2  l,m,h;      // Low / Mid / High - Sample Values

        // Filter #1 (lowpass)
        //m_f1p0  += (lf * (val   - m_f1p0));
        eq->m_f1p0 = eSimdFma(eq->m_f1p0, lf, eSimdSub(val, eq->m_f1p0));
        //m_f1p1  += (lf * (m_f1p0 - m_f1p1));
        eq->m_f1p1 = eSimdFma(eq->m_f1p1, lf, eSimdSub(eq->m_f1p0, eq->m_f1p1));
        //m_f1p2  += (lf * (m_f1p1 - m_f1p2));
        eq->m_f1p2 = eSimdFma(eq->m_f1p2, lf, eSimdSub(eq->m_f1p1, eq->m_f1p2));
        //m_f1p3  += (lf * (m_f1p2 - m_f1p3));
        eq->m_f1p3 = eSimdFma(eq->m_f1p3, lf, eSimdSub(eq->m_f1p2, eq->m_f1p3));

        l = eq->m_f1p3;

        // Filter #2 (highpass)
        //m_f2p0  += (hf * (val   - m_f2p0));
        eq->m_f2p0 = eSimdFma(eq->m_f2p0, hf, eSimdSub(val, eq->m_f2p0));
        //m_f2p1  += (hf * (m_f2p0 - m_f2p1));
        eq->m_f2p1 = eSimdFma(eq->m_f2p1, hf, eSimdSub(eq->m_f2p0, eq->m_f2p1));
        //m_f2p2  += (hf * (m_f2p1 - m_f2p2));
        eq->m_f2p2 = eSimdFma(eq->m_f2p2, hf, eSimdSub(eq->m_f2p1, eq->m_f2p2));
        //m_f2p3  += (hf * (m_f2p2 - m_f2p3));
        eq->m_f2p3 = eSimdFma(eq->m_f2p3, hf, eSimdSub(eq->m_f2p2, eq->m_f2p3));

        h = eSimdSub(eq->m_sdm3, eq->m_f2p3);

        // Calculate midrange (signal - (low + high))
        m = eSimdSub(eq->m_sdm3, eSimdAdd(h, l));

        // Scale, Combine and store
        l = eSimdMul(l, lg);
        m = eSimdMul(m, mg);
        h = eSimdMul(h, hg);

        // Shuffle history buffer
        eq->m_sdm3 = eq->m_sdm2;
        eq->m_sdm2 = eq->m_sdm1;
        eq->m_sdm1 = val;

        eF32x2 out = eSimdAdd(eSimdAdd(l, m), h);

        eSimdStore2(out, *in1, *in2);
        in1++;
        in2++;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT CHORUS
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectChorusCreate()
{
    eTfEffectChorus *chorus = static_cast<eTfEffectChorus *>(eAllocAligned(sizeof(eTfEffectChorus), 16));
    eMemSet(chorus, 0, sizeof(eTfEffectChorus));

    eRandom rand;

    rand.SeedRandomly();

    for(eU32 i=0; i<2*TF_FX_CHORUS_DELAYCOUNT; i++)
    {
        eTfDelayInit(chorus->delay[i], eTRUE);
        chorus->lfoPhase[i] = rand.NextFloat();
    }

    return chorus;
}

void eTfEffectChorusDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectChorusProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eTfEffectChorus *chorus = static_cast<eTfEffectChorus *>(fx);

    eF32 depth = instr.params[TF_CHORUS_DEPTH];
    eF32 gain = instr.params[TF_CHORUS_GAIN];
    eF32 freq = instr.params[TF_CHORUS_RATE];

    const eF32 range = TF_FX_CHORUS_DELAY_MAX - TF_FX_CHORUS_DELAY_MIN;
    freq = (freq * freq) / synth.sampleRate * len * 50.0f;
    gain *= 0.7f;

    for(eU32 i=0; i<2 * TF_FX_CHORUS_DELAYCOUNT; i++)
    {
        eF32 sine = eSin(chorus->lfoPhase[i])+1.0f/2.0f;
        eF32 delay = (sine * depth * range) + TF_FX_CHORUS_DELAY_MIN;
        delay = eClamp<eF32>(TF_FX_CHORUS_DELAY_MIN, delay, TF_FX_CHORUS_DELAY_MAX);
        eTfDelayUpdate(chorus->delay[i], synth.sampleRate, delay);
        eTfDelayProcess(chorus->delay[i], signal[i%2], len, gain);
        chorus->lfoPhase[i] += freq;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------
//  EFFECT FLANGER
// ---------------------------------------------------------------------------------------------------------------------------

eTfEffect * eTfEffectFlangerCreate()
{
    eTfEffectFlanger *flanger = static_cast<eTfEffectFlanger *>(eAllocAligned(sizeof(eTfEffectFlanger), 16));
    eMemSet(flanger, 0, sizeof(eTfEffectFlanger));
    return flanger;
}

void eTfEffectFlangerDelete(eTfEffect *fx)
{
    eFreeAligned(fx);
}

void eTfEffectFlangerProcess(eTfEffect *fx, eTfSynth &synth, eTfInstrument &instr, eF32 **signal, eU32 len)
{
    eTfEffectFlanger *flanger = static_cast<eTfEffectFlanger *>(fx);

    eF32 *pcmleft  = signal[0];
    eF32 *pcmright = signal[1];

    eF32 amp = instr.params[TF_FLANGER_AMPLITUDE];
    eF32 freq = instr.params[TF_FLANGER_FREQUENCY];
    eF32 lfo = instr.params[TF_FLANGER_LFO];
    eF32 wet = instr.params[TF_FLANGER_WET];

    const eF32 DELAYMIN = (eF32)synth.sampleRate * 0.1f / 1000.0f;    // 0.1 ms delay min
    const eF32 DELAYMAX = (eF32)synth.sampleRate *12.1f / 1000.0f;    // 12.1 ms delay max
    eF32 inc = 0.1f*(eF32)len;

    while(len--)
    {
        if (flanger->bidi==0)
        {
            flanger->lfocount += lfo * inc;

            if (flanger->lfocount > freq)
            {
                flanger->lfocount = 1.0f;
                flanger->bidi = 1;
            }
        }
        else
        {
            flanger->lfocount -= lfo * inc;

            if (flanger->lfocount < 0.0f)
            {
                flanger->lfocount = 0.01f;
                flanger->bidi = 0;
            }
        }

        eF32 frequency = flanger->lfocount;

        if (frequency==0.0)
            frequency=1.0f;

        if(flanger->lastBpm != frequency)
        {
            flanger->angle0 += (eF32)((eF64)flanger->angle * frequency * 120.0f / (synth.sampleRate * 4.0f * 60.0f / ePI));
            flanger->angle1 += (eF32)((eF64)flanger->angle * (1.0f - frequency) * 120.0f / (synth.sampleRate * 4.0f * 60.0f / ePI));
            flanger->lastBpm = frequency;
            flanger->angle = 0;
        }

        eInt deltaleft = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * amp * 4096.0f * (1.0f - eCos(flanger->angle0 + flanger->angle * frequency / (synth.sampleRate * 4 * 60 / ePI))));
        eInt deltaright = eFtoL(DELAYMIN + ((DELAYMAX - DELAYMIN) / 8192.0f) * amp * 4096.0f * (1.0f - eCos(flanger->angle1 + flanger->angle * frequency / (synth.sampleRate * 4 * 60 / ePI))));

        flanger->angle++;

        eInt ppleft = flanger->buffpos - deltaleft;
        eInt ppright = flanger->buffpos - deltaright;

        if(ppleft<0)
            ppleft += TF_FX_FLANGERBUFFSIZE;

        if(ppright<0)
            ppright += TF_FX_FLANGERBUFFSIZE;

        eF32 l = *pcmleft - wet * flanger->buffleft[ppleft];

        if (l<-1.0f)
            l=-1.0f;
        else if (l>1.0f)
            l=1.0f;

        eF32 r = *pcmright - wet * flanger->buffright[ppright];

        if (r<-1.0f)
            r=-1.0f;
        else if (r>1.0f)
            r=1.0f;

        eUndenormalise(l);
        eUndenormalise(r);

        *pcmleft = l;
        *pcmright = r;

        flanger->buffleft[flanger->buffpos] = l;
        flanger->buffright[flanger->buffpos] = r;

        pcmleft++;
        pcmright++;
        flanger->buffpos++;

        if (flanger->buffpos >= TF_FX_FLANGERBUFFSIZE)
            flanger->buffpos = 0;
    }
}
