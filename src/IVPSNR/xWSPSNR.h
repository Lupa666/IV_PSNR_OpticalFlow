﻿#pragma once

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

#include "xPSNR.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xWSPSNR : public xPSNR
{
protected:
  bool                m_UseWS                        = false;
  bool                m_LegacyPeakValue8bitEmulation = false;
  std::vector<flt64 > m_EquirectangularWeights;
  flt64               m_DistortionCorrection = 1.0;

  std::vector<uint64> m_RowDistortions[4];
  std::vector<flt64 > m_RowErrors     [4];

public:
  void  init           (int32 Height);
  void  setLegacyWS8bit(bool LegacyPeakValue8bitEmulation) { m_LegacyPeakValue8bitEmulation = LegacyPeakValue8bitEmulation; }
  void  initWS         (bool UseWS, int32 Width, int32 Height, int32 BitDepth, int32 LonRangeDeg = 360, int32 LatRangeDeg = 180);

  tRes4 calcPicWSPSNR  (const xPicP* Tst, const xPicP* Ref                  );
  tRes4 calcPicWSPSNRM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);

protected:
  tRes1 calcCmpWSPSNR (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  tRes1 calcCmpWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB