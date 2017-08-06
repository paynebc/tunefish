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

#include "../JuceLibraryCode/JuceHeader.h"
#include "synth/tf4.hpp"

class eTfSynthProgram
{
public:
    eTfSynthProgram();
    eTfSynthProgram(const eTfSynthProgram& copy) noexcept; 
    eTfSynthProgram& operator=(const eTfSynthProgram& copy); 

	void            loadFactory(int i);
    void            setParam(eU32 index, eF32 value);
    eF32            getParam(eU32 index) const;
    String          getName() const;
    void            setName(String newname);
    void            loadFromSynth(eTfInstrument *tf);
    void            applyToSynth(eTfInstrument *tf) const;

private:
	eF32            params[TF_PARAM_COUNT];
	String          name;
};

#endif
