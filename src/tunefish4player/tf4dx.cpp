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

#include <stdio.h>
#define NOMINMAX
#include <windows.h>
#include <dsound.h>

#ifdef eENIGMA
#include "synth.hpp"
#else
#include "tf4dx.hpp"
#include "tf4player.hpp"
#include "threading.hpp"
#endif

static struct eTfDx
{
	eTfDx()
	{
		dsound = nullptr;
		dsoundBuffer = nullptr;
		nextWriteOffset = 0;
		bufferSize = 0;
		sampleRate = 0;
		playing = eFALSE;
		joinRequest = eFALSE;
		thread = nullptr;
	}

	IDirectSound8 *         dsound;
	IDirectSoundBuffer8 *   dsoundBuffer;
	eU32                    nextWriteOffset;
	eU32                    bufferSize;
	eU32                    sampleRate;
	eArray<eTfPlayer *>		players;
	eBool                   playing;
	eBool                   joinRequest;
	class eTfDxThread *     thread;
} s_tfdx;

class eTfDxThread : public eThread
{
public:
	eTfDxThread()
	{
		SetPriority(eTHP_HIGH);
		Start();
		eMemZero(m_silence);
	}

	virtual eU32 operator () () override
	{
		while (!s_tfdx.joinRequest)
		{
			_processSynth();
		}

		return 0;
	}

	void _processSynth()
	{
		if (eTfDxNeedMore())
		{
			const eU8 *data = nullptr;

			for (eU32 i = 0; i < s_tfdx.players.size(); i++)
			{
				eTfPlayer &player = *s_tfdx.players[i];
				eTfPlayerProcess(player, &data);
			}

			eTfDxFill((data ? data : m_silence), sizeof(m_silence));
		}
		else
			Sleep(0);
	}

private:
	eU8 m_silence[TF_FRAMESIZE * 2 * sizeof(eS16)];
};

eBool eTfDxInit(eU32 sampleRate)
{
	if (FAILED(DirectSoundCreate8(NULL, &s_tfdx.dsound, NULL)))
		return eFALSE;
	if (FAILED(s_tfdx.dsound->SetCooperativeLevel(GetForegroundWindow(), DSSCL_NORMAL)))
		return eFALSE;

	s_tfdx.bufferSize = sampleRate * 4;
	s_tfdx.sampleRate = sampleRate;

	PCMWAVEFORMAT pcmwf;
	eMemZero(pcmwf);
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wf.nSamplesPerSec = sampleRate;
	pcmwf.wf.nBlockAlign = 4;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec*pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample = 16;

	DSBUFFERDESC dsbDesc;
	eMemZero(dsbDesc);
	dsbDesc.dwSize = sizeof(dsbDesc);
	dsbDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	dsbDesc.dwBufferBytes = s_tfdx.bufferSize;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	if (FAILED(s_tfdx.dsound->CreateSoundBuffer(&dsbDesc, (IDirectSoundBuffer **)&s_tfdx.dsoundBuffer, nullptr)))
		return eFALSE;

	s_tfdx.joinRequest = eFALSE;
	s_tfdx.thread = new eTfDxThread();
	s_tfdx.nextWriteOffset = s_tfdx.bufferSize / 2;

	if (FAILED(s_tfdx.dsoundBuffer->SetCurrentPosition(0)))
		return eFALSE;

	if (FAILED(s_tfdx.dsoundBuffer->Play(0, 0, DSBPLAY_LOOPING)))
		return eFALSE;

	return eTRUE;
}

void eTfDxShutdown()
{
	if (s_tfdx.dsoundBuffer)
	{
		const HRESULT res = s_tfdx.dsoundBuffer->Stop();
		eASSERT(!FAILED(res));
	}

	if (s_tfdx.thread)
	{
		s_tfdx.joinRequest = eTRUE;
		s_tfdx.thread->Join();
		eDelete(s_tfdx.thread);
	}

	eReleaseCom(s_tfdx.dsoundBuffer);
	eReleaseCom(s_tfdx.dsound);
}

void eTfDxAddPlayer(eTfPlayer &player)
{
	eTfPlayerSetSampleRate(player, s_tfdx.sampleRate);
	s_tfdx.players.append(&player);
}

void eTfDxRemovePlayer(eTfPlayer &player)
{
	eInt index = s_tfdx.players.find(&player);
	if (index >= 0)
		s_tfdx.players.removeAt(index);
}

void eTfDxFill(const eU8 *data, eU32 count)
{
	if (!s_tfdx.dsoundBuffer)
		return;

	ePtr write0, write1;
	DWORD length0, length1;
	HRESULT hr = s_tfdx.dsoundBuffer->Lock(s_tfdx.nextWriteOffset, count, &write0, &length0, &write1, &length1, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		hr = s_tfdx.dsoundBuffer->Restore();
		eASSERT(!FAILED(hr));
		hr = s_tfdx.dsoundBuffer->Lock(0, count, &write0, &length0, &write1, &length1, 0);
	}

	eASSERT(!FAILED(hr));
	eMemCopy(write0, data, length0);

	if (write1)
		eMemCopy(write1, data + length0, length1);

	hr = s_tfdx.dsoundBuffer->Unlock(write0, length0, write1, length1);
	eASSERT(!FAILED(hr));
	s_tfdx.nextWriteOffset += count;
	s_tfdx.nextWriteOffset %= s_tfdx.bufferSize;
}

eBool eTfDxNeedMore()
{
	if (!s_tfdx.dsoundBuffer)
		return eFALSE;

	DWORD playCursor, writeCursor;
	s_tfdx.dsoundBuffer->GetCurrentPosition(&playCursor, &writeCursor);

	eS32 writeOffset = s_tfdx.nextWriteOffset;
	const eS32 minDistance = s_tfdx.bufferSize / 2;
	const eS32 maxDistance = minDistance + 4 * TF_FRAMESIZE;
	eS32 distance = writeOffset - playCursor;

	if (distance < 0)
		distance += s_tfdx.bufferSize;

	const eBool needMore = distance < minDistance || distance > maxDistance;

	return needMore;
}