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

#ifndef TF_SYNTHPROGRAM_HPP
#define TF_SYNTHPROGRAM_HPP

const eU32 TF_MIN_PARAM_CAPACITY = 256;
const eU32 TF_MAX_PROGRAMNAME_LEN = 64;

class eTfSynthProgram
{
public:
    eTfSynthProgram();

	void            loadDefault(int i);
    void            setParam(eU32 index, eF32 value);
    eF32            getParam(eU32 index);
    eChar *         getName();
    void            setName(const char *newname);

private:
    void            _ensureSize(eU32 size);

	eArray<eF32>    params;
	eChar           name[TF_MAX_PROGRAMNAME_LEN];
};

#endif
