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

#include "xWSPSNR.h"
#include "xPlane.h"

//SSE implementation
#if X_USE_SSE && X_SSE_ALL
#define X_CAN_USE_SSE 1
#else
#define X_CAN_USE_SSE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVPSNR : public xWSPSNR
{
public:
  //IVPSNR params
  static constexpr bool    c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;
  static constexpr int32   c_DefaultSearchRange   = 2;
  static constexpr int32V4 c_DefaultCmpWeights    = { 4, 1, 1, 4 };
  static constexpr flt32V4 c_DefaultUnntcbCoef    = { 0.01f, 0.01f, 0.01f, 0.0f };
  //debug calback types
  using tDCfGCS = std::function<void(const int32V4&)>;
  using tDCfQAP = std::function<void(flt64, flt64)>;

protected:
  int32   m_SearchRange       = c_DefaultSearchRange;
  int32V4 m_CmpWeightsAverage = c_DefaultCmpWeights;
  int32V4 m_CmpWeightsSearch  = c_DefaultCmpWeights;
  flt32V4 m_CmpUnntcbCoef     = c_DefaultUnntcbCoef;

  tDCfGCS m_DebugCallbackGCS;
  tDCfQAP m_DebugCallbackQAP;

public:
  void  setSearchRange (const int32   SearchRange) { m_SearchRange       = SearchRange; }
  void  setCmpWeights  (const int32V4& CmpWeights) { m_CmpWeightsAverage = CmpWeights; m_CmpWeightsSearch = CmpWeights; }
  void  setUnntcbCoef  (const flt32V4& UnntcbCoef) { m_CmpUnntcbCoef     = UnntcbCoef; }

  void  setDebugCallbackGCS(tDCfGCS DebugCallbackGCS) { m_DebugCallbackGCS = DebugCallbackGCS; }
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

  flt64 calcPicIVPSNR  (const xPicP* Ref, const xPicP* Tst, const xPicI* RefI = nullptr, const xPicI* TstI = nullptr);
  flt64 calcPicIVPSNRFlowCheck(const xPicP* Ref, const xPicP* Tst, const xPlane<flt32V2>* RefPlane = nullptr, const xPlane<flt32V2>* TstPlane = nullptr);
  flt64 calcPicIVPSNRFlowUse(const xPicP* Ref, const xPicP* Tst, const xPlane<flt32V2>* RefPlane = nullptr, const xPlane<flt32V2>* TstPlane = nullptr);

protected:
  //global color shift
  static int32V4 xCalcGlobalColorShift(const xPicP* Ref, const xPicP* Tst, const flt32V4& CmpUnntcbCoef, xThreadPoolInterface* ThreadPoolIf = nullptr);
  static flt64   xCalcAvgColorDiff    (const uint16* RefPtr, const uint16* TstPtr, const int32 RefStride, const int32 TstStride, const int32 Width, const int32 Height);

  //asymetric Q planar
  flt64          xCalcQualAsymmetricPic					(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift);
  flt64          xCalcQualAsymmetricPicFlowCheck		(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const xPlane<flt32V2>* RefPlane, const xPlane<flt32V2>* TstPlane);
  flt64          xCalcQualAsymmetricPicFlow				(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const xPlane<flt32V2>* RefPlane, const xPlane<flt32V2>* TstPlane);
  static int32V4 xCalcDistAsymmetricRow					(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32V4 xCalcDistAsymmetricRow					(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, const xPlane<flt32V2>* RefPlane, const xPlane<flt32V2>* TstPlane); // TODO: FINISH
  static int32   xFindBestPixelWithinBlock				(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
  static int32   xFindBestPixelWithinBlock				(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights, const xPlane<flt32V2>* RefPlane, const flt32V2& TstPos); // TODO: FINISH
  
  //asymetric Q interleaved
  flt64          xCalcQualAsymmetricPic   (const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift);
#if X_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow   (const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRow_SSE(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#else //X_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow   (const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRow_STD(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif //X_CAN_USE_SSE

  //asymetric Q interleaved - STD
  static int32V4 xCalcDistAsymmetricRow_STD   (const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32   xFindBestPixelWithinBlock_STD(const xPicI* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);

  //asymetric Q interleaved - SSE
#if X_CAN_USE_SSE
  static int32V4 xCalcDistAsymmetricRow_SSE(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static __m128i xCalcDistWithinBlock_SSE  (const xPicI* Ref, const __m128i& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeights);
#endif //X_CAN_USE_SSE

};

//===============================================================================================================================================================================================================

class xIVPSNRM : public xIVPSNR
{
public:
  flt64 calcPicIVPSNRM (const xPicP* Ref, const xPicP* Tst, const xPicP* Mask, const xPicI* RefI, const xPicI* TstI);

protected:
  //global color shift
  static int32V4 xCalcGlobalColorShiftM(const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, xThreadPoolInterface* ThreadPoolIf = nullptr);
  static int64   xCalcSumColorDiffM    (const uint16* RefPtr, const uint16* TstPtr, const uint16* MskPtr, const int32 RefStride, const int32 TstStride, const int32 MskStride, const int32 Width, const int32 Height);

  //asymetric Q interleaved
  flt64                  xCalcQualAsymmetricPicM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift, const int32 NumNonMasked);
  static inline uint64V4 xCalcDistAsymmetricRowM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRowM_STD(Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }

  //asymetric Q interleaved - STD
  static uint64V4 xCalcDistAsymmetricRowM_STD   (const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32    xFindBestPixelWithinBlockM_STD(const xPicI* Ref, const int32V4& TstPel, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB