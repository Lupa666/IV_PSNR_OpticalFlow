#pragma once

/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2021, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 // Original authors: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
 //                   Adrian Dziembowski, adrian.dziembowski@put.poznan.pl,
 //                   Poznan University of Technology, Pozna�, Poland

#include "xCommonDefIVPSNR.h"
#include "xPlane.h"
#include "xPic.h"
#include "xVec.h"
#include "xThreadPool.h"
#include "xMathUtils.h"
#include <numeric>
#include <vector>
#include <tuple>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPSNR
{
public:
  static constexpr bool  c_UseKBNS = xc_USE_KBNS;
  static constexpr int32 c_NumComponents = 3;
  //results
  using tRes4 = std::tuple<flt64V4, boolV4>;
  using tRes1 = std::tuple<flt64  , bool  >;
  //debug calback types
  using tDCfMSK = std::function<void(int32)>;

protected:

protected:
  int32   m_NumComponents = c_NumComponents;
  tDCfMSK m_DebugCallbackMSK;

  xThreadPoolInterface m_ThreadPoolIf;

public:
  void  setDebugCallbackMSK(tDCfMSK DebugCallbackMSK) { m_DebugCallbackMSK = DebugCallbackMSK; }
  
  void  initThreadPool  (xThreadPool* ThreadPool, int32 Height) { if(ThreadPool) { m_ThreadPoolIf.init(ThreadPool, Height); } }
  void  uninitThreadPool(                                     ) { m_ThreadPoolIf.uininit(); }

  tRes4 calcPicPSNR    (const xPicP* Tst, const xPicP* Ref);
  flt64 calcPicPSNRFlow(const xPlane<flt32V2>* Tst, const xPlane<flt32V2>* Ref);
  tRes4 calcPicPSNRM   (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);

protected:
  tRes1 xCalcCmpPSNR    (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  flt64 xCalcCmpPSNRFlow(const xPlane<flt32V2>* Tst, const xPlane<flt32V2>* Ref);
  tRes1 xCalcCmpPSNRM   (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);

public:
  static inline flt64 Accumulate(std::vector<flt64>& Data)
  {
    if constexpr(c_UseKBNS) { return xMathUtils::KahanBabuskaNeumaierSumation(Data.data(), (int32)Data.size()); }
    else                    { return std::accumulate(Data.begin(), Data.end(), (flt64)0); }
  }
  static flt64 CalcPSNRfromSSD      (flt64 SSD, int32 Area, int32 BitDepth);
  static flt64 CalcPSNRfromMaskedSSD(flt64 SSD, int32 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB