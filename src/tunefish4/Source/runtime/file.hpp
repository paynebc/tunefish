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
 along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#ifndef FILE_HPP
#define FILE_HPP

#ifndef ePLAYER

enum eFileSeekMode
{
    eSEEK_BEGIN,
    eSEEK_CURRENT,
    eSEEK_END
};

enum eFileOpenMode
{
    eFOM_READ      = 0x01,
    eFOM_WRITE     = 0x02,
    eFOM_READWRITE = eFOM_READ|eFOM_WRITE
};

class eFile
{
public:
    eFile(const eChar *path);
    ~eFile();

    eBool               open(eFileOpenMode openMode);
    eBool               close();
    void                clear();
    eU32                read(ePtr buf, eU32 numBytes);
    eU32                readAll(eByteArray &buf);
    void                write(eConstPtr buf, eU32 numBytes);
    void                write(const eByteArray &buf);
    eU32                seek(eFileSeekMode seekMode, eU32 offset);
    eU32                tell() const;
    eU32                getSize() const;
    static eBool        readAll(const eChar *path, eByteArray &data);

private:
    const eChar *       m_path;
    eFileOpenMode       m_openMode;
    eBool               m_changed;
    eBool               m_opened;
    eByteArray          m_data;
    eU32                m_pos;
};

#endif

#endif