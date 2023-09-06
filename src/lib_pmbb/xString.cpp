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


#include "xString.h"
#include <charconv>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

std::string xString::replaceFirst(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = Result.find(Token);
  if(CurrentPos != std::string::npos) { Result.replace(CurrentPos, Token.length(), ReplaceTo); }
  return Result;
}
std::string xString::replaceLast(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = Result.rfind(Token);
  if(CurrentPos != std::string::npos) { Result.replace(CurrentPos, Token.length(), ReplaceTo); }
  return Result;
}
std::string xString::replaceAll(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = 0;
  while((CurrentPos = Result.find(Token, CurrentPos)) != std::string::npos)
  {
    Result.replace(CurrentPos, Token.length(), ReplaceTo);
    CurrentPos += ReplaceTo.length();
  }
  return Result;
}
int32V2 xString::scanResolution(const std::string& ResolutionString)
{
  if(ResolutionString.empty() || ResolutionString.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  std::string_view ResolutionView = ResolutionString;
  ResolutionView = xString::stripR(xString::stripL(ResolutionView));

  if(ResolutionView.empty() || ResolutionView.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  uintPtr NumX = std::count_if(ResolutionView.begin(), ResolutionView.end(), [](char i) { return (i == 'x' || i == 'X'); });
  if(NumX != 1) { return { NOT_VALID, NOT_VALID }; }

  const uintSize Pos = xMin(ResolutionView.find('x'), ResolutionView.find('X'));
  const uintSize Len = ResolutionView.length();
  if(Pos == 0 || Pos == Len-1) { return { NOT_VALID, NOT_VALID }; }

  std::string_view WidthView  = ResolutionView.substr(0, Pos);
  std::string_view HeightView = ResolutionView.substr(Pos+1);
  
  //Parsing
  int32 Width  = NOT_VALID;
  int32 Height = NOT_VALID;
  std::from_chars(WidthView .data(), WidthView .data() + WidthView .length(), Width );
  std::from_chars(HeightView.data(), HeightView.data() + HeightView.length(), Height);

  return { Width, Height };
}
int32V4 xString::scanIntWeights(const std::string& CmpWeightsString) //parse vector of 4 nonnegative integers (format {d}:{d}:{d}:{d}), returns {-1, -1, -1, -1} on failure
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
  int32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  int32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  int32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  int32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}
flt32V4 xString::scanFltWeights(const std::string& CmpWeightsString) //parse vector of 4 integers (format {d}:{d}:{d}:{d})
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4<flt32>(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4<flt32>(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
#if defined(_MSC_VER)
  flt32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  flt32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  flt32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  flt32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
#else //don't use std::from_chars for float, since gcc (or rather libstdc++) is a bit retarded in this matter
  flt32 WeightLm = std::stof(std::string(WeightLmView));
  flt32 WeightCb = std::stof(std::string(WeightCbView));
  flt32 WeightCr = std::stof(std::string(WeightCrView));
  flt32 WeightXx = std::stof(std::string(WeightXxView));
#endif
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}


//===============================================================================================================================================================================================================

} //end of namespace PMBB
