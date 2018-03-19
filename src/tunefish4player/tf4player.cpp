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

#include "tf4player.hpp"
#ifdef eENIGMA
#include "../system/datastream.hpp"
#else
#include "datastream.hpp"
#endif

void eTfPlayerInit(eTfPlayer &player)
{
	eTfSynthInit(player.synth);

	player.song.instrCount = 0;
	player.playing = eFALSE;
	player.volume = 0.1f;
	player.playbackBuffer = nullptr;

	for (auto i = 0; i < TF_MAX_INSTR; i++)
		player.instrumentMuted[i] = false;
}

void eTfPlayerMuteInstrument(eTfPlayer& player, eU32 index, eBool muted)
{
	eASSERT(index < TF_MAX_INSTR);
	player.instrumentMuted[index] = muted;
}

void eTfPlayerReverseMutes(eTfPlayer& player)
{
	for (eU32 i = 0; i < TF_MAX_INSTR; i++)
	{
		player.instrumentMuted[i] = !player.instrumentMuted[i];
	}
}

void eTfPlayerSetSampleRate(eTfPlayer &player, eU32 sampleRate)
{
	player.synth.sampleRate = sampleRate;
}

void eTfPlayerLoadSong(eTfPlayer &player, const eU8 *data, eU32 len, eF32 delay)
{
	eTfPlayerUnloadSong(player);
	if (!len) return;

	eDataStream stream(data, len);

	eTfSong &song = player.song;
	eTfSynth &synth = player.synth;

	eU16 eventCounts[TF_MAX_INSTR];
	song.instrCount = stream.ReadU16();
	song.tempo = stream.ReadU16();

	// calculate speed values
	const eU32 rows_per_beat = 4;
	const eU32 rows_per_min = song.tempo * rows_per_beat;
	const eF32 rows_per_sec = (eF32)rows_per_min / 60.0f;
	const eF32 secs_per_row = 1.0f / rows_per_sec;

	//  init instruments & event arrays
	// -----------------------------------------------------------------------------------------
	for (eU32 i = 0; i < song.instrCount; i++)
	{
		synth.instr[i] = new eTfInstrument;
		eTfInstrumentInit(*synth.instr[i]);
		eU16 eventCount = stream.ReadU16();
		eventCounts[i] = eventCount;
		eArrayInit(reinterpret_cast<ePtrArray*>(&song.events[i]), sizeof(eTfEvent), eventCount);
	}

	//  read instruments
	// -----------------------------------------------------------------------------------------
	eU32 tagInst = stream.ReadU32();
	eASSERT(eMemEqual(&tagInst, "INST", 4));

	for (eU32 j = 0; j < song.instrCount; j++)
	{
		for (eU8 i = 0; i < TF_PARAM_COUNT; i++)
		{
			eF32 p = (eF32)stream.ReadU8() / 100.0f;
			synth.instr[j]->params[i] = p;
		}
	}

	//  read song
	// -----------------------------------------------------------------------------------------
	eU32 tagSong = stream.ReadU32();
	eASSERT(eMemEqual(&tagSong, "SONG", 4));

	for (eU32 j = 0; j < song.instrCount; j++)
	{
		eArray<eTfEvent> &events = song.events[j];

		// read times
		eU32 row = 0;
		for (eU32 i = 0; i < eventCounts[j]; i++)
		{
			eU32 diff = stream.ReadU16();
			row += diff;
			events[i].time = (eF32)row * secs_per_row + delay;
		}

		// read notes
		for (eU32 i = 0; i < eventCounts[j]; i++)
		{
			events[i].note = stream.ReadU8();
		}

		// read velocities
		for (eU32 i = 0; i < eventCounts[j]; i++)
		{
			events[i].velocity = stream.ReadU8();
		}
	}

	eU32 tagEnd = stream.ReadU32();
	eASSERT(eMemEqual(&tagEnd, "ENDS", 4));
}

void eTfPlayerUnloadSong(eTfPlayer &player)
{
	for (eU32 i = 0; i < TF_MAX_INSTR; i++)
	{
		player.song.events[i].clear();
		player.song.instrCount = 0;
		eDelete(player.synth.instr[i]);
	}
}

void eTfPlayerProcess(eTfPlayer &player, const eS16 **output)
{
	if (!player.playing)
		return;

	if (player.playbackBuffer)
	{
		if (player.playbackBufferOffset < player.playbackBufferLength)
		{
			*output = &player.playbackBuffer[player.playbackBufferOffset];
			player.playbackBufferOffset += TF_FRAMESIZE * 2;
		}
		return;
	}

	eF32 timeStep = (eF32)TF_FRAMESIZE / player.synth.sampleRate;
	eF32 nextTime = player.time + timeStep;

	eTfSong &song = player.song;
	eTfSynth &synth = player.synth;

	for (eU32 j = 0; j < song.instrCount; j++)
	{
		if (player.instrumentMuted[j])
			continue;

		eArray<eTfEvent> &events = song.events[j];

		for (eU32 i = 0; i < events.size(); i++)
		{
			eTfEvent &ev = events[i];

			if (ev.time >= player.time && ev.time < nextTime)
			{
				if (ev.note && ev.instr >= 0)
				{
					eTfInstrument *instr = synth.instr[j];

					if (instr)
					{
						if (!ev.velocity)
							eTfInstrumentNoteOff(*instr, ev.note);
						else
							eTfInstrumentNoteOn(*instr, ev.note, ev.velocity);
					}
				}
			}
		}
	}

	eF32 *tempSignals[2];
	tempSignals[0] = &player.tempSignal[0];
	tempSignals[1] = &player.tempSignal[TF_FRAMESIZE];

	eF32 *signals[2];
	signals[0] = &player.outputSignal[0];
	signals[1] = &player.outputSignal[TF_FRAMESIZE];

	eMemSet(player.outputSignal, 0, sizeof(eF32)*TF_FRAMESIZE * 2);

	//eU32 polyPhony = 0;
	for (eU32 i = 0; i < TF_MAX_INSTR; i++)
	{
		eTfInstrument *instr = player.synth.instr[i];

		if (instr)
		{
			eMemSet(player.tempSignal, 0, sizeof(eF32)*TF_FRAMESIZE * 2);
			eTfInstrumentProcess(player.synth, *instr, tempSignals, TF_FRAMESIZE);
			//polyPhony += eTfInstrumentGetPolyphony(*instr);
			eTfSignalMix(signals, tempSignals, TF_FRAMESIZE, 1.0f);
		}
	}

	eTfSignalToS16(signals, player.outputFinal, 1000.0f * TF_MASTER_VOLUME * player.volume, TF_FRAMESIZE);
	*output = player.outputFinal;

	player.time = nextTime;
}

