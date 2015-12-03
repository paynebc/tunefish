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

#ifndef ePLAYER

#include <fstream>
#include "system.hpp"

eFile::eFile(const eChar *path) :
    m_path(path),
    m_openMode(eFOM_READ),
    m_changed(eFALSE),
    m_opened(eFALSE),
    m_pos(0)
{
}

eFile::~eFile()
{
    close();
}

eBool eFile::open(eFileOpenMode openMode)
{
    m_pos = 0;
    m_openMode = openMode;

    eASSERT(openMode&eFOM_READ || openMode&eFOM_WRITE);   
    std::fstream::openmode fsom = std::fstream::binary;

    if (openMode&eFOM_READ)
        fsom |= std::fstream::in;
    if (openMode&eFOM_WRITE)
        fsom |= std::fstream::out;

    std::fstream f(m_path, fsom);
    if (!f.is_open())
        return eFALSE;

    // retrieve file size
    f.seekg(0, std::ifstream::end);
    const eU32 size = (eU32)f.tellg();
    f.seekg(0, std::ifstream::beg);

    // buffer file
    if (size > 0)
    {
        m_data.resize(size);
        f.read((eChar *)&m_data[0], m_data.size());
    }

    m_opened = eTRUE;
    return eTRUE;
}

eBool eFile::close()
{
    if (m_opened && m_openMode&eFOM_WRITE && m_changed)
    {
        std::fstream f(m_path, std::fstream::binary|std::fstream::out|std::fstream::trunc);

        if (f.is_open())
            f.write((eChar *)&m_data[0], m_data.size());
        else
            return eFALSE;
    }

    m_opened = eFALSE;
    return eTRUE;
}

void eFile::clear()
{
    eASSERT(m_opened);
    eASSERT(m_openMode&eFOM_WRITE);

    m_data.clear();
    m_pos = 0;
    m_changed = eTRUE;
}

// returns number of read bytes
eU32 eFile::read(ePtr buf, eU32 numBytes)
{
    eASSERT(m_opened);
    eASSERT(m_openMode&eFOM_READ);

    const eU32 readCount = (m_pos+numBytes <= m_data.size() ? numBytes : m_data.size()-m_pos);
    eMemCopy(buf, &m_data[0]+m_pos, readCount);
    m_pos += readCount;
    return readCount;
}

eU32 eFile::readAll(eByteArray &buf)
{
    eASSERT(m_openMode&eFOM_READ);
    buf = m_data;
    return buf.size();
}

void eFile::write(eConstPtr buf, eU32 numBytes)
{
    eASSERT(m_opened);
    eASSERT(m_openMode&eFOM_WRITE);

    if (m_pos+numBytes > m_data.size())
        m_data.resize(m_pos+numBytes);

    eMemCopy(&m_data[m_pos], buf, numBytes);
    m_pos += numBytes;
    m_changed = eTRUE;
}

void eFile::write(const eByteArray &buf)
{
    write(&buf[0], buf.size());
}

// returns new absolute file position
eU32 eFile::seek(eFileSeekMode seekMode, eU32 offset)
{
    eASSERT(m_opened);

    switch (seekMode)
    {
    case eSEEK_BEGIN:
        m_pos = eMin(offset, m_data.size());
        break;
    case eSEEK_CURRENT:
        m_pos = eMin(m_pos+offset, m_data.size());
        break;
    case eSEEK_END:
        m_pos = eMax(0, (eInt)m_data.size()-(eInt)offset);
        break;
    }

    return m_pos;
}

eU32 eFile::tell() const
{
    eASSERT(m_opened);
    return m_pos;
}

eU32 eFile::getSize() const
{
    eASSERT(m_opened);
    return m_data.size();
}

eBool eFile::readAll(const eChar *path, eByteArray &data)
{
    eFile f(path);
    
    if (f.open(eFOM_READ))
    {
        f.readAll(data);
        return eTRUE;
    }

    return eFALSE;
}

#endif