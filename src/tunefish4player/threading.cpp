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

#ifdef eENIGMA
#include "system.hpp"
#else
#include "../tunefish4/Source/runtime/system.hpp"
#include "threading.hpp"
#endif

eThread::eThread(eThreadFunc threadFunc) :
    Handle(nullptr),
    ThreadFunc(threadFunc)
{
}

eThread::~eThread()
{
    Join();
}

void eThread::Sleep(eU32 ms)
{
    ::Sleep(ms);
}

void eThread::Start(eInt flags)
{
    eASSERT(!Handle);
    const eU32 tf = (flags&eTHCF_SUSPENDED ? CREATE_SUSPENDED : 0);
    Handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadTrunk, this, tf, (LPDWORD)&Tid);
    eASSERT(Handle);
    SetPriority((eThreadPriority)(flags&(~eTHCF_SUSPENDED)));

#ifdef eEDITOR
    Ctx.Thread = this;
    Ctx.Tid = Tid;
#endif
}

void eThread::Join()
{
    if (Handle)
    {
        WaitForSingleObject((HANDLE)Handle, INFINITE);
        CloseHandle((HANDLE)Handle);
        Handle = nullptr;
    }
}

void eThread::Resume()
{
    ResumeThread(Handle);
}

void eThread::Suspend()
{
    SuspendThread(Handle);
}

void eThread::Terminate(eU32 exitCode)
{
    TerminateThread(Handle, exitCode);
}

void eThread::SetPriority(eThreadPriority prio)
{
    const eInt tp[] =
    {
        THREAD_PRIORITY_LOWEST,        // low
        THREAD_PRIORITY_NORMAL,        // normal
        THREAD_PRIORITY_TIME_CRITICAL, // high
    };

    const eU32 index = (prio == eTHP_LOW ? 0 : (prio == eTHP_NORMAL ? 1 : 2));
    SetThreadPriority(Handle, tp[index]);
    Prio = prio;
}

#ifdef eEDITOR
static eThreadCtx MainThreadCtx;
static eTHREADLOCAL eThreadCtx *CurThreadCtx = nullptr;

eThreadCtx & eThread::GetThisContext()
{
    return (CurThreadCtx ? *CurThreadCtx : MainThreadCtx);
}
#endif

eU32 eThread::operator () ()
{
    // either a callback func must be specified
    // or this function must be overloaded
    eASSERT(eFALSE);
    return 0;
}

eU32 eThread::ThreadTrunk(ePtr arg)
{
    eThread *thread = (eThread *)arg;
#ifdef eEDITOR
    CurThreadCtx = &thread->Ctx;
#endif
    return (*thread)();
}

eMutex::eMutex() :
    IsLocked(eFALSE)
{
    CRITICAL_SECTION *cs = new CRITICAL_SECTION;
    InitializeCriticalSection(cs);
    Handle = (ePtr)cs;
}

eMutex::~eMutex()
{
    eASSERT(!IsLocked);
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *)Handle;
    DeleteCriticalSection(cs);
    eDelete(cs);
}

void eMutex::Enter()
{
    EnterCriticalSection((CRITICAL_SECTION *)Handle);
    eASSERT(!IsLocked);
    IsLocked = eTRUE;
}

void eMutex::TryEnter()
{
    if (TryEnterCriticalSection((CRITICAL_SECTION *)Handle))
        IsLocked = eTRUE;
}

void eMutex::Leave()
{
    eASSERT(IsLocked);
    IsLocked = eFALSE;
    LeaveCriticalSection((CRITICAL_SECTION *)Handle);
}

eSemaphore::eSemaphore(eU32 InCount, eU32 InMaxCount) :
    MaxCount(InMaxCount)
{
    SemaphoreHandle = CreateSemaphore(nullptr, InCount, InMaxCount, nullptr);
}
eSemaphore::~eSemaphore() 
{
    CloseHandle(SemaphoreHandle);
}

void eSemaphore::Wait(eU32 Count)
{
    // TODO: no clue how to wait for multiple semaphore handles at once with windows semaphores, so we do it one by one
    for (eU32 Idx = 0; Idx < Count; Idx++)
        WaitForSingleObject(SemaphoreHandle, INFINITE);
}

void eSemaphore::Signal(eU32 Count)
{
    ReleaseSemaphore(SemaphoreHandle, Count, nullptr);
}
