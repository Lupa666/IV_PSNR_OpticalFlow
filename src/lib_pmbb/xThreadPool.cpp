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

#include "xThreadPool.h"

using namespace std::chrono_literals;

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xThreadPool::create(int32 NumThreads, int32 WaitingQueueSize)
{
  assert(NumThreads      >0);
  assert(WaitingQueueSize>0);

  m_NumThreads = NumThreads;
  m_WaitingTasks.setSize(WaitingQueueSize);

  for(int32 i=0; i<m_NumThreads; i++)
  {
    std::packaged_task<uint32(xThreadPool*)> PackagedTask(xThreadStarter);
    m_Future.push_back(PackagedTask.get_future());
    std::thread Thread = std::thread(std::move(PackagedTask), this);
    m_ThreadId.push_back(Thread.get_id());
    m_Thread  .push_back(std::move(Thread));      
  } 

  m_Event.set();
}
void xThreadPool::destroy()
{
  bool AnyActive = false;

  assert(isWaitingQueueEmpty());

  for(int32 i=0; i<m_NumThreads; i++)
  {
    xWorkerTask* Terminator = new xWorkerTaskTerminator;
    m_WaitingTasks.EnqueueWait(Terminator);
  }

  for(int32 i=0; i<m_NumThreads; i++)
  {   
    std::future_status Status = m_Future[i].wait_for(500ms);
    if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join(); }
    else                                                              { AnyActive = true;   }
  }

  if(AnyActive)
  {
    for(int32 i=0; i<m_NumThreads; i++)
    {
      std::future_status Status = m_Future[i].wait_for(5s);
      if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join();    }
      else                                                              { m_Thread[i].~thread(); }
    }
  }

  for(std::pair<const uintPtr, xQueue<xWorkerTask*>>& Pair : m_CompletedTasks)
  {
    xQueue<xWorkerTask*>& CompletedTaskQueue = Pair.second;
    int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
    for(int32 i=0; i<NumCompleted; i++)
    {
      xWorkerTask* Task;
      CompletedTaskQueue.DequeueWait(Task);
      delete Task;
    }
  }  
}
bool xThreadPool::registerClient(uintPtr ClientId, int32 CompletedQueueSize)
{
  if(m_CompletedTasks.find(ClientId) != m_CompletedTasks.end()) { return false; }

  m_CompletedTasks.emplace(ClientId, CompletedQueueSize);
  return true;
}
bool xThreadPool::unregisterClient(uintPtr ClientId)
{
  if(m_CompletedTasks.find(ClientId) == m_CompletedTasks.end()) { return false; }

  xQueue<xWorkerTask*>& CompletedTaskQueue = m_CompletedTasks.at(ClientId);
  int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
  for(int32 i=0; i<NumCompleted; i++)
  {
    xWorkerTask* Task;
    CompletedTaskQueue.DequeueWait(Task);
    delete Task;
  }

  m_CompletedTasks.erase(ClientId);
  return true;
}
uint32 xThreadPool::xThreadFunc() 
{
  m_Event.wait();
  std::thread::id ThreadId = std::this_thread::get_id();
  int32 ThreadIdx = (int32)(std::find(m_ThreadId.begin(), m_ThreadId.end(), ThreadId) - m_ThreadId.begin());
  while(1)
  {    
    xWorkerTask* Task;
    m_WaitingTasks.DequeueWait(Task);
    if(Task->getStatus() == eTaskStatus::Terminate) 
    {
      delete Task; break;
    }
    xWorkerTask::StarterFunction(Task, ThreadIdx);
    m_CompletedTasks.at(Task->getClientId()).EnqueueWait(Task);
  }
  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================

void xThreadPool::xWorkerTask::StarterFunction(xWorkerTask* WorkerTask, int32 ThreadIdx)
{
  assert(WorkerTask->m_Status == eTaskStatus::Waiting);
  WorkerTask->m_Status = eTaskStatus::Processed;
  WorkerTask->WorkingFunction(ThreadIdx);
  WorkerTask->m_Status = eTaskStatus::Completed;
}

//===============================================================================================================================================================================================================

void xThreadPoolInterface::init(xThreadPool* ThreadPool, int32 CompletedQueueSize)
{
  m_ThreadPool = ThreadPool;
  m_ThreadPool->registerClient(getClientId(), CompletedQueueSize);
  m_NumChunks  = m_ThreadPool->getNumThreads();
}
void xThreadPoolInterface::uininit()
{
  if(m_ThreadPool == nullptr) { return; }
  m_ThreadPool->unregisterClient(getClientId());
  m_ThreadPool = nullptr;
}
void xThreadPoolInterface::addWaitingTask(xThreadPool::xWorkerTask* Task)
{ 
  Task->setClientId(getClientId());
  Task->setPriority(m_Priority);
  m_ThreadPool->addWaitingTask(Task);
}
void xThreadPoolInterface::addWaitingTask(std::function<void(int32)> Function)
{ 
  xThreadPool::xWorkerTaskFunction* Task = new xThreadPool::xWorkerTaskFunction(getClientId(), m_Priority, Function);
  m_ThreadPool->addWaitingTask(Task);
}
void xThreadPoolInterface::waitUntilTasksFinished(int32 NumTasksToWaitFor)
{
  for(int32 TaskId=0; TaskId < NumTasksToWaitFor; TaskId++)
  {
    xWorkerTask* Task = receiveCompletedTask();
    delete Task;
  }
}
void xThreadPoolInterface::executeTask(std::function<void(int32)> Function)
{
  addWaitingTask(Function);
  waitUntilTasksFinished(1);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
