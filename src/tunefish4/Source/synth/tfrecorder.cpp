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

#include "tfrecorder.hpp"

eTfRecorder eTfRecorder::m_recorder;

#define ePARAM_ZERO(x) params[x] = 0.0f;
#define ePARAM_ZERO_IF_ZERO(x, y) if (params[x] < 0.01f) params[y] = 0.0f;
#define ePARAM_ZERO_IF_OFF(x, y) if (params[x] < 0.5f) params[y] = 0.0f;
#define ePARAM_ZERO_ALL_IF_ONE_ZERO(x1, x2, y) if (params[x1] < 0.01f || params[x2] < 0.01f) params[y] = params[x1] = params[x2] = 0.0f; 

static eBool isMMSourceUsed(eF32 *params, eU32 source)
{
	for (eU32 i=0; i<10; i++)
	{
		if (eFtoL(eRoundNearest(params[TF_MM1_SOURCE + i*3] * (eTfModMatrix::INPUT_COUNT-1))) == static_cast<int>(source))
			return eTRUE;
	}

	return eFALSE;
}

static void optimizeParams(eF32 *params)
{
	ePARAM_ZERO_IF_ZERO(TF_NOISE_AMOUNT, TF_NOISE_FREQ);
	ePARAM_ZERO_IF_ZERO(TF_NOISE_AMOUNT, TF_NOISE_BW);

	ePARAM_ZERO_IF_OFF(TF_LP_FILTER_ON, TF_LP_FILTER_CUTOFF);
	ePARAM_ZERO_IF_OFF(TF_LP_FILTER_ON, TF_LP_FILTER_RESONANCE);

	ePARAM_ZERO_IF_OFF(TF_HP_FILTER_ON, TF_HP_FILTER_CUTOFF);
	ePARAM_ZERO_IF_OFF(TF_HP_FILTER_ON, TF_HP_FILTER_RESONANCE);

    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_BANDWIDTH);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_NUMHARMONICS);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_DAMP);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_MODULATION);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_SLOP);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_OCTAVE);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_GLIDE);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_DETUNE);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_FREQ);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_DRIVE);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_UNISONO);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_SPREAD);
    ePARAM_ZERO_IF_ZERO(TF_GEN_VOLUME, TF_GEN_SCALE);

	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM1_SOURCE, TF_MM1_TARGET, TF_MM1_MOD);
    ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM2_SOURCE, TF_MM2_TARGET, TF_MM2_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM3_SOURCE, TF_MM3_TARGET, TF_MM3_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM4_SOURCE, TF_MM4_TARGET, TF_MM4_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM5_SOURCE, TF_MM5_TARGET, TF_MM5_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM6_SOURCE, TF_MM6_TARGET, TF_MM6_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM7_SOURCE, TF_MM7_TARGET, TF_MM7_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM8_SOURCE, TF_MM8_TARGET, TF_MM8_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM9_SOURCE, TF_MM9_TARGET, TF_MM9_MOD);
	ePARAM_ZERO_ALL_IF_ONE_ZERO(TF_MM10_SOURCE, TF_MM10_TARGET, TF_MM10_MOD);

	if (!isMMSourceUsed(params, eTfModMatrix::INPUT_LFO1))
	{
		ePARAM_ZERO(TF_LFO1_RATE);
		ePARAM_ZERO(TF_LFO1_DEPTH);
		ePARAM_ZERO(TF_LFO1_SHAPE);
		ePARAM_ZERO(TF_LFO1_SYNC);
	}

	if (!isMMSourceUsed(params, eTfModMatrix::INPUT_LFO2))
	{
		ePARAM_ZERO(TF_LFO2_RATE);
		ePARAM_ZERO(TF_LFO2_DEPTH);
		ePARAM_ZERO(TF_LFO2_SHAPE);
		ePARAM_ZERO(TF_LFO2_SYNC);
	}

	if (!isMMSourceUsed(params, eTfModMatrix::INPUT_ADSR1))
	{
		ePARAM_ZERO(TF_ADSR1_ATTACK);
		ePARAM_ZERO(TF_ADSR1_DECAY);
		ePARAM_ZERO(TF_ADSR1_SUSTAIN);
		ePARAM_ZERO(TF_ADSR1_RELEASE);
		ePARAM_ZERO(TF_ADSR1_SLOPE);
	}

	if (!isMMSourceUsed(params, eTfModMatrix::INPUT_ADSR2))
	{
		ePARAM_ZERO(TF_ADSR2_ATTACK);
		ePARAM_ZERO(TF_ADSR2_DECAY);
		ePARAM_ZERO(TF_ADSR2_SUSTAIN);
		ePARAM_ZERO(TF_ADSR2_RELEASE);
		ePARAM_ZERO(TF_ADSR2_SLOPE);
	}
}

