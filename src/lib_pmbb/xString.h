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
#include "xVec.h"
#include <string>
#include <sstream>
#include <vector>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xString
{
public:
  static inline bool xIsAlpha   (int32 c) { return ((c>='a' && c<='z') || (c>='A' && c<='Z')); }  
  static inline bool xIsNumeric (int32 c) { return ( c>='0' && c<='9'); }
  static inline bool xIsAlphaNum(int32 c) { return (xIsAlpha(c) || xIsNumeric(c)); }
  static inline bool xIsBlank   (int32 c) { return (c==' '  || c=='\a' || c=='\b' || c=='\t' || c=='\f'); }
  static inline bool xIsEndl    (int32 c) { return (c=='\n' || c=='\r' || c=='\v'); }
  static inline bool xIsSpace   (int32 c) { return (c==' '  || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r'); }

public:
  static std::string replaceFirst(const std::string& Source, const std::string& Token, const std::string& ReplaceTo);
  static std::string replaceLast (const std::string& Source, const std::string& Token, const std::string& ReplaceTo);
  static std::string replaceAll  (const std::string& Source, const std::string& Token, const std::string& ReplaceTo);

public:
  static inline std::string_view stripR(std::string_view S) { while(S.length() > 0 && xIsSpace(S.back ())) { S.remove_suffix(1); } return S; }
  static inline std::string_view stripL(std::string_view S) { while(S.length() > 0 && xIsSpace(S.front())) { S.remove_prefix(1); } return S; }

  static inline void trimL(std::string& s) { s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {  return !xIsSpace(ch); })); }
  static inline void trimR(std::string& s) { s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !xIsSpace(ch); }).base(), s.end()); }

  static int32V2 scanResolution(const std::string& ResolutionString); //parse resolution (format {d}x{d} or {d}X{d})
  static int32V4 scanIntWeights(const std::string& CmpWeightsString); //parse vector of 4 nonnegative integers (format {d}:{d}:{d}:{d}), returns {-1, -1, -1, -1} on failure
  static flt32V4 scanFltWeights(const std::string& CmpWeightsString); //parse vector of 4 nonnegative floats   (format {f}:{f}:{f}:{f}), returns {-1, -1, -1, -1} on failure

  static std::string formatResolution(const int32V2 Resolution) { return fmt::sprintf("%dx%d", Resolution.getX(), Resolution.getY()); }
  static std::string formatIntWeights(const int32V4 CmpWeights) { return fmt::sprintf("%d:%d:%d:%d", CmpWeights[0], CmpWeights[1], CmpWeights[2], CmpWeights[3]); }
  static std::string formatFltWeights(const flt32V4 CmpWeights) { return fmt::sprintf("%.2f:%.2f:%.2f:%.2f", CmpWeights[0], CmpWeights[1], CmpWeights[2], CmpWeights[3]); }

  static void printError(const std::string& Message)
  {
    fmt::fprintf(stdout, Message + "\n");
    fmt::fprintf(stderr, Message + "\n");
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

