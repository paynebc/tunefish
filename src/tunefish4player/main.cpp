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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdio.h>

#include "tf4player.hpp"
#include "tf4dx.hpp"

#include "../../media/tf4modules/no_remorse.tfm.h"

#if defined(eRELEASE)

#pragma function(memset)
void *memset(void *dest, int c, size_t count)
{
	char *bytes = (char *)dest;
	while (count--)
	{
		*bytes++ = (char)c;
	}
	return dest;
}

#pragma function(memcpy)
void *memcpy(void *dest, const void *src, size_t count)
{
	char *dest8 = (char *)dest;
	const char *src8 = (const char *)src;
	while (count--)
	{
		*dest8++ = *src8++;
	}
	return dest;
}

// call ctors and dtors of static and global variables
typedef void (eCDECL *ePVFV)();

#pragma data_seg(".CRT$XCA")
ePVFV __xc_a[] = { nullptr };
#pragma data_seg(".CRT$XCZ")
ePVFV __xc_z[] = { nullptr };
#pragma data_seg() // reset data segment

static const eU32 eMAX_ATEXITS = 32;
static ePVFV g_atExitList[eMAX_ATEXITS];

static void initTerm(ePVFV *pfbegin, ePVFV *pfend)
{
	while (pfbegin < pfend)
	{
		if (*pfbegin)
			(**pfbegin)();

		pfbegin++;
	}
}

static void initAtExit()
{
	eMemZero(g_atExitList);
}

static void doAtExit()
{
	initTerm(g_atExitList, g_atExitList + eMAX_ATEXITS);
}

eInt eCDECL atexit(ePVFV func)
{
	// get next free entry in atexist list
	eU32 index = 0;
	while (g_atExitList[index++]);
	eASSERT(index < eMAX_ATEXITS);

	// put function pointer to destructor there
	if (index < eMAX_ATEXITS)
	{
		g_atExitList[index] = func;
		return 0;
	}

	return -1;
}

eInt eCDECL _purecall()
{
	eASSERT(eFALSE);
	return 0;
}

// fixed unresolved externals to c-lib symbols
extern "C"
{
	eInt    _fltused = 0;
#define _PAGESIZE_ 0x1000

	eNAKED void eCDECL _chkstk(void)
	{
		__asm
		{
			push    ecx
			lea     ecx, [esp] + 8 - 4
			sub     ecx, eax
			sbb     eax, eax
			not eax
			and     ecx, eax
			mov     eax, esp
			and     eax, ~(_PAGESIZE_ - 1)
			cs10:       cmp     ecx, eax
						jb      short cs20
						mov     eax, ecx
						pop     ecx
						xchg    esp, eax
						mov     eax, dword ptr[eax]
						mov     dword ptr[esp], eax
						ret
						cs20 :
			sub     eax, _PAGESIZE_
				test    dword ptr[eax], eax
				jmp     short cs10
				ret
		}
	}

	eNAKED void eCDECL _alloca_probe_16()
	{
		__asm
		{
			push    ecx
			lea     ecx, [esp + 8]
			sub     ecx, eax
			and     ecx, 15
			add     eax, ecx
			sbb     ecx, ecx
			or eax, ecx
			pop     ecx
			jmp    _chkstk
		}
	}
};
#endif

void eGlobalsStaticsInit()
{
#ifdef eRELEASE
	initAtExit();
	initTerm(__xc_a, __xc_z);
#endif
}

void eGlobalsStaticsFree()
{
#ifdef eRELEASE
	doAtExit();
#endif
}

#ifdef eDEBUG
eInt WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, eChar *cmdLine, eInt showCmd)
#else
void WinMainCRTStartup()
#endif
{
	eSimdSetArithmeticFlags(eSAF_RTN | eSAF_FTZ);
	eGlobalsStaticsInit();

	eTfPlayer player;
	const eU32 sampleRate = 44100;

	eTfPlayerInit(player);
	eTfPlayerSetSampleRate(player, sampleRate);
	eTfPlayerLoadSong(player, song, sizeof(song), 0.0f);
	player.volume = 0.4f;

  /* THE FOLLOWING IS FOR BACKWARD PLAYBACK!
	// Calculate Buffer with everything except base and snare
	// --------------------------------------------------------------------------------------
	eTfPlayerMuteInstrument(player, 0, true);  // basedrum
	eTfPlayerMuteInstrument(player, 1, true);  // snare
	eTfPlayerMuteInstrument(player, 12, true); // bass1
	eTfPlayerMuteInstrument(player, 14, true);  // bass2
	eS16 *songBuffer = nullptr;
	const eF32 songLength = eTfPlayerGetSongLength(player);
	eU32 songSamples = eTfPlayerRecordToBuffer(player, songLength, &songBuffer);
	eTfPlayerReverseBuffer(songBuffer, songSamples);

	FILE *fp1 = fopen("c:\\dev\\no_remorse_backward.raw", "wb");
	fwrite(songBuffer, sizeof(eS16), songSamples, fp1);
	fclose(fp1);

	// Calculate buffer with just base and snare
	// --------------------------------------------------------------------------------------
	eTfPlayerReverseMutes(player);
	eTfPlayerReverseAllEvents(player);
	eS16 *songBufferDrums = nullptr;
	eU32 songSamplesDrums = eTfPlayerRecordToBuffer(player, songLength, &songBufferDrums);
	eASSERT(songSamplesDrums == songSamples);

	FILE *fp2 = fopen("c:\\dev\\no_remorse_forward.raw", "wb");
	fwrite(songBufferDrums, sizeof(eS16), songSamples, fp2);
	fclose(fp2);

	// Create a mix of both buffers
	// --------------------------------------------------------------------------------------
	eTfSignalMix16(songBuffer, songBufferDrums, songSamples);
	eTfPlayerSetPlaybackBuffer(player, songBuffer, songSamples);

	// Write to disk
	// --------------------------------------------------------------------------------------
	FILE *fp = fopen("c:\\dev\\no_remorse.raw", "wb");
	fwrite(songBuffer, sizeof(eS16), songSamples, fp);
	fclose(fp);
  */

	// playback
	// --------------------------------------------------------------------------------------
	eTfDxInit(sampleRate);
	eTfDxAddPlayer(player);
	eTfPlayerStart(player, 0.0f);

	MessageBox(nullptr, "Playing: No remorse\n\nVisit www.tunefish-synth.com", "Tunefish4", 0);

	eTfPlayerStop(player);
	eTfDxRemovePlayer(player);
	eTfDxShutdown();

	//delete[] songBuffer;

	eGlobalsStaticsFree();
	ExitProcess(0);
#ifdef eDEBUG
	return 0;
#endif
}
