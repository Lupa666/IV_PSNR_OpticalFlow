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
 //                   Poznan University of Technology, Poznań, Poland

#include "xPSNR.h"
#include "xDistortion.h"
#include "xDistortionSTD.h"
#include "xPixelOps.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPSNR
//===============================================================================================================================================================================================================
xPSNR::tRes4 xPSNR::calcPicPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  flt64V4  PSNR  = xMakeVec4(flt64_max );
  boolV4   Exact = xMakeVec4(false     );

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      m_ThreadPoolIf.addWaitingTask([this, &PSNR, &Exact, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/) { std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = xCalcCmpPSNR(Tst, Ref, (eCmp)CmpIdx); });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(m_NumComponents);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = xCalcCmpPSNR(Tst, Ref, (eCmp)CmpIdx);
    }
  }

  return std::make_tuple(PSNR, Exact);
}
flt64 xPSNR::calcPicPSNRFlow(const xPlane<flt32V2>* Tst, const xPlane<flt32V2>* Ref)
{
    assert(Ref != nullptr && Tst != nullptr);
    assert(Ref->isCompatible(Tst));

    flt64 PSNR = 0.0;
    std::vector<bool>   Exact = std::vector<bool>(false);

    if (m_ThreadPoolIf.isActive())
    {
        m_ThreadPoolIf.addWaitingTask([this, &PSNR, &Exact, &Tst, &Ref](int32 /*ThreadIdx*/) { PSNR = xCalcCmpPSNRFlow(Tst, Ref); });
        m_ThreadPoolIf.waitUntilTasksFinished(1);
    }
    else
    {
        PSNR = xCalcCmpPSNRFlow(Tst, Ref);
    }

    return PSNR;
}
xPSNR::tRes4 xPSNR::calcPicPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible    (Tst));
  assert(Ref->isSameSizeMargin(Msk));

  const int32 NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight());

  flt64V4 PSNR  = xMakeVec4(flt64_max);
  boolV4  Exact = xMakeVec4(false    );

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      m_ThreadPoolIf.addWaitingTask([this, &PSNR, &Exact, &Tst, &Ref, &Msk, &NumNonMasked, CmpIdx](int32 /*ThreadIdx*/) { std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = xCalcCmpPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx); });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(m_NumComponents);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = xCalcCmpPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx);
    }
  }

  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  return std::make_tuple(PSNR, Exact);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xPSNR::tRes1 xPSNR::xCalcCmpPSNR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId);
  const uint16* RefPtr    = Ref->getAddr  (CmpId);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();

  uint64 FrameDistortion = 0;
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcSSD(RefPtr, TstPtr, Width);
    FrameDistortion += RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  flt64 PSNR  = CalcPSNRfromSSD((flt64)FrameDistortion, Tst->getArea(), Tst->getBitDepth());
  bool  Exact = FrameDistortion == 0;
  if(Exact)
  {
    flt64 FakePSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth());
    PSNR  = FakePSNR;
  }

  return std::make_tuple(PSNR, Exact);
}
flt64 xPSNR::xCalcCmpPSNRFlow(const xPlane<flt32V2>* Tst, const xPlane<flt32V2>* Ref)
{
    const int32   Width = Ref->getWidth();
    const int32   Height = Ref->getHeight();
    const flt32V2* TstPtr = Tst->getAddr();
    const flt32V2* RefPtr = Ref->getAddr();
    const int32   TstStride = Tst->getStride();
    const int32   RefStride = Ref->getStride();

    uint64 FrameDistortion = 0;
    for (int32 y = 0; y < Height; y++)
    {
        uint64 RowSSD = xDistortionSTD::CalcSSD(RefPtr, TstPtr, Width);
        FrameDistortion += RowSSD;
        TstPtr += TstStride;
        RefPtr += RefStride;
    }

    flt64 PSNR = CalcPSNRfromSSD((flt64)FrameDistortion, Tst->getArea(), Tst->getBitDepth());
    bool  Exact = FrameDistortion == 0;
    if (Exact)
    {
        flt64 FakePSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth());
        PSNR = FakePSNR;
    }

    return PSNR;
}
xPSNR::tRes1 xPSNR::xCalcCmpPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId   );
  const uint16* RefPtr    = Ref->getAddr  (CmpId   );
  const uint16* MskPtr    = Msk->getAddr  (eCmp::LM);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const int32   MskStride = Msk->getStride();

  uint64 FrameDistortion = 0;
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcWeightedSSD(RefPtr, TstPtr, MskPtr, Width);
    FrameDistortion += RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
    MskPtr += MskStride;
  }  

  flt64 PSNR  = CalcPSNRfromMaskedSSD((flt64)FrameDistortion, NumNonMasked, Tst->getBitDepth(), Msk->getBitDepth());
  bool  Exact = FrameDistortion == 0;
  if(Exact)
  {
    flt64 FakePSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth());
    PSNR  = FakePSNR;
  }

  return std::make_tuple(PSNR, Exact);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xPSNR::CalcPSNRfromSSD(flt64 SSD, int32 Area, int32 BitDepth)
{
  uint64 NumPoints = Area;
  uint64 MaxValue  = xBitDepth2MaxValue(BitDepth);
  uint64 MAX  = (NumPoints) * xPow2(MaxValue);
  flt64  PSNR = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : flt64_max;
  return PSNR;
}

flt64 xPSNR::CalcPSNRfromMaskedSSD(flt64 SSD, int32 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk)
{
  const int64 MaxValuePic = xBitDepth2MaxValue(BitDepthPic);
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);
  const int64 MAX         = (int64)NumNonMasked * xPow2(MaxValuePic) * MaxValueMsk;
  flt64       PSNR        = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : flt64_max;
  return PSNR;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB