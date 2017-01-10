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

#ifndef TF_RECORDER_HPP
#define TF_RECORDER_HPP

#include "../PluginProcessor.h"

class eTfRecorder 
{
public:
	eTfRecorder();
	~eTfRecorder();

	static eTfRecorder&         getInstance();

	void				        reset();
	void				        startRecording();
	void				        stopRecording();
	bool				        isRecording() const;

    bool				        saveToFile(File &file);
	void				        recordEvent(eTfEvent e);
	void				        setTempo(eU16 tempo);

	eS32 				        addSynth(Tunefish4AudioProcessor *synth);
	void				        removeSynth(Tunefish4AudioProcessor *synth);

private:
    CriticalSection             m_cs;
	eArray<eTfEvent>	        m_events;
	eU16				        m_tempo;
    Tunefish4AudioProcessor	*	m_synths[TF_MAX_INSTR];
	eBool				        m_isRecording;

	static eTfRecorder          m_recorder;
};

#endif 
