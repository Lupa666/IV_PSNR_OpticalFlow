#pragma once
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
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xEvent - thread safe binary event
//===============================================================================================================================================================================================================
class xEvent
{
protected:
  bool                    m_State;
  bool                    m_ManualReset;
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
  xEvent(bool ManualReset, bool InitialState) : m_State(InitialState), m_ManualReset(ManualReset) {}
  xEvent(const xEvent&) = delete;
  xEvent& operator=(const xEvent&) = delete;

  inline void set     ();
  inline void reset   ();
  inline void wait    ();
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xEvent::set()
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  m_State = true;
  m_ConditionVariable.notify_all();
}  
void xEvent::reset()
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  m_State = false;
}
void xEvent::wait()
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_State == false){ m_ConditionVariable.wait(LockManager, [&]{ return m_State;}); }
  if(!m_ManualReset) { m_State = false; }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
