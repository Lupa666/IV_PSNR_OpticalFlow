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

#define XSEQ_IMPLEMENTATION
#include "xSeq.h"
#include "xPixelOps.h"
#include "xFile.h"
#include <cassert>
#include <cstring>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xSeq::create(int32V2 Size, int32 BitDepth, int32 ChromaFormat)
{
  m_Width  = Size.getX();
  m_Height = Size.getY();

  m_BitsPerSample  = BitDepth;
  m_BytesPerSample = m_BitsPerSample <= 8 ? 1 : 2;
  m_ChromaFormat   = ChromaFormat;

  m_FileCmpNumPels  = m_Width * m_Height;
  m_FileCmpNumBytes = m_Width * m_Height * m_BytesPerSample;

  switch(m_ChromaFormat)
  {
    case 444: m_FileImgNumBytes = 3 * m_FileCmpNumBytes; break;
    case 422: m_FileImgNumBytes = m_FileCmpNumBytes << 1; break;
    case 420: m_FileImgNumBytes = m_FileCmpNumBytes + (m_FileCmpNumBytes >> 1); break;
    case 400: m_FileImgNumBytes = m_FileCmpNumBytes; break;
    default: assert(0);
  }

  m_FileBuffer = (uint8*)xAlignedMalloc(m_FileImgNumBytes, xc_AlignmentPel);
}
void xSeq::destroy()
{
  m_FileName.clear();
  m_FileMode = eMode::Unknown;
  m_FileSize = NOT_VALID;
  m_File.close();

  m_Width  = NOT_VALID;
  m_Height = NOT_VALID;

  m_BitsPerSample  = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = NOT_VALID;

  m_FileCmpNumPels  = NOT_VALID;
  m_FileCmpNumBytes = NOT_VALID;

  if(m_FileBuffer) { xAlignedFree(m_FileBuffer); m_FileBuffer = nullptr; }
}
xSeq::tResult xSeq::openFile(const std::string& FileName, eMode FileMode)
{
  m_FileName   = FileName;
  m_FileMode   = FileMode;
  switch(FileMode)
  {
    case eMode::Read  : m_File.open(m_FileName, "rb"); break;
    case eMode::Write : m_File.open(m_FileName, "wb"); break;
    case eMode::Append: m_File.open(m_FileName, "ab"); break;
    default: return eRetv::WrongArg;
  }

  if(m_File.valid())
  {
    m_FileSize     = m_File.size();
    m_NumOfFrames  = calcNumFramesInFile({ m_Width, m_Height }, m_BitsPerSample, m_ChromaFormat, m_FileSize);
    m_CurrFrameIdx = 0;
  }
  else
  {
    m_FileSize     = NOT_VALID;
    m_NumOfFrames  = NOT_VALID;
    m_CurrFrameIdx = NOT_VALID;
  }
  
  eRetv Result = m_File.valid() ? eRetv::Success : eRetv::Error;
  return Result;
}
xSeq::tResult xSeq::closeFile()
{
  m_File.close();
  m_FileName.clear();
  m_FileMode = eMode::Unknown;
  m_FileSize = NOT_VALID;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;
  return eRetv::Success;
}
xSeq::tResult xSeq::seekFrame(int32 FrameNumber)
{
  if(m_FileMode == eMode::Read && FrameNumber >= m_NumOfFrames) { return eRetv::WrongArg; }
  if(m_FileMode != eMode::Read) { return eRetv::Error; }

  //seek frame
  uintSize Offset = (uintSize)m_FileImgNumBytes * (uintSize)FrameNumber;
  bool SeekResult = m_File.seek(Offset, xFile::seek_mode::beg);
  if(!SeekResult) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += FrameNumber;

  return eRetv::Success;
}
xSeq::tResult xSeq::readFrame(xPicP* Pic)
{
  if(m_FileMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_FileMode != eMode::Read) { return eRetv::Error; }

  //read frame
  uintSize Read = m_File.read(m_FileBuffer, m_FileImgNumBytes);
  if(Read != (uintSize)m_FileImgNumBytes) { return eRetv::Error; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Pic);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeq::tResult xSeq::writeFrame(const xPicP* Pic)
{
  //pack frame
  bool Packed = xPackFrame(Pic);
  if(!Packed) { return eRetv::Error; }

  //write frame
  uintSize Written = m_File.write(m_FileBuffer, m_FileImgNumBytes);
  if(Written != (uintSize)m_FileImgNumBytes) { return eRetv::Error; }
  m_File.flush();

  //update state
  m_FileSize     += m_FileImgNumBytes;
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
#if HAS_XPLANE
xSeq::tResult xSeq::readFrame(xPlane<uint16>* Plane)
{
  if(m_FileMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_FileMode != eMode::Read) { return eRetv::Error; }

  //read frame
  uintSize Read = m_File.read(m_FileBuffer, m_FileImgNumBytes);
  if(Read != (uintSize)m_FileImgNumBytes) { return eRetv::Error; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Plane);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeq::tResult xSeq::writeFrame(const xPlane<uint16>* Plane)
{
  //pack frame
  bool Packed = xPackFrame(Plane);
  if(!Packed) { return eRetv::Error; }

  //write frame
  uintSize Written = m_File.write(m_FileBuffer, m_FileImgNumBytes);
  if(Written != (uintSize)m_FileImgNumBytes) { return eRetv::Error; }
  m_File.flush();

  //update state
  m_FileSize     += m_FileImgNumBytes;
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
#endif //HAS_XPLANE

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool xSeq::xUnpackFrame(xPicP* Pic)
{
  uint16* PtrLm      = Pic->getAddr  (eCmp::LM);
  uint16* PtrCb      = Pic->getAddr  (eCmp::CB);
  uint16* PtrCr      = Pic->getAddr  (eCmp::CR);
  const int32 Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_FileBuffer           , Stride, m_Width, m_Width, m_Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_FileBuffer), Stride, m_Width, m_Width, m_Height); }

  //process chroma (if there is any chroma)
  if(m_ChromaFormat > 400)
  {
    const uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;

    if(m_ChromaFormat == 420)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtUpsample(PtrCb, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtUpsample(PtrCr, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
      else
      {
        xPixelOps::Upsample(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::Upsample(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
    }
    else if(m_ChromaFormat == 444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(PtrCb, ChromaPtr, Stride, m_Width, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Cvt(PtrCr, ChromaPtr, Stride, m_Width, m_Width, m_Height);
      }
      else
      { 
        xPixelOps::Copy(PtrCb, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Copy(PtrCr, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
      }
    }
    else { return false; }
  }
  return true;
}
bool xSeq::xPackFrame(const xPicP* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  (eCmp::LM);
  const uint16* PtrCb  = Pic->getAddr  (eCmp::CB);
  const uint16* PtrCr  = Pic->getAddr  (eCmp::CR);
  const int32   Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_FileBuffer           , PtrLm, m_Width, Stride, m_Width, m_Height); }
  else                      { xPixelOps::Copy((uint16*)(m_FileBuffer), PtrLm, m_Width, Stride, m_Width, m_Height); }

  //process chroma (if there is any chroma)
  if(m_ChromaFormat > 400)
  {
    uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
    if(m_ChromaFormat == 420)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtDownsample(ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtDownsample(ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
      else
      {
        xPixelOps::Downsample((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::Downsample((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
    }
    else if(m_ChromaFormat == 444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Cvt(ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
      }
      else
      { 
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
      }
    }
    else { return false; }
  }
  return true;
}

#if HAS_XPLANE
bool xSeq::xUnpackFrame(xPlane<uint16>* Pic)
{
  uint16* PtrLm      = Pic->getAddr  ();
  const int32 Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_FileBuffer           , Stride, m_Width, m_Width, m_Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_FileBuffer), Stride, m_Width, m_Width, m_Height); }

  return true;
}
bool xSeq::xPackFrame(const xPlane<uint16>* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  ();
  const int32   Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_FileBuffer           , PtrLm, m_Width, Stride, m_Width, m_Height); }
  else                      { xPixelOps::Copy((uint16*)(m_FileBuffer), PtrLm, m_Width, Stride, m_Width, m_Height); }

  //process chroma
  uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
  int32  CromaNumPels = 0;

  switch(m_ChromaFormat)
  {
    case 444: CromaNumPels = m_FileCmpNumPels << 1; break;
    case 422: CromaNumPels = m_FileCmpNumPels     ; break;
    case 420: CromaNumPels = m_FileCmpNumPels >> 1; break;
    case 400: CromaNumPels = 0                    ; break;
    default: assert(0); return false;
  }
  if(CromaNumPels)
  {
    if(m_BytesPerSample == 1) { xMemsetX((uint8* )ChromaPtr, (uint8 )xBitDepth2MidValue(m_BitsPerSample), CromaNumPels); }
    else                      { xMemsetX((uint16*)ChromaPtr, (uint16)xBitDepth2MidValue(m_BitsPerSample), CromaNumPels); }
  }

  return true;
}
#endif //HAS_XPLANE

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xSeq::calcNumFramesInFile(int32V2 Size, int32 BitDepth, int32 ChromaFormat, int64 FileSize)
{
  int32 BytesPerSample  = BitDepth <= 8 ? 1 : 2;
  int32 FileCmpNumPels  = Size.getMul();
  int32 FileCmpNumBytes = FileCmpNumPels * BytesPerSample;

  int32 FileImgNumBytes = NOT_VALID;
  switch(ChromaFormat)
  {
    case 444: FileImgNumBytes = 3 * FileCmpNumBytes; break;
    case 422: FileImgNumBytes = 2 * FileCmpNumBytes; break;
    case 420: FileImgNumBytes = FileCmpNumBytes + (FileCmpNumBytes >> 1); break;
    case 400: FileImgNumBytes = FileCmpNumBytes; break;
    default: assert(0);
  }

  return (int32)(FileSize / FileImgNumBytes);
}
xSeq::tResult xSeq::dumpFrame(const xPicP* Pic, const std::string& FileName, int32 ChromaFormat, bool Append)
{
  xSeq Seq(Pic->getSize(), Pic->getBitDepth(), ChromaFormat);
  tResult ResultOpen = Seq.openFile(FileName, Append ? eMode::Append : eMode::Write);
  if(ResultOpen != eRetv::Success) { return ResultOpen; }
  tResult ResultWrite = Seq.writeFrame(Pic);
  if(ResultWrite != eRetv::Success) { return ResultWrite; }
  tResult ResultClose = Seq.closeFile();
  if(ResultClose != eRetv::Success) { return ResultClose; }
  return eRetv::Success;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
