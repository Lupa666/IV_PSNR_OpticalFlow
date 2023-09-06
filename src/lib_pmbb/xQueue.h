﻿#pragma once
/* ############################################################################
The copyright in this software is being made available under the 3-clause BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.

Author(s):
  * Jakub Stankowski, jakub.stankowski@put.poznan.pl,
    Poznan University of Technology, Poznań, Poland


Copyright (c) 2010-2021, Poznan University of Technology. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
############################################################################ */


#include "xCommonDefPMBB.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xQueue - thread safe std::queue (FIFO) for any type of data
//===============================================================================================================================================================================================================
template <class XXX> class xQueue
{
protected:
  std::queue<XXX> m_Queue;
  uint32          m_QueueSize;

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_EnqueueConditionVariable;
  std::condition_variable m_DequeueConditionVariable;
  
public:
  xQueue(int32 QueueSize = 1) { setSize(QueueSize); }

  int32    getSize  (          ) const { return m_QueueSize; }
  void     setSize  (int32 Size)       { assert(Size>0); std::lock_guard<std::mutex> LockManager(m_Mutex); m_QueueSize = Size; m_EnqueueConditionVariable.notify_all(); }
  bool     isEmpty  (          ) const { return m_Queue.empty(); }
  bool     isFull   (          ) const { return (m_Queue.size() == m_QueueSize); }
  uintSize getLoad  (          ) const { return m_Queue.size(); }

  void EnqueueResize(XXX  Data);
  bool DequeueTry   (XXX& Data);

  void EnqueueWait  (XXX  Data);
  void DequeueWait  (XXX& Data);

  template<class Rep, class Period> bool EnqueueWaitFor(XXX  Data, const std::chrono::duration<Rep, Period>& Duration);
  template<class Rep, class Period> bool DequeueWaitFor(XXX& Data, const std::chrono::duration<Rep, Period>& Duration);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX> void xQueue<XXX>::EnqueueResize(XXX Data)
{
  m_Mutex.lock();
  if(isFull()) { m_QueueSize++; }
  m_Queue.push(Data);
  m_Mutex.unlock();
  m_DequeueConditionVariable.notify_one();
}
template<class XXX> bool xQueue<XXX>::DequeueTry(XXX& Data)
{
  m_Mutex.lock(); 
  if(!m_Queue.empty())
  {
    Data = m_Queue.front();
    m_Queue.pop();
    m_Mutex.unlock();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else
  {
    m_Mutex.unlock();
    return false;
  }
}
template<class XXX> void xQueue<XXX>::EnqueueWait(XXX Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size()>=m_QueueSize) { m_EnqueueConditionVariable.wait(LockManager, [&]{ return m_Queue.size()<m_QueueSize; } ); }
  m_Queue.push(Data);
  LockManager.unlock();
  m_DequeueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}
template<class XXX> void xQueue<XXX>::DequeueWait(XXX& Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.empty()) { m_DequeueConditionVariable.wait(LockManager, [&]{ return !m_Queue.empty(); } ); }
  Data = m_Queue.front();
  m_Queue.pop();
  LockManager.unlock();
  m_EnqueueConditionVariable.notify_one();
}
template<class XXX> template<class Rep, class Period> bool xQueue<XXX>::EnqueueWaitFor(XXX Data, const std::chrono::duration<Rep, Period>& Duration)
{
  if(Duration == std::chrono::duration<Rep, Period>::max()) { EnqueueWait(Data); return true; } //fix for some broken implementations (i.e. MSVC 14)

  std::unique_lock<std::mutex> LockManager(m_Mutex);
  std::cv_status Status = std::cv_status::no_timeout;
  if(m_Queue.size()>=m_QueueSize) { Status = m_EnqueueConditionVariable.wait_for(LockManager, Duration); }
  if(Status==std::cv_status::no_timeout && m_Queue.size()<m_QueueSize)
  {
    m_Queue.push(Data);
    LockManager.unlock();
    m_DequeueConditionVariable.notify_one();
    return true;
  }
  else { return false; }    
  //release lock - std::unique_lock destructor... 
}
template<class XXX> template<class Rep, class Period> bool xQueue<XXX>::DequeueWaitFor(XXX& Data, const std::chrono::duration<Rep, Period>& Duration)
{
  if(Duration == std::chrono::duration<Rep, Period>::max()) { DequeueWait(Data); return true; } //fir for some broken implementations (i.e. MSVC 14)

  std::unique_lock<std::mutex> LockManager(m_Mutex);
  std::cv_status Status = std::cv_status::no_timeout;
  if(m_Queue.empty()) { Status = m_DequeueConditionVariable.wait_for(LockManager, Duration); }
  if(Status==std::cv_status::no_timeout && !m_Queue.empty())
  {
    Data = m_Queue.front();
    m_Queue.pop();
    LockManager.unlock();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else { return false; }
  //release lock - std::unique_lock destructor... 
}

//===============================================================================================================================================================================================================
//xPriorityQueue - thread safe std::priority_queue for any type of data
//===============================================================================================================================================================================================================
template<class XXX> class xPriorityQueue
{
  using BaseType = typename std::remove_pointer<XXX>::type::Comparator;
protected:
  std::priority_queue<XXX, std::vector<XXX>, BaseType> m_Queue;
  uint32                   m_QueueSize;

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_EnqueueConditionVariable;
  std::condition_variable m_DequeueConditionVariable;

public:
  xPriorityQueue(int32 QueueSize = 1) { setSize(QueueSize); }

  int32    getSize  (          ) { return m_QueueSize; }
  void     setSize  (int32 Size) { assert(Size>0); std::lock_guard<std::mutex> LockManager(m_Mutex); m_QueueSize = Size; m_EnqueueConditionVariable.notify_all(); }
  bool     isEmpty  (          ) { return m_Queue.empty(); }
  bool     isFull   (          ) { return (m_Queue.size() == m_QueueSize); }
  uintSize getLoad  (          ) { return m_Queue.size(); }

  void EnqueueResize(XXX  Data);
  bool DequeueTry   (XXX& Data);

  void EnqueueWait  (XXX  Data);
  void DequeueWait  (XXX& Data);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX> void xPriorityQueue<XXX>::EnqueueResize(XXX Data)
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  if(isFull()) { m_QueueSize++; }
  m_Queue.push(Data);
  m_DequeueConditionVariable.notify_one();
  //release lock - std::lock_guard destructor...
}
template<class XXX> bool xPriorityQueue<XXX>::DequeueTry(XXX& Data)
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  if(!m_Queue.empty())
  {
    Data = m_Queue.top();
    m_Queue.pop();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else { return false; }
  //release lock - std::lock_guard destructor...
}
template<class XXX> void xPriorityQueue<XXX>::EnqueueWait(XXX Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size()>=m_QueueSize) { m_EnqueueConditionVariable.wait(LockManager, [&]{ return m_Queue.size()<m_QueueSize;} ); }
  m_Queue.push(Data);
  m_DequeueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}
template<class XXX> void xPriorityQueue<XXX>::DequeueWait(XXX& Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.empty()) { m_DequeueConditionVariable.wait(LockManager, [&]{ return !m_Queue.empty(); } ); }
  Data = m_Queue.top();
  m_Queue.pop();
  m_EnqueueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
