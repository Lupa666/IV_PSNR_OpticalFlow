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
#include "xFile.h"
#include "xPic.h"

#if __has_include("xPlane.h")
#include "xPlane.h"
#define HAS_XPLANE 1
#else
#define HAS_XPLANE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSeq
{
public:
  enum class eMode : int32 { Unknown, Read, Write, Append };
  enum class eRetv : int32 { Success, EndOfFile, Error, WrongArg };

  static std::string_view ResultToString(eRetv Result)
  {
    switch(Result)
    {
      case eRetv::Success  : return "Success"  ; break;
      case eRetv::EndOfFile: return "EndOfFile"; break;
      case eRetv::Error    : return "Error"    ; break;
      case eRetv::WrongArg : return "WrongArg" ; break;
      default:               return "Unknown"  ; break;
    }
  }

  class tResult
  {
  protected:
    const eRetv       m_Result;
    const std::string m_Message;

  public:
    tResult(eRetv Result, const std::string Message = std::string()) : m_Result(Result), m_Message(Message) {}

    explicit operator bool() const { return CheckResult(m_Result); }

    static inline bool CheckResult(eRetv Result)
    {
      if(Result == eRetv::Success) { return true; }
      fmt::print("xSeq error type = {}", ResultToString(Result));
      return false;
    }

    inline bool operator== (const eRetv Res) const { return m_Result == Res; }
    inline bool operator!= (const eRetv Res) const { return m_Result != Res; }
  };

protected:
  std::string m_FileName;
  eMode       m_FileMode = eMode::Unknown;
  int64       m_FileSize = NOT_VALID;
  xFile       m_File;

  int32   m_Width           = NOT_VALID;
  int32   m_Height          = NOT_VALID;

  int32   m_BitsPerSample   = NOT_VALID;
  int32   m_BytesPerSample  = NOT_VALID;
  int32   m_ChromaFormat    = NOT_VALID;

  int32   m_FileCmpNumPels  = NOT_VALID;
  int32   m_FileCmpNumBytes = NOT_VALID;
  int32   m_FileImgNumBytes = NOT_VALID;

  int32   m_NumOfFrames     = NOT_VALID;
  int32   m_CurrFrameIdx    = NOT_VALID;

  uint8*  m_FileBuffer      = nullptr;

public:
  xSeq() { m_FileBuffer = nullptr; };
  xSeq(int32V2 Size, int32 BitDepth, int32 ChromaFormat) { create(Size, BitDepth, ChromaFormat); }
  ~xSeq() { destroy(); }

  void    create    (int32V2 Size, int32 BitDepth, int32 ChromaFormat);
  void    destroy   ();
  tResult openFile  (const std::string& FileName, eMode FileMode);
  tResult closeFile ();
  tResult seekFrame (int32 FrameNumber);
  tResult readFrame (xPicP*       Pic);
  tResult writeFrame(const xPicP* Pic);
#if HAS_XPLANE
  tResult readFrame (xPlane<uint16>*       Plane);
  tResult writeFrame(const xPlane<uint16>* Plane);
#endif //HAS_XPLANE

public:
  inline int32 getWidth   () const { return m_Width           ; }
  inline int32 getHeight  () const { return m_Height          ; }
  inline int32 getArea    () const { return m_Width * m_Height; }
  inline int32 getBitDepth() const { return m_BitsPerSample   ; }

  inline std::string getFileName() const { return m_FileName; }
  inline eMode       getFileMode() const { return m_FileMode; }
  inline int64       getFileSize() const { return m_FileSize; }

  inline int32       getNumOfFrames () const { return m_NumOfFrames ; }
  inline int32       getCurrFrameIdx() const { return m_CurrFrameIdx; }

protected:
  bool xUnpackFrame(      xPicP* Pic);
  bool xPackFrame  (const xPicP* Pic);
#if HAS_XPLANE
  bool xUnpackFrame(      xPlane<uint16>* Pic);
  bool xPackFrame  (const xPlane<uint16>* Pic);
#endif //HAS_XPLANE

public:
  static int32 calcNumFramesInFile(int32V2 Size, int32 BitDepth, int32 ChromaFormat, int64 FileSize);

  static tResult dumpFrame(const xPicP* Pic, const std::string& FileName, int32 ChromaFormat, bool Append); //slow stateless write for debug purposes
};

//===============================================================================================================================================================================================================

#ifdef XSEQ_IMPLEMENTATION
#undef HAS_XPLANE
#endif

} //end of namespace PMBB
