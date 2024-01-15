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

#include "xIVPSNR.h"
#include "xPlane.h"
#include "xDistortion.h"
#include <iostream>
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVPSNR
//===============================================================================================================================================================================================================
flt64 xIVPSNR::calcPicIVPSNR(const xPicP* Ref, const xPicP* Tst, const xPicI* RefI, const xPicI* TstI)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  int32V4 GlobalColorShiftRef2Tst = xCalcGlobalColorShift(Ref, Tst, m_CmpUnntcbCoef, &m_ThreadPoolIf);
  int32V4 GlobalColorShiftTst2Ref = -GlobalColorShiftRef2Tst;
  
  flt64 R2T = std::numeric_limits<flt64>::quiet_NaN();
  flt64 T2R = std::numeric_limits<flt64>::quiet_NaN();
  if(RefI != nullptr && TstI != nullptr)
  {
    R2T = xCalcQualAsymmetricPic(RefI, TstI, GlobalColorShiftRef2Tst);
    T2R = xCalcQualAsymmetricPic(TstI, RefI, GlobalColorShiftTst2Ref);
  }
  else
  {
    R2T = xCalcQualAsymmetricPic(Ref, Tst, GlobalColorShiftRef2Tst);
    T2R = xCalcQualAsymmetricPic(Tst, Ref, GlobalColorShiftTst2Ref);
  }

  flt64 IVPSNR = xMin(R2T, T2R);

  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorShiftRef2Tst); }
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R               ); }

  return IVPSNR;
}

//===============================================================================================================================================================================================================
// xTIVPSNR
//===============================================================================================================================================================================================================

flt64 xTIVPSNR::calcPicIVPSNRFlowCheck(const xPicP* Ref, const xPicP* Tst, const tFlowPlane* RefPlane, const tFlowPlane* TstPlane)
{
    assert(Ref != nullptr && Tst != nullptr);
    assert(Ref->isCompatible(Tst));

    int32V4 GlobalColorShiftRef2Tst = xCalcGlobalColorShift(Ref, Tst, m_CmpUnntcbCoef, &m_ThreadPoolIf);
    int32V4 GlobalColorShiftTst2Ref = -GlobalColorShiftRef2Tst;

    flt64 R2T = std::numeric_limits<flt64>::quiet_NaN();
    flt64 T2R = std::numeric_limits<flt64>::quiet_NaN();

    R2T = xCalcQualAsymmetricPicFlowCheck(Ref, Tst, GlobalColorShiftRef2Tst, RefPlane, TstPlane);
    T2R = xCalcQualAsymmetricPicFlowCheck(Tst, Ref, GlobalColorShiftTst2Ref, TstPlane, RefPlane);

    flt64 IVPSNR = xMin(R2T, T2R);

    if (m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorShiftRef2Tst); }
    if (m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

    return IVPSNR;
}

flt64 xTIVPSNR::calcPicIVPSNRFlowUse(const xPicP* Ref, const xPicP* Tst, const tFlowPlane* RefPlane, const tFlowPlane* TstPlane)
{
    assert(Ref != nullptr && Tst != nullptr);
    assert(Ref->isCompatible(Tst));

    int32V4 GlobalColorShiftRef2Tst = xCalcGlobalColorShift(Ref, Tst, m_CmpUnntcbCoef, &m_ThreadPoolIf);
    int32V4 GlobalColorShiftTst2Ref = -GlobalColorShiftRef2Tst;

    flt64 R2T = std::numeric_limits<flt64>::quiet_NaN();
    flt64 T2R = std::numeric_limits<flt64>::quiet_NaN();

    R2T = xCalcQualAsymmetricPicFlow(Ref, Tst, GlobalColorShiftRef2Tst, RefPlane, TstPlane);
    T2R = xCalcQualAsymmetricPicFlow(Tst, Ref, GlobalColorShiftTst2Ref, TstPlane, RefPlane);

    flt64 IVPSNR = xMin(R2T, T2R);

    if (m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorShiftRef2Tst); }
    if (m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

    return IVPSNR;
}

