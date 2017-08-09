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

#include "threading.hpp"

eThread::eThread(eInt flags, eThreadFunc threadFunc) :
m_prio((eThreadPriority)(flags&(~eTHCF_SUSPENDED))),
m_threadFunc(threadFunc)
{
  const eU32 tf = (flags&eTHCF_SUSPENDED ? CREATE_SUSPENDED : 0);
  m_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_threadTrunk, this, tf, (LPDWORD)&m_tid);
  eASSERT(m_handle);

#ifdef eEDITOR
  m_ctx.thread = this;
  m_ctx.tid = m_tid;
#endif

  setPriority(m_prio);
}

eThread::~eThread()
{
  join();
}

void eThread::sleep(eU32 ms)
{
  Sleep(ms);
}

void eThread::join()
{
  if (m_handle)
  {
    WaitForSingleObject((HANDLE)m_handle, INFINITE);
    CloseHandle((HANDLE)m_handle);
    m_handle = nullptr;
  }
}

void eThread::yield()
{
  sleep(1);
}

void eThread::resume()
{
  ResumeThread(m_handle);
}

void eThread::suspend()
{
  SuspendThread(m_handle);
}

void eThread::terminate(eU32 exitCode)
{
  TerminateThread(m_handle, exitCode);
}

void eThread::setPriority(eThreadPriority prio)
{
  int tp[] =
  {
    THREAD_PRIORITY_LOWEST,        // low
    THREAD_PRIORITY_NORMAL,        // normal
    THREAD_PRIORITY_TIME_CRITICAL, // high
  };

  const eU32 index = (prio == eTHP_LOW ? 0 : (prio == eTHP_NORMAL ? 1 : 2));
  SetThreadPriority(m_handle, tp[index]);
  m_prio = prio;
}

eU32 eThread::getId() const
{
  return m_tid;
}

eThreadPriority eThread::getPriority() const
{
  return m_prio;
}

#ifdef eEDITOR
static eThreadCtx g_mainThreadCtx;
static eTHREADLOCAL eThreadCtx *g_curThreadCtx = nullptr;

eThreadCtx & eThread::getThisContext()
{
  return (g_curThreadCtx ? *g_curThreadCtx : g_mainThreadCtx);
}

eThreadCtx & eThread::getContext()
{
  return m_ctx;
}

const eThreadCtx & eThread::getContext() const
{
  return m_ctx;
}
#endif

eU32 eThread::operator () ()
{
  // either a callback func must be specified
  // or this function must be overloaded
  //    eASSERT(eFALSE);
  return 0;
}

eU32 eThread::_threadTrunk(ePtr arg)
{
  eThread *thread = (eThread *)arg;
#ifdef eEDITOR
  g_curThreadCtx = &thread->m_ctx;
#endif
  return (*thread)();
}

eMutex::eMutex() :
m_locked(eFALSE)
{
  CRITICAL_SECTION *cs = new CRITICAL_SECTION;
  InitializeCriticalSection(cs);
  m_handle = (ePtr)cs;
}

eMutex::~eMutex()
{
  eASSERT(!m_locked);
  CRITICAL_SECTION *cs = (CRITICAL_SECTION *)m_handle;
  DeleteCriticalSection(cs);
  eDelete(cs);
}

void eMutex::enter()
{
  EnterCriticalSection((CRITICAL_SECTION *)m_handle);
  eASSERT(!m_locked);
  m_locked = eTRUE;
}

void eMutex::tryEnter()
{
  if (TryEnterCriticalSection((CRITICAL_SECTION *)m_handle))
    m_locked = eTRUE;
}

void eMutex::leave()
{
  eASSERT(m_locked);
  m_locked = eFALSE;
  LeaveCriticalSection((CRITICAL_SECTION *)m_handle);
}

eBool eMutex::isLocked() const
{
  return m_locked;
}