void eTfPlayerSeek(eTfPlayer &player, eF32 time)
{
	eTfPlayerAllNotesOff(player);
	player.time = time;
}

void eTfPlayerStart(eTfPlayer &player, eF32 time)
{
	player.time = time;
	player.playing = eTRUE;
}

void eTfPlayerStop(eTfPlayer &player)
{
	player.playing = eFALSE;
	eTfPlayerAllNotesOff(player);
}

void eTfPlayerAllNotesOff(eTfPlayer &player)
{
	for (eU32 i = 0; i < TF_MAX_INSTR; i++)
	{
		eTfInstrument *instr = player.synth.instr[i];

		if (instr)
		{
			eTfInstrumentAllNotesOff(*instr);
		}
	}
}

eF32 eTfPlayerGetSongLength(eTfPlayer& player)
{
	eTfSong &song = player.song;
	eF32 maxTime = 0.0f;

	for (eU32 i = 0; i < song.instrCount; i++)
	{
		eArray<eTfEvent> &events = song.events[i];
		eTfEvent &lastEvent = events[events.size()-1];
		if (lastEvent.time > maxTime)
			maxTime = lastEvent.time;
	}

	return maxTime;
}

void eTfPlayerReverseAllEvents(eTfPlayer& player)
{
	eTfSong &song = player.song;

	for (eU32 i = 0; i < song.instrCount; i++)
	{
		eTfPlayerReverseEvents(player, i);
	}
}

void eTfPlayerReverseEvents(eTfPlayer& player, eU32 instrument)
{
	eTfSong &song = player.song;
	eArray<eTfEvent> &events = song.events[instrument];
	const eU32 eventCount = events.size();
	const eU32 halfEventCount = eventCount / 2;
	const eF32 songLength = eTfPlayerGetSongLength(player);

	// reverse time values
	for (eU32 i=0; i<eventCount; i++)
	{
		eTfEvent &ev = events[i];
		eF32 newTime = songLength - ev.time;
		ev.time = newTime;
	}

	// reverse order
	for (eU32 i=0; i<halfEventCount; i++)
	{
		eTfEvent &ev0 = events[i];
		eTfEvent &ev1 = events[eventCount - i - 1];

		eTfEvent temp = ev0;
		ev0 = ev1;
		ev1 = temp;
	}

	// correct note off events
	for (eU32 i = 0; i < eventCount-2; i++)
	{
		eTfEvent &ev0 = events[i];
		eTfEvent &ev1 = events[i+1];
		eTfEvent &ev2 = events[i+2];

		if (ev0.velocity > 0 && ev1.velocity == 0 && ev2.velocity > 0)
		{
			if (ev1.time == ev0.time)
			{
				ev1.time = ev2.time;
			}
		}
	}
}

eU32 eTfPlayerRecordToBuffer(eTfPlayer& player, eF32 endTime, eS16 **buffer)
{
	eU32 samplesTotalPerChannel = eFtoL(static_cast<eF32>(player.synth.sampleRate) * endTime);
	eU32 blocksTotal = eU32(samplesTotalPerChannel / TF_FRAMESIZE) + 1;
	eU32 samplesTotal = blocksTotal * TF_FRAMESIZE * 2;
	eU32 frameSizeBytes = TF_FRAMESIZE * 4;
	eU32 offset = 0;

	eS16 *output = new eS16[samplesTotal];

	eTfPlayerStart(player, 0.0f);

	for (eU32 i = 0; i < blocksTotal; i++)
	{
		const eS16 *data = nullptr;
		eTfPlayerProcess(player, &data);
		eMemCopy(&reinterpret_cast<eU8*>(output)[offset], data, frameSizeBytes);
		offset += frameSizeBytes;
	}

	eTfPlayerStop(player);
	*buffer = output;
	return samplesTotal;
}

void eTfPlayerReverseBuffer(eS16* buffer, eU32 totalSamples)
{
	const eU32 halfSamples = totalSamples / 2;

	for (eU32 i=0; i<halfSamples; i++)
	{
		eS16 temp = buffer[i];
		buffer[i] = buffer[totalSamples - i - 1];
		buffer[totalSamples - i - 1] = temp;
	}
}

void eTfPlayerSetPlaybackBuffer(eTfPlayer& player, eS16* buffer, eU32 totalSamples)
{
	player.playbackBuffer = buffer;
	player.playbackBufferOffset = 0;
	player.playbackBufferLength = totalSamples;
}
