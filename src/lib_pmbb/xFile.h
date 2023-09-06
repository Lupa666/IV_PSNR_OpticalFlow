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


// MSVC workaround
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "xCommonDefPMBB.h"
#include <cstdio>
#include <string>
#include <cerrno>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xFile
{
public:
  enum class seek_mode : int32 { beg = 0, cur = 1, end = 2 };

  #if X_SYSTEM_WINDOWS
  static inline int64 xFtell64(FILE* FileStream                            ) { return _ftelli64(FileStream                ); }
  static inline int32 xFseek64(FILE* FileStream, int64 Offset, int32 Origin) { return _fseeki64(FileStream, Offset, Origin); }
#elif X_SYSTEM_LINUX
  static inline int64 xFtell64(FILE* FileStream                            ) { return ftello64(FileStream                ); }
  static inline int32 xFseek64(FILE* FileStream, int64 Offset, int32 Origin) { return fseeko64(FileStream, Offset, Origin); }
#else
  #error Unrecognized platform
#endif

protected:
  FILE*  m_FileHandle;

public:
  inline       xFile (                                                  ) { m_FileHandle = nullptr; }
  inline       xFile (const std::string FilePath, const std::string Attr) { m_FileHandle = nullptr; open(FilePath, Attr); }
  inline       ~xFile(                                                  ) { close(); }
  inline void   open (const std::string FilePath, const std::string Attr); 
  inline void   open (const char*       FilePath, const char*       Attr) { open(std::string(FilePath), std::string(Attr)); };
  inline void   close();

  inline uint64 read (      void* Memmory, uint32 Length) { return fread (Memmory, 1, Length, m_FileHandle); }
  inline uint64 write(const void* Memmory, uint32 Length) { return fwrite(Memmory, 1, Length, m_FileHandle); }
  inline uint64 write(const std::string& String         ) { return write(String.c_str(), (uint32)String.size()); }
  inline bool   skip (uint32 Length) {   return seek(Length, seek_mode::cur); }

  inline byte   get  (         ) { return (byte)getc(m_FileHandle); }
  inline void   put  (byte Byte) { putc(Byte, m_FileHandle); }

  inline bool   valid() { return (m_FileHandle!=nullptr && !ferror(m_FileHandle)); }
  inline uint64 size ();
  inline uint64 tell () { return ftell(m_FileHandle); }
  inline bool   end  ();

  inline bool   seek (int64 Position, seek_mode SeekMode);  
  inline void   flush() { fflush(m_FileHandle); }

protected:
  inline void   xPrinterror() { fmt::printf("%s ", strerror(errno)); }

public:
  static inline bool  exist   (const std::string FilePath);
  static inline int64 filesize(const std::string FilePath);
};

//=============================================================================================================================================================================

void xFile::open(const std::string FilePath, const std::string Attr)
{
  if(m_FileHandle != nullptr) { return; }
  if(FilePath=="" || Attr=="") { return; };
  std::string Mode = Attr;
  if(Mode.find('b') == std::string::npos) { Mode.append(1, 'b'); }
  m_FileHandle = fopen(FilePath.c_str(), Mode.c_str());
  if(m_FileHandle==nullptr) { xPrinterror(); fmt::printf("(%s)\n", FilePath); }
}
void xFile::close()
{
  if(m_FileHandle!=nullptr)
  {
    fclose(m_FileHandle);
    m_FileHandle = nullptr;
  }
}
uint64 xFile::size()
{
  int64 CurrPosition = xFtell64(m_FileHandle);
  xFseek64(m_FileHandle, 0, (int)seek_mode::end);
  int64 Size = xFtell64(m_FileHandle);
  xFseek64(m_FileHandle, CurrPosition, (int)seek_mode::beg);
  return Size;
}
bool xFile::end()
{
  int64 CurrPosition = xFtell64(m_FileHandle);
  xFseek64(m_FileHandle, 0, (int)seek_mode::end);
  int64 EndPosition = xFtell64(m_FileHandle);
  if(CurrPosition == EndPosition) { return true; }
  xFseek64(m_FileHandle, CurrPosition, (int)seek_mode::beg);
  return false;
}
bool xFile::seek(int64 Position, seek_mode SeekMode)
{
  int32 Result = xFseek64(m_FileHandle, Position, (int)SeekMode);
  if(Result!=0) { xPrinterror(); }
  return (Result == 0);
}
bool xFile::exist(const std::string FilePath)
{
  FILE* FileHandle = fopen(FilePath.c_str(), "r");
  if(FileHandle != nullptr)
  {
    fclose(FileHandle);
    return true;
  }  
  return false;
}
int64 xFile::filesize(const std::string FilePath)
{
  FILE* FileHandle = fopen(FilePath.c_str(), "rb");
  if(FileHandle == nullptr) { return NOT_VALID; }
  xFseek64(FileHandle, 0, (int)seek_mode::end);
  int64 EndPosition = xFtell64(FileHandle);
  fclose(FileHandle);
  return EndPosition;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