flt64 xTIVPSNR::calcPicIVPSNROnlyFlow(const tFlowPlane* Ref, const tFlowPlane* Tst)
{
    assert(Ref != nullptr && Tst != nullptr);
    assert(Ref->isCompatible(Tst));

    flt64 R2T = std::numeric_limits<flt64>::quiet_NaN();
    flt64 T2R = std::numeric_limits<flt64>::quiet_NaN();

    R2T = xCalcQualAsymmetricPicOnlyFlow(Ref, Tst);
    T2R = xCalcQualAsymmetricPicOnlyFlow(Tst, Ref);

    flt64 IVPSNR = xMin(R2T, T2R);

    if (m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

    return IVPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// global color shift
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xIVPSNR::xCalcGlobalColorShift(const xPicP* Ref, const xPicP* Tst, const flt32V4& CmpUnntcbCoef, xThreadPoolInterface* ThreadPoolIf)
{
  const int32   MaxValue = Ref->getMaxPelValue();
  const int32V4 MaxDiff  = xRoundFltToInt32(CmpUnntcbCoef * (flt32)MaxValue);

  flt64V4 AvgColorDiff = { 0, 0, 0, 0 };

  if(ThreadPoolIf && ThreadPoolIf->isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      ThreadPoolIf->addWaitingTask([&AvgColorDiff, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/)
        { AvgColorDiff[CmpIdx] = xIVPSNR::xCalcAvgColorDiff(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Ref->getStride(), Tst->getStride(), Ref->getWidth(), Ref->getHeight()); }
      );
    }
    ThreadPoolIf->waitUntilTasksFinished(3);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      AvgColorDiff[CmpIdx] = xIVPSNR::xCalcAvgColorDiff(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Ref->getStride(), Tst->getStride(), Ref->getWidth(), Ref->getHeight());
    }
  }

  int32V4 GlobalColorShift = xRoundFltToInt32(AvgColorDiff);
  GlobalColorShift.modClip(-MaxDiff, MaxDiff);

  return GlobalColorShift;
}
flt64 xIVPSNR::xCalcAvgColorDiff(const uint16* RefPtr, const uint16* TstPtr, const int32 RefStride, const int32 TstStride, const int32 Width, const int32 Height)
{
  int32 SumColorDiff = xDistortion::CalcSD(RefPtr, TstPtr, RefStride, TstStride, Width, Height);
  int32 Area         = Width * Height;
  flt64 AvgColorDiff = (flt64)SumColorDiff / (flt64)Area;
  return AvgColorDiff;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();
  const int32 Area   = Ref->getArea  ();

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThreadPoolIf.addWaitingTask(
        [this, &Tst, &Ref, &GlobalColorShift, y](int32 /*ThreadIdx*/)
        {
          const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
          for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++)
    {
      const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
    }
  }

  flt64V4 FrameError = { 0, 0, 0, 0 };
  if(m_UseWS)
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowErrors[CmpIdx][y] = (flt64)(m_RowDistortions[CmpIdx][y]) * m_EquirectangularWeights[y]; }
    }

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = Accumulate(m_RowErrors[CmpIdx]); }
  }
  else //!m_UseWS
  {    
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = (flt64)std::accumulate(m_RowDistortions[CmpIdx].begin(), m_RowDistortions[CmpIdx].end(), (uint64)0); }
  }

  flt64V4 FrameQuality  = { 0, 0, 0, 0 };
  flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
  const flt64   WeightedFrameQuality          = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  return WeightedFrameQuality;
}
int32 xIVPSNR::xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
    const int32 BegY = CenterY - SearchRange;
    const int32 EndY = CenterY + SearchRange;
    const int32 BegX = CenterX - SearchRange;
    const int32 EndX = CenterX + SearchRange;

    const uint16* RefPtrY = Ref->getAddr(eCmp::LM);
    const uint16* RefPtrU = Ref->getAddr(eCmp::CB);
    const uint16* RefPtrV = Ref->getAddr(eCmp::CR);
    const int32   Stride = Ref->getStride();


    int32 BestError = std::numeric_limits<int32>::max();
    int32 BestOffset = NOT_VALID;

    for (int32 y = BegY; y <= EndY; y++)
    {
        for (int32 x = BegX; x <= EndX; x++)
        {
            const int32 Offset = y * Stride + x;
            const int32 DistY = xPow2(TstPel[0] - (int32)(RefPtrY[Offset]));
            const int32 DistU = xPow2(TstPel[1] - (int32)(RefPtrU[Offset]));
            const int32 DistV = xPow2(TstPel[2] - (int32)(RefPtrV[Offset]));
            if constexpr (c_UseRuntimeCmpWeights)
            {
                const int32 Error = DistY * CmpWeights[0] + DistU * CmpWeights[1] + DistV * CmpWeights[2];
                if (Error < BestError) { BestError = Error; BestOffset = Offset; }
            }
            else
            {
                const int32 Error = (DistY << 2) + DistU + DistV;
                if (Error < BestError) { BestError = Error; BestOffset = Offset; }
            }
        } //x
    } //y

    return BestOffset;
}
int32V4 xIVPSNR::xCalcDistAsymmetricRow(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
    const int32  Width = Tst->getWidth();
    const int32  TstStride = Tst->getStride();
    const int32  TstOffset = y * TstStride;

    int32V4 RowDist = { 0, 0, 0, 0 };

    const uint16* TstPtrY = Tst->getAddr(eCmp::LM) + TstOffset;
    const uint16* TstPtrU = Tst->getAddr(eCmp::CB) + TstOffset;
    const uint16* TstPtrV = Tst->getAddr(eCmp::CR) + TstOffset;

    //TODO: get the point

    for (int32 x = 0; x < Width; x++)
    {
        const int32V4 CurrTstValue = int32V4((int32)(TstPtrY[x]), (int32)(TstPtrU[x]), (int32)(TstPtrV[x]), 0) + GlobalColorShift;
        //flt32V2 CurrTstPlane = flt32V2(PlaneTst);
        const int32   BestRefOffset = xFindBestPixelWithinBlock(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);

        for (uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
        {
            const uint16* RefAddr = Ref->getAddr((eCmp)CmpIdx);
            int32 Diff = CurrTstValue[CmpIdx] - (int32)(RefAddr[BestRefOffset]);
            int32 Dist = xPow2(Diff);
            RowDist[CmpIdx] += Dist;
        }
    }//x

    return RowDist;
}

//===============================================================================================================================================================================================================
// xTIVPSNR - asymetric Q planar
//===============================================================================================================================================================================================================
flt64 xTIVPSNR::xCalcQualAsymmetricPicFlowCheck(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const tFlowPlane* RefPlane, const tFlowPlane* TstPlane)
{
    const int32 Height = Ref->getHeight();
    const int32 Area = Ref->getArea();

    if (m_ThreadPoolIf.isActive())
    {
        for (int32 y = 0; y < Height; y++)
        {
            m_ThreadPoolIf.addWaitingTask(
                [this, &Tst, &Ref, &GlobalColorShift, y, &RefPlane, &TstPlane](int32 /*ThreadIdx*/)
                {
                    const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, RefPlane, TstPlane);
                    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
                });
        }
        m_ThreadPoolIf.waitUntilTasksFinished(Height);
    }
    else
    {
        for (int32 y = 0; y < Height; y++)
        {
            const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, RefPlane, TstPlane);
            for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        }
    }

    flt64V4 FrameError = { 0, 0, 0, 0 };
    if (m_UseWS)
    {
        for (int32 y = 0; y < Height; y++)
        {
            for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowErrors[CmpIdx][y] = (flt64)(m_RowDistortions[CmpIdx][y]) * m_EquirectangularWeights[y]; }
        }

        for (uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = Accumulate(m_RowErrors[CmpIdx]); }
    }
    else //!m_UseWS
    {
        for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = (flt64)std::accumulate(m_RowDistortions[CmpIdx].begin(), m_RowDistortions[CmpIdx].end(), (uint64)0); }
    }

    flt64V4 FrameQuality = { 0, 0, 0, 0 };
    flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }

    const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
    const int32   SumCmpWeight = CmpWeightsAverage.getSum();
    const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
    const flt64   WeightedFrameQuality = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
    return WeightedFrameQuality;
}
flt64 xTIVPSNR::xCalcQualAsymmetricPicFlow(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const tFlowPlane* RefPlane, const tFlowPlane* TstPlane)
{
    const int32 Height = Ref->getHeight();
    const int32 Area = Ref->getArea();

    if (m_ThreadPoolIf.isActive())
    {
        for (int32 y = 0; y < Height; y++)
        {
            m_ThreadPoolIf.addWaitingTask(
                [this, &Tst, &Ref, &GlobalColorShift, y, &RefPlane, &TstPlane](int32 /*ThreadIdx*/)
                {
                    const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, RefPlane, TstPlane);
                    for (int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
                });
        }
        m_ThreadPoolIf.waitUntilTasksFinished(Height);
    }
    else
    {
        for (int32 y = 0; y < Height; y++)
        {
            const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, RefPlane, TstPlane);
            for (int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        }
    }

    flt64V4 FrameError = { 0, 0, 0, 0 };
    if (m_UseWS)
    {
        for (int32 y = 0; y < Height; y++)
        {
            for (int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { m_RowErrors[CmpIdx][y] = (flt64)(m_RowDistortions[CmpIdx][y]) * m_EquirectangularWeights[y]; }
            //m_RowErrors[3][y] = (flt64)(m_RowDistortions[3][y]) * m_EquirectangularWeights[y];
        }

        for (uint32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { FrameError[CmpIdx] = Accumulate(m_RowErrors[CmpIdx]); }
    }
    else //!m_UseWS
    {
        for (int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { FrameError[CmpIdx] = (flt64)std::accumulate(m_RowDistortions[CmpIdx].begin(), m_RowDistortions[CmpIdx].end(), (uint64)0); }
        //FrameError[3] = /*ADD 4TH PART*/0;
    }

    flt64V4 FrameQuality = { 0, 0, 0, 0 };
    flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
    //flt64   PSNR_20logMAXFlow = 20 * log10(flt64_max);
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }
    FrameQuality[3] = PSNR_20logMAX - (10 * log10((FrameError[3]) / Area)); //flow

    const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
    const int32   SumCmpWeight = CmpWeightsAverage.getSum();
    const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
    const flt64   WeightedFrameQuality = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
    return WeightedFrameQuality;
}
flt64 xTIVPSNR::xCalcQualAsymmetricPicOnlyFlow(const tFlowPlane* Ref, const tFlowPlane* Tst)
{
    const int32 Height = Ref->getHeight();
    const int32 Area = Ref->getArea();

    std::vector<flt64> RowDistortions(Height);

    if (m_ThreadPoolIf.isActive())
    {
        for (int32 y = 0; y < Height; y++)
        {
            m_ThreadPoolIf.addWaitingTask(
                [this, &Tst, &Ref, y, &RowDistortions](int32 /*ThreadIdx*/)
                {
                    const flt64 RowDist = xCalcDistAsymmetricRowOnlyFlow(Ref, Tst, y, m_SearchRange, m_CmpWeightsSearch);
                    RowDistortions[y] = RowDist;
                });
        }
        m_ThreadPoolIf.waitUntilTasksFinished(Height);
    }
    else
    {
        for (int32 y = 0; y < Height; y++)
        {
            const flt64 RowDist = (flt64)xCalcDistAsymmetricRowOnlyFlow(Ref, Tst, y, m_SearchRange, m_CmpWeightsSearch);
            RowDistortions[y] = RowDist;
        }
    }

    flt64 FrameError = 0;
    if (m_UseWS)
    {
        for (int32 y = 0; y < Height; y++)
        {
            m_RowErrors[0][y] = (flt64)(m_RowDistortions[0][y]) * m_EquirectangularWeights[y];
        }
        FrameError = Accumulate(m_RowErrors[0]);
    }
    else //!m_UseWS
    {
        FrameError = (flt64)std::accumulate(m_RowDistortions[0].begin(), m_RowDistortions[0].end(), (uint64)0);
    }

    flt64 FrameQuality = 0;
    flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
    //flt64   PSNR_20logMAXFlow = 20 * log10(flt64_max);
    FrameQuality = PSNR_20logMAX - 10 * log10((FrameError) / Area);

    //const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
    //const int32   SumCmpWeight = CmpWeightsAverage.getSum();
    //const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
    //const flt64   WeightedFrameQuality = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
    return FrameQuality;
}

int32V4 xTIVPSNR::xCalcDistAsymmetricRow(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, const tFlowPlane* RefPlane, const tFlowPlane* TstPlane)
{
    const int32  Width = Tst->getWidth();
    const int32  TstStride = Tst->getStride();
    const int32  TstOffset = y * TstStride;

    int32V4 RowDist = { 0, 0, 0, 0 };

    const uint16* TstPtrY = Tst->getAddr(eCmp::LM) + TstOffset;
    const uint16* TstPtrU = Tst->getAddr(eCmp::CB) + TstOffset;
    const uint16* TstPtrV = Tst->getAddr(eCmp::CR) + TstOffset;
    const flt32V2* TstPtrP = TstPlane->getAddr() + TstOffset;

    for (int32 x = 0; x < Width; x++)
    {
        const int32V4 CurrTstValue = int32V4((int32)(TstPtrY[x]), (int32)(TstPtrU[x]), (int32)(TstPtrV[x]), 0) + GlobalColorShift;
        const flt32V2 CurrTstPValue = TstPtrP[x];
        const int32   BestRefOffset = xFindBestPixelWithinBlock(Ref, CurrTstValue, x, y, SearchRange, CmpWeights, RefPlane, CurrTstPValue);

        for (uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
        {
            const uint16* RefAddr = Ref->getAddr((eCmp)CmpIdx);
            int32 Diff = CurrTstValue[CmpIdx] - (int32)(RefAddr[BestRefOffset]);
            int32 Dist = xPow2(Diff);
            RowDist[CmpIdx] += Dist;
        }
        const flt32V2* RefPlaneAddr = RefPlane->getAddr();
        flt32V2 Diff = CurrTstPValue - (RefPlaneAddr[BestRefOffset]);
        int32 Dist = (int32)xRoundFlt64ToInt32(xPow2(Diff[0]) + xPow2(Diff[1]));
        RowDist[3] += Dist;
    }//x

    return RowDist;
}
flt64 xTIVPSNR::xCalcDistAsymmetricRowOnlyFlow(const tFlowPlane* Ref, const tFlowPlane* Tst, const int32 y, const int32 SearchRange, const int32V4& CmpWeights)
{
    const int32  Width = Tst->getWidth();
    const int32  TstStride = Tst->getStride();
    const int32  TstOffset = y * TstStride;

    flt64 RowDist = 0.0;

    const flt32V2* TstPtrP = Tst->getAddr() + TstOffset;

    for (int32 x = 0; x < Width; x++)
    {
        const flt32V2 CurrTstValue = TstPtrP[x];
        const int32   BestRefOffset = xFindBestPixelWithinBlockOnlyFlow(Ref, CurrTstValue, x, y, SearchRange);

        const flt32V2* RefAddr = Ref->getAddr();
        flt32V2 Diff = CurrTstValue - (RefAddr[BestRefOffset]);
        flt32 Dist = (flt32)(xPow2(Diff[0]) + xPow2(Diff[1]));
        RowDist += Dist;
    }//x

    return RowDist;
}
int32 xTIVPSNR::xFindBestPixelWithinBlockOnlyFlow(const tFlowPlane* Ref, const flt32V2& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange) {

    const int32 BegY = CenterY - SearchRange;
    const int32 EndY = CenterY + SearchRange;
    const int32 BegX = CenterX - SearchRange;
    const int32 EndX = CenterX + SearchRange;

    const flt32V2* RefPtr = Ref->getAddr();
    const int32  Stride = Ref->getStride();


    int32 BestError = std::numeric_limits<int32>::max();
    int32 BestOffset = NOT_VALID;

    for (int32 y = BegY; y <= EndY; y++)
    {
        for (int32 x = BegX; x <= EndX; x++)
        {
            const int32 Offset = y * Stride + x;
            const flt32 Dist = xPow2(TstPel[0] - RefPtr[Offset][0]) + xPow2(TstPel[1] - RefPtr[Offset][1]);
            const flt32 Error = Dist;
            if (Error < BestError) { BestError = Error; BestOffset = Offset; }
        } //x
    } //y

    return BestOffset;
}

int32 xTIVPSNR::xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights, const tFlowPlane* RefPlane, const flt32V2& TstPos)
{
    const int32 BegY = CenterY - SearchRange;
    const int32 EndY = CenterY + SearchRange;
    const int32 BegX = CenterX - SearchRange;
    const int32 EndX = CenterX + SearchRange;

    const uint16* RefPtrY = Ref->getAddr(eCmp::LM);
    const uint16* RefPtrU = Ref->getAddr(eCmp::CB);
    const uint16* RefPtrV = Ref->getAddr(eCmp::CR);
    const int32   Stride = Ref->getStride();
    const flt32V2* RefPtrM = RefPlane->getAddr();

    int32 BestError = std::numeric_limits<int32>::max();
    int32 BestOffset = NOT_VALID;

    for (int32 y = BegY; y <= EndY; y++)
    {
        for (int32 x = BegX; x <= EndX; x++)
        {
            const int32 Offset = y * Stride + x;
            const int32 DistY = xPow2(TstPel[0] - (int32)(RefPtrY[Offset]));
            const int32 DistU = xPow2(TstPel[1] - (int32)(RefPtrU[Offset]));
            const int32 DistV = xPow2(TstPel[2] - (int32)(RefPtrV[Offset]));
            const flt32 DistM = xPow2(TstPos[0] - RefPtrM[Offset][0]) + xPow2(TstPos[1] - RefPtrM[Offset][1]);
            if constexpr (c_UseRuntimeCmpWeights)
            {
                const int32 Error = DistY * CmpWeights[0] + DistU * CmpWeights[1] + DistV * CmpWeights[2] + DistM * CmpWeights[3];
                if (Error < BestError) { BestError = Error; BestOffset = Offset; }
            }
            else
            {
                const int32 Error = (DistY << 2) + DistU + DistV;
                if (Error < BestError) { BestError = Error; BestOffset = Offset; }
            }
        } //x
    } //y

    return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();
  const int32 Area   = Ref->getArea  ();

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThreadPoolIf.addWaitingTask(
        [this, &Tst, &Ref, &GlobalColorShift, y](int32 /*ThreadIdx*/)
        {
          const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
          for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++)
    {
      const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
    }
  }

  flt64V4 FrameError = { 0, 0, 0, 0 };
  if(m_UseWS)
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowErrors[CmpIdx][y] = (flt64)(m_RowDistortions[CmpIdx][y]) * m_EquirectangularWeights[y]; }
    }

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = Accumulate(m_RowErrors[CmpIdx]); }
  }
  else //!m_UseWS
  {    
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = (flt64)std::accumulate(m_RowDistortions[CmpIdx].begin(), m_RowDistortions[CmpIdx].end(), (uint64)0); }
  }

  flt64V4 FrameQuality  = { 0, 0, 0, 0 };
  flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
  const flt64   WeightedFrameQuality          = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  return WeightedFrameQuality;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - STD
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xIVPSNR::xCalcDistAsymmetricRow_STD(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  int32V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr  = Tst->getAddr() + TstOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlock_STD(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]);
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += Dist;
  }//x

  return RowDist;
}
int32 xIVPSNR::xFindBestPixelWithinBlock_STD(const xPicI* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const int32     Stride = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32   Offset = y * Stride + x;
      const int32V4 RefPel = (int32V4)(RefPtr[Offset]);
      const int32V4 Dist   = (TstPel - RefPel).getVecPow2();
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = (Dist * CmpWeights).getSum();
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (Dist[0] << 2) + Dist[1] + Dist[2];
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - SSE
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if X_CAN_USE_SSE
int32V4 xIVPSNR::xCalcDistAsymmetricRow_SSE(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const __m128i CmpWeightsV       = _mm_loadu_si128((__m128i*) &CmpWeights);
  const __m128i GlobalColorShiftV = _mm_loadu_si128((__m128i*) &GlobalColorShift);

  const uint16V4* TstPtr = Tst->getAddr() + TstOffset;
  __m128i RowDistV = _mm_setzero_si128();
  for (int32 x = 0; x < Width; x++)
  {
    __m128i TstU16V  = _mm_loadl_epi64((__m128i*)(TstPtr + x));
    __m128i TstV     = _mm_add_epi32(_mm_unpacklo_epi16(TstU16V, _mm_setzero_si128()), GlobalColorShiftV);
    __m128i BestDist = xCalcDistWithinBlock_SSE(Ref, TstV, x, y, SearchRange, CmpWeightsV);
    RowDistV = _mm_add_epi32(RowDistV, BestDist);
  }//x

  int32V4 RowDist;
  _mm_storeu_si128((__m128i*)&RowDist, RowDistV);
  return RowDist;
}
__m128i xIVPSNR::xCalcDistWithinBlock_SSE(const xPicI* Ref, const __m128i& TstPelV, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeightsV)
{
  const int32 WindowSize = 2 * SearchRange + 1;
  const int32 BegY = CenterY - SearchRange;
  const int32 BegX = CenterX - SearchRange;

  const int32     Stride = Ref->getStride();
  const uint16V4* RefPtr = Ref->getAddr() + BegY * Stride + BegX;

  int32   BestError = std::numeric_limits<int32>::max();
  __m128i BestDistV = _mm_setzero_si128();

  for (int32 y = 0; y < WindowSize; y++)
  {
    const uint16V4* RefPtrY = RefPtr + y * Stride;
    for (int32 x = 0; x < WindowSize; x++)
    {
      __m128i RefU16V = _mm_loadl_epi64((__m128i*)(RefPtrY + x));
    //__m128i RefV    = _mm_unpacklo_epi16(RefU16V, _mm_setzero_si128());
      __m128i RefV    = _mm_cvtepu16_epi32(RefU16V);
      __m128i DiffV   = _mm_sub_epi32     (TstPelV, RefV);
      __m128i DistV   = _mm_mullo_epi32   (DiffV, DiffV);
      __m128i ErrorV  = _mm_mullo_epi32   (DistV, CmpWeightsV);
      __m128i Tmp1    = _mm_hadd_epi32    (ErrorV, ErrorV);
      __m128i Tmp2    = _mm_hadd_epi32    (Tmp1, Tmp1);
      int32   Error   = _mm_extract_epi32 (Tmp2, 0);
      if (Error < BestError) { BestError = Error; BestDistV = DistV; }
    } //x
  } //y

  return BestDistV;
}
#endif //X_CAN_USE_SSE

//===============================================================================================================================================================================================================

} //end of namespace PMBB