eTfRecorder::eTfRecorder()
{
	eMemSet(m_synths, 0, sizeof(Tunefish4AudioProcessor*) * TF_MAX_INSTR);
	m_isRecording = eFALSE;
	m_tempo = 0;
}

eTfRecorder::~eTfRecorder()
{

}

eTfRecorder & eTfRecorder::getInstance()
{
	return m_recorder;
}

void eTfRecorder::reset()
{
	m_cs.enter();
	m_events.clear();
	m_cs.exit();
}

void eTfRecorder::startRecording()
{
	if (!m_isRecording)
	{
		reset();
		m_isRecording = eTRUE;
	}
}

void eTfRecorder::stopRecording()
{
	m_isRecording = eFALSE;
}

eBool eTfRecorder::isRecording() const
{
	return m_isRecording;
}

eBool eTfRecorder::saveToFile(File &fileBin)
{
	stopRecording();

    // write binary file
	// -------------------------------------------------------------------
    String fileName = fileBin.getFullPathName();
	File fileLog(fileName + ".log");
    File fileJSON(fileName + ".js");
	
	if (!fileBin.deleteFile())
		return eFALSE;

	if (!fileLog.deleteFile())
		return eFALSE;

	if (!fileJSON.deleteFile())
		return eFALSE;

    auto outBin = fileBin.createOutputStream();
    auto outLog = fileLog.createOutputStream();
    auto outJSON = fileJSON.createOutputStream();

    if (!outBin || !outLog || !outJSON)
    {
        return eFALSE;
    }

    m_cs.enter();

	// count stuff
	eU16 synthCount = 0;
	eU16 eventCount[TF_MAX_INSTR];

	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
		eventCount[i] = 0;
		if (m_synths[i] != nullptr)
			synthCount++;
	}

	for(eU32 i=0;i<m_events.size();i++)
	{
		eventCount[m_events[i].instr]++;
	}

	// write header values
    outBin->write(reinterpret_cast<const char *>(&synthCount), sizeof(eU16));
    outBin->write(reinterpret_cast<const char *>(&m_tempo), sizeof(eU16));

    outJSON->writeText("var tf_synthcount = ", false, false, nullptr);
    outJSON->writeText(String(synthCount), false, false, nullptr);
    outJSON->writeText(";\r\n", false, false, nullptr);

    outJSON->writeText("var tf_tempo = ", false, false, nullptr);
    outJSON->writeText(String(m_tempo), false, false, nullptr);
    outJSON->writeText(";\r\n", false, false, nullptr);

	outLog->writeText("Instruments: ", false, false, nullptr);
    outLog->writeText(String(synthCount), false, false, nullptr);
    outLog->writeText("\r\n", false, false, nullptr);

    outLog->writeText("Tempo: ", false, false, nullptr);
    outLog->writeText(String(m_tempo), false, false, nullptr);
    outLog->writeText("\r\n", false, false, nullptr);

	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
		if (m_synths[i] != nullptr)
		{
			outBin->write(reinterpret_cast<const char *>(&eventCount[i]), sizeof(eU16));

            outLog->writeText("Eventcount for instr ", false, false, nullptr);
            outLog->writeText(String(i), false, false, nullptr);
            outLog->writeText(": ", false, false, nullptr);
            outLog->writeText(String(eventCount[i]), false, false, nullptr);
            outLog->writeText("\r\n", false, false, nullptr);
		}
	}

    outLog->writeText("Instruments\r\n", false, false, nullptr);
    outLog->writeText("-----------------------------------------\r\n", false, false, nullptr);

	outBin->writeText("INST", false, false, nullptr);

	// optimize instruments
	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
        Tunefish4AudioProcessor *synth = m_synths[i];
		if (synth != nullptr)
		{
            eTfSynth *tf = synth->getSynth();
			optimizeParams(tf->instr[0]->params);
		}
	}

	// write instruments  (grouped by instruments)
    outJSON->writeText("var tf_instruments = [\r\n", false, false, nullptr);
	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
		Tunefish4AudioProcessor *synth = m_synths[i];

		if (synth != nullptr)
		{
			eTfSynth *tf = synth->getSynth();

			outLog->writeText("Params for instr ", false, false, nullptr);
            outLog->writeText(String(i), false, false, nullptr);
            outLog->writeText("\r\n", false, false, nullptr);
            outLog->writeText("-----------------------------------------\r\n", false, false, nullptr);
            
            outJSON->writeText("[", false, false, nullptr);

			for(eU32 j=0; j<TF_PARAM_COUNT; j++)
			{
				eF32 value = tf->instr[0]->params[j];
				eU8 ivalue = static_cast<eU8>(value * 100.0f);

                outLog->writeText(TF_NAMES[j], false, false, nullptr);
                outLog->writeText(": ", false, false, nullptr);
                outLog->writeText(String(value), false, false, nullptr);
                outLog->writeText(" -> ", false, false, nullptr);
                outLog->writeText(String(ivalue), false, false, nullptr);
                outLog->writeText("\r\n", false, false, nullptr);

                outJSON->writeText(String(value), false, false, nullptr);
                if (j < TF_PARAM_COUNT-1)
                    outJSON->writeText(", ", false, false, nullptr);

				outBin->write(reinterpret_cast<const char *>(&ivalue), sizeof(eU8));
			}

            outJSON->writeText("],\r\n", false, false, nullptr);
		}
	}
    outJSON->writeText("];\r\n", false, false, nullptr);

	/*
	// write instruments  (grouped by paramindex)		
	for(eU32 j=0; j<TF_PARAM_COUNT; j++)
	{
		for(eU32 i=0;i<TF_MAX_INSTR; i++) 
		{
			Tunefish4AudioProcessor *synth = m_synths[i];

			if (synth != nullptr)
			{
				eTfSynth *tf = synth->getSynth();

				eF32 value = tf->instr[0]->params[j];
				eU8 ivalue = static_cast<eU8>(value * 100.0f);

				outBin->write(reinterpret_cast<const char *>(&ivalue), sizeof(eU8));
			}
		}
	}
    */
	
    outLog->writeText("Events (Time, iTime, Instrument, Note, Velocity)\r\n", false, false, nullptr);
    outLog->writeText("-----------------------------------------\r\n", false, false, nullptr);

	// calculate speed values
    const eU32 rows_per_beat = 4;
	const eU32 rows_per_min = m_tempo * rows_per_beat;
    const eF32 rows_per_sec = static_cast<eF32>(rows_per_min) / 60.0f;

    // write events
    outJSON->writeText("var tf_song = [\r\n", false, false, nullptr);
	for(eU32 i=0;i<m_events.size();i++)
	{
		eTfEvent &e = m_events[i];

        outLog->writeText("Event: ", false, false, nullptr);
        outLog->writeText(String(e.time), false, false, nullptr);
        outLog->writeText("\t", false, false, nullptr);
        outLog->writeText(String(static_cast<eU32>(e.time * rows_per_sec)), false, false, nullptr);
		outLog->writeText("\t", false, false, nullptr);
		outLog->writeText(String(e.instr), false, false, nullptr);
		outLog->writeText("\t", false, false, nullptr);
		outLog->writeText(String(e.note), false, false, nullptr);
		outLog->writeText("\t", false, false, nullptr);
		outLog->writeText(String(e.velocity), false, false, nullptr);
		outLog->writeText("\r\n", false, false, nullptr);

        eU16 row = static_cast<eU16>(eFtoL(eRoundNearest(e.time * rows_per_sec)));

        outJSON->writeText("[", false, false, nullptr);
        outJSON->writeText(String(row), false, false, nullptr);
        outJSON->writeText(", ", false, false, nullptr);
        outJSON->writeText(String(e.instr), false, false, nullptr);
        outJSON->writeText(", ", false, false, nullptr);
        outJSON->writeText(String(e.note), false, false, nullptr);
        outJSON->writeText(", ", false, false, nullptr);
        outJSON->writeText(String(e.velocity), false, false, nullptr);
        outJSON->writeText("],\r\n", false, false, nullptr);
	}
    outJSON->writeText("];\r\n", false, false, nullptr);

	outBin->writeText("SONG", false, false, nullptr);

	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
        Tunefish4AudioProcessor *synth = m_synths[i];

		if (synth != nullptr)
		{
			// write times
			eU16 oldRow = 0;
			for(eU32 j=0;j<m_events.size();j++)
			{
				eTfEvent &e = m_events[j];
				if (e.instr == i)
				{
                    eU16 row = static_cast<eU16>(eFtoL(eRoundNearest(e.time * rows_per_sec)));
                    eU16 diff = row - oldRow;
                    oldRow = row;

                    outBin->write(reinterpret_cast<const char *>(&diff), sizeof(eU16));
				}
			}

			// write notes
			for(eU32 j=0;j<m_events.size();j++)
			{
				eTfEvent &e = m_events[j];
				if (e.instr == i)
				{
					outBin->write(reinterpret_cast<const char *>(&e.note), sizeof(eU8));
				}
			}

			// write velocities
			for(eU32 j=0;j<m_events.size();j++)
			{
				eTfEvent &e = m_events[j];
				if (e.instr == i)
				{
					eU8 vel = e.velocity;
					outBin->write(reinterpret_cast<const char *>(&vel), sizeof(eU8));
				}	
			}
		}
	}

    outBin->writeText("ENDS", false, false, nullptr);

    outBin.reset();
    outLog.reset();
    outJSON.reset();

	m_cs.exit();

	// write header file
	// -------------------------------------------------------------------
	File headerIn(fileName);
	File headerOut(fileName + ".h");

    if (!headerOut.deleteFile())
        return eFALSE;

    auto inHeader = headerIn.createInputStream();
    
    if (!inHeader)
        return eFALSE;

    MemoryBlock input;
    size_t inputLen = inHeader->readIntoMemoryBlock(input);
    inHeader.reset();

    auto outHeader = headerOut.createOutputStream();

    if (!outHeader)
    {
        return eFALSE;
    }
	
    outHeader->writeText("const unsigned char song[] = {\r\n", false, false, nullptr);
	
	for(eS32 i=0;i<inputLen;i++)
	{
		eBool lastByte = i == inputLen-1;
		eBool lastInRow = i % 16 == 15;
		eBool firstInRow = i % 16 == 0;

		if (firstInRow)
            outHeader->writeText("\t", false, false, nullptr);

		String numstr = String::toHexString(static_cast<const eU8>(input[i]));        
		if (numstr.length() == 1)
			numstr = "0" + numstr;

        outHeader->writeText(String("0x") + numstr, false, false, nullptr);

		if (!lastByte)
            outHeader->writeText(", ", false, false, nullptr);

		if (lastInRow || lastByte)
            outHeader->writeText("\r\n", false, false, nullptr);
	}

    outHeader->writeText("};\r\n", false, false, nullptr);

	return eTRUE;
}

void eTfRecorder::recordEvent(eTfEvent e)
{
	if (m_isRecording)
	{
		m_cs.enter();
		m_events.push(e);
		m_cs.exit();
	}
}

void eTfRecorder::setTempo(eU16 tempo)
{
	m_tempo = tempo;
}

eS32 eTfRecorder::addSynth(Tunefish4AudioProcessor *synth)
{
	eS32 index = -1;

	m_cs.enter();
	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
		if (m_synths[i] == nullptr) 
		{
			m_synths[i] = synth;
			index = i;
			break;
		}
	}
	m_cs.exit();

	return index;
}

void eTfRecorder::removeSynth(Tunefish4AudioProcessor *synth)
{
	m_cs.enter();
	for(eU32 i=0;i<TF_MAX_INSTR; i++) 
	{
		if (m_synths[i] == synth) 
		{
			m_synths[i] = nullptr;
			break;
		}
	}
	m_cs.exit();
}
