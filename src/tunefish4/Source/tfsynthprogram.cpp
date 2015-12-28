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

#include "runtime/system.hpp"
#include "tfsynthprogram.hpp"
#include <stdio.h>
#include <string.h>

eTfSynthProgram::eTfSynthProgram()
{
    params.reserve(TF_MIN_PARAM_CAPACITY);
}

void eTfSynthProgram::loadDefault(int i)
{
    sprintf(name, "INIT %i", i);
}

void eTfSynthProgram::setParam(eU32 index, eF32 value)
{
    _ensureSize(index);
    params[index] = value;
}

eF32 eTfSynthProgram::getParam(eU32 index)
{
    _ensureSize(index);
    return params[index];
}

eChar * eTfSynthProgram::getName()
{
    return name;
}

void eTfSynthProgram::setName(const char *newname)
{
    strncpy(name, newname, TF_MAX_PROGRAMNAME_LEN-1);
    name[TF_MAX_PROGRAMNAME_LEN-1] = 0;
}

void eTfSynthProgram::_ensureSize(eU32 size)
{
    if (size >= params.size())
    {
        params.resize(size);
    }
}
