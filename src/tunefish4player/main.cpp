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

#include "tf4player.hpp"
#include "tf4dx.hpp"

#include "../../media/tf4modules/tf4testsong.tfm.h"

eInt WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, eChar *cmdLine, eInt showCmd)
{
  eTfPlayer player;

  eTfDxInit(44100);
  eTfPlayerInit(player);
  player.volume = 0.4f;

  eTfPlayerLoadSong(player, song, sizeof(song), 0.0f);
  eTfDxAddPlayer(player);
  eTfPlayerStart(player, 0.0f);

  MessageBox(0, "Playing: TF4 Testsong\n\nVisit www.tunefish-synth.com", "Tunefish4", 0);

  eTfPlayerStop(player);
  eTfDxRemovePlayer(player);
  eTfDxShutdown();
}