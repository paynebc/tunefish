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

#ifndef RANDOM_HPP
#define RANDOM_HPP

class eRandom
{
public:
    eRandom();
    eRandom(eU32 initSeed);

    void    Seed(eU32 newSeed);
    void    SeedRandomly();

    eU32    NextInt();
    eInt    NextInt(eInt min, eInt max);
    eF32    NextFloat();
    eF32    NextFloat(eF32 min, eF32 max);
    eF32    NextFloatNormalDistribution(eF32 Mean, eF32 StdDeviation);

    eU32    Max() const;

private:
    eU32    m_seed;
};

#endif
