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

#ifndef THREADING_HPP
#define THREADING_HPP

#include "../tunefish4/Source/runtime/system.hpp"

// callback function for threads (if you prefer
// the c-style and don't want to derive)
typedef eU32(*eThreadFunc)(ePtr arg);

enum eThreadCreateFlag
{
  eTHCF_SUSPENDED = 1,
};

enum eThreadPriority
{
  eTHP_LOW = 2,
  eTHP_NORMAL = 4,
  eTHP_HIGH = 8,
};

// either derive from eThread and overwrite the
// function call operator (), or provide a thread
// callback function in the constructor.
class eThread
{
public:
  eThread(eInt flags = eTHP_NORMAL, eThreadFunc threadFunc = nullptr);
  virtual ~eThread();

  static void         sleep(eU32 ms);
  void                join();
  void                yield();
  void                resume();
  void                suspend();
  void                terminate(eU32 exitCode = 0);
  void                setPriority(eThreadPriority prio);

#ifdef eEDITOR
  static eThreadCtx & getThisContext();
  eThreadCtx &        getContext();
  const eThreadCtx &  getContext() const;
#endif
  eU32                getId() const;
  eThreadPriority     getPriority() const;

  virtual eU32        operator () ();

private:
  static eU32         _threadTrunk(ePtr arg);

private:
#ifdef eEDITOR
  eThreadCtx          m_ctx;
#endif
  ePtr                m_handle;
  eThreadFunc         m_threadFunc;
  eThreadPriority     m_prio;
  eU32                m_tid;
};

class eMutex
{
public:
  eMutex();
  ~eMutex();

  void                enter();
  void                tryEnter();
  void                leave();

  eBool               isLocked() const;

private:
  eMutex(const eMutex &mtx) = delete;
  eMutex & operator = (const eMutex &mtx) = delete;

private:
  ePtr                m_handle;
  eBool               m_locked;
};

class eScopedLock
{
public:
  eScopedLock(eMutex &mutex) :
    m_mutex(mutex)
  {
    m_mutex.enter();
  }

  ~eScopedLock()
  {
    m_mutex.leave();
  }

private:
  eMutex & m_mutex;
};

#endif