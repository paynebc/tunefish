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

#ifndef TF4PLAYER_HPP
#define TF4PLAYER_HPP

#ifdef eENIGMA
#include "../system/system.hpp"
#include "tf4.hpp"
#else
#include "../tunefish4/Source/runtime/system.hpp"
#include "../tunefish4/Source/synth/tf4.hpp"
#endif

struct eTfPlayer
{
	eTfSong		song;
	eTfSynth	synth;

	eF32		volume;
	eF32		time;
	eBool		playing;

	eS16 *      playbackBuffer;
	eU32		playbackBufferOffset;
	eU32		playbackBufferLength;

	eBool		instrumentMuted[TF_MAX_INSTR];
	eF32		outputSignal[sizeof(eF32)*TF_FRAMESIZE * 2];
	eF32		tempSignal[sizeof(eF32)*TF_FRAMESIZE * 2];
	eS16		outputFinal[sizeof(eF32)*TF_FRAMESIZE];
};

void		eTfPlayerInit(eTfPlayer &player);
void		eTfPlayerMuteInstrument(eTfPlayer &player, eU32 index, eBool muted);
void		eTfPlayerReverseMutes(eTfPlayer &player);
void		eTfPlayerSetSampleRate(eTfPlayer &player, eU32 sampleRate);
void		eTfPlayerLoadSong(eTfPlayer &player, const eU8 *data, eU32 len, eF32 delay);
void		eTfPlayerUnloadSong(eTfPlayer &player);
void		eTfPlayerProcess(eTfPlayer &player, const eS16 **output);
void		eTfPlayerStart(eTfPlayer &player, eF32 time);
void		eTfPlayerStop(eTfPlayer &player);
void		eTfPlayerSeek(eTfPlayer &player, eF32 time);
void		eTfPlayerAllNotesOff(eTfPlayer &player);
eF32		eTfPlayerGetSongLength(eTfPlayer &player);
void		eTfPlayerReverseAllEvents(eTfPlayer &player);
void		eTfPlayerReverseEvents(eTfPlayer &player, eU32 instrument);
eU32		eTfPlayerRecordToBuffer(eTfPlayer& player, eF32 endTime, eS16 **buffer);
void	    eTfPlayerReverseBuffer(eS16 *buffer, eU32 totalSamples);
void		eTfPlayerSetPlaybackBuffer(eTfPlayer &player, eS16 * buffer, eU32 totalSamples);

#endif
