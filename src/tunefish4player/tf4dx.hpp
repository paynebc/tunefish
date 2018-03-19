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

#ifndef TF4DX_HPP
#define TF4DX_HPP

#include "tf4player.hpp"

eBool eTfDxInit(eU32 sampleRate);
void  eTfDxShutdown();
void  eTfDxAddPlayer(eTfPlayer &player);
void  eTfDxRemovePlayer(eTfPlayer &player);
void  eTfDxFill(const eS16 *data, eU32 count);
eBool eTfDxNeedMore();

#endif