/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       ______      _                          ______
 *      / ____/___  (_)___ _____ ___  ____ _   / ____/
 *     / __/ / __ \/ / __ `/ __ `__ \/ __ `/  /___ \
 *    / /___/ / / / / /_/ / / / / / / /_/ /  ____/ /
 *   /_____/_/ /_/_/\__, /_/ /_/ /_/\__,_/  /_____/.
 *                 /____/
 *
 *   Copyright © 2003-2014 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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