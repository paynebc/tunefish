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

// callback function for threads (if you prefer
// the c-style and don't want to derive)
typedef eU32 (* eThreadFunc)(ePtr arg);

enum eThreadCreateFlag
{
    eTHCF_SUSPENDED     = 1,
};

enum eThreadPriority
{
    eTHP_LOW            = 2,
    eTHP_NORMAL         = 4,
    eTHP_HIGH           = 8,
};

#ifdef eEDITOR
struct eThreadCtx
{
    eU32                Tid;
    class eThread *     Thread;
};
#endif

// either derive from eThread and overwrite the
// function call operator (), or provide a thread
// callback function in the constructor.
class eThread
{
public:
    eThread(eThreadFunc threadFunc=nullptr);
    virtual ~eThread();

    void                Start(eInt flags=eTHP_NORMAL);
    void                Join();
    void                Resume();
    void                Suspend();
    void                Terminate(eU32 exitCode=0);
    void                SetPriority(eThreadPriority prio);

    static void         Sleep(eU32 ms);
#ifdef eEDITOR
    static eThreadCtx & GetThisContext();
#endif

    virtual eU32        operator () ();

private:
    static eU32         ThreadTrunk(ePtr arg);

public:
#ifdef eEDITOR
    eThreadCtx          Ctx;
#endif
    eThreadPriority     Prio;
    eU32                Tid;

private:
    ePtr                Handle;
    eThreadFunc         ThreadFunc;
};

class eMutex
{
public:
    eMutex();
    eMutex(const eMutex &mtx) = delete;
    eMutex & operator = (const eMutex &mtx) = delete;
    ~eMutex();

    void                Enter();
    void                TryEnter();
    void                Leave();

public:
    eBool               IsLocked;

private:
    ePtr                Handle;
};

class eScopedLock
{
public:
    eScopedLock(eMutex &mutex) : m_mutex(mutex)
    {
        m_mutex.Enter();
    }

    ~eScopedLock()
    {
        m_mutex.Leave();
    }

private:
    eMutex & m_mutex;
};

class eSemaphore
{
public:
    eSemaphore(eU32 InCount, eU32 InMaxCount);
    ~eSemaphore();

    void Wait(eU32 Count = 1);
    void Signal(eU32 Count = 1);

    const   eU32    MaxCount;
private:
    void*  SemaphoreHandle;
};

#endif