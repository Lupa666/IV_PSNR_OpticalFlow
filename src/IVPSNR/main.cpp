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

//===============================================================================================================================================================================================================

#include "xFile.h"
#include "xSeq.h"
#include "xIVPSNR.h"
#include "xCfgINI.h"
#include "xUtilsOCV.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
#include <iostream>
#include "fmt/chrono.h"
#include "xPlane.h"
#include <opencv2/opencv.hpp>

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const char BannerString[] =
R"IVPSNRRAWSTRING(
=============================================================================

IV-PSNR software v4.0-dev

Copyright (c) 2010-2021, ISO/IEC, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Adrian Dziembowski

=============================================================================

)IVPSNRRAWSTRING";

static const char HelpString[] =
R"AVLIBRAWSTRING(
=============================================================================
IV-PSNR software v4.0-dev

Usage:

 Cmd | ParamName        | Description
 -i0   InputFile0         File path - sequence 0
 -i1   InputFile1         File path - sequence 1
 -w    PictureWidth       Width of sequence
 -h    PictureHeight      Height of sequence
 -bd   BitDepth           Bit depth     (optional, default 8, up to 14) 
 -cf   ChromaFormat       Chroma format (optional, default 420) [420, 444]
 -s0   StartFrame0        Start frame   (optional, default 0) 
 -s1   StartFrame1        Start frame   (optional, default 0) 
 -l    NumberOfFrames     Number of frames to be processed 
                          (optional, default -1=all)
 -o    OutputFile         Output file path (optional)

 -im   InputFileM         File path - mask (optional)
 -bdm  BitDepthM          Bit depth for mask     (optional, default=BitDepth, up to 14)
 -cfm  ChromaFormatM      Chroma format for mask (optional, default=ChromaFormat) [400, 420, 444]

 -erp  Equirectangular    Equirectangular sequence (flag, default disabled)
 -lor  LonRangeDeg        Range for ERP sequence in degrees - Longitudinal
                          (optional, default 360)
 -lar  LatRangeDeg        Range for ERP sequence in degrees - Lateral
                          (optional, default 180)

 -sr   SearchRange        IV-PSNR search range around center point
                          (optional, default 2=5x5)
 -cws  ComponentWeights   IV-PSNR component weights
                          ("Lm:Cb:Cr:0" - per component integer weight, default "4:1:1:0")
                          quotes are required
 -unc  UnnoticeableCoef   IV-PSNR unnoticable color difference threshold coeff
                          ("Lm:Cb:Cr:0" - per component coeff, default "0.01:0.01:0.01:0")
                          quotes are required
 -ws8  Peak8bitWSPSNR     Use 1020 as peak value for 10-bps videos in WSPSNR metric
                          (provides compatibility with original WSPSNR implementation
                          optional, default=1)

 -t    NumberOfThreads    Number of worker threads
                          (optional, default -1=all, suggested 4-8)
 -ilp  InterleavedPic     Use additional image buffer with interleaved layout for IVPSNR 
                          (improves performance at a cost of increased memory usage
                          optional, default=1)
 -v    VerboseLevel       Verbose level (optional, default=2)

 -c    "config.cfg"       External config file - in INI format (optional)

VerboseLevel:
  0 = final PSNR, WSPSNR, IVPSNR values only
  1 = 0 + configuration + detected frame numbers
  2 = 1 + argc/argv + frame level PSNR, WSPSNR, IVPSNR
  3 = 2 + computing time (LOAD, PSNR, WSPSNR, IVPSNR)
          (uses high_resolution_clock, could slightly slow down computations)
  4 = 3 + IVPSNR specific debug data (GlobalColorShift, R2T+T2R)

Example - commandline parameters:
  IVPSNR -i0 "A.yuv" -i1 "B.yuv" -w 2048 -h 1088 -bd 10 -cf 420 -v 3 -o "o.txt"

Example - config file:
  InputFile0      = "A.yuv"
  InputFile1      = "B.yuv"
  PictureWidth    = 2048
  PictureHeight   = 1088
  BitDepth        = 10
  ChromaFormat    = 420
  VerboseLevel    = 3
  OutputFile      = "o.txt"


=============================================================================
)AVLIBRAWSTRING";

//===============================================================================================================================================================================================================

static void xPrintError(std::string Message)
{
  fmt::fprintf(stdout, Message + "\n");
  fmt::fprintf(stderr, Message + "\n");
}

//===============================================================================================================================================================================================================
// Main
//===============================================================================================================================================================================================================
#ifndef IVPSNR_MAIN
#define IVPSNR_MAIN main
#endif

int32 IVPSNR_MAIN(int argc, char *argv[], char* /*envp*/[])
{
  fmt::printf(BannerString);
  fmt::printf("\n");

  //==============================================================================
  // parsing configuration
  xCfgINI::xParser CfgParser;
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-i0" , "", "InputFile0"          ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-i1" , "", "InputFile1"          ));  
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-w"  , "", "PictureWidth"        ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-h"  , "", "PictureHeight"       ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-bd" , "", "BitDepth"            ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-cf" , "", "ChromaFormat"        ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-s0" , "", "StartFrame0"         ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-s1" , "", "StartFrame1"         ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-l"  , "", "NumberOfFrames"      ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-o"  , "", "OutputFile"          ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-im" , "", "InputFileM"          ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-bdm", "", "BitDepthM"           ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-cfm", "", "ChromaFormatM"       ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-erp", "", "Equirectangular", "1"));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-lor", "", "LonRangeDeg"         ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-lar", "", "LatRangeDeg"         ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-sr" , "", "SearchRange"         ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-cws", "", "ComponentWeights"    ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-unc", "", "UnnoticeableCoef"    )); 
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-ws8", "", "Legacy8bitWSPSNR"    ));  
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-t"  , "", "NumberOfThreads"     ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-ilp", "", "InterleavedPic"      ));
  CfgParser.addCommandlineParam(xCfgINI::xCmdParam("-v"  , "", "VerboseLevel"        ));
  

  bool CommandlineResult = CfgParser.loadFromCommandline(argc, argv);
  if(!CommandlineResult) { xCfgINI::printErrorMessage("! invalid commandline\n", HelpString); return EXIT_FAILURE; }
   
  //TODO: temp variables, to be changed later
   
  bool CalcCheckFlow = true;
  bool CalcPSNRFlow = true;
  bool CalcIVPSNRFlow = true;
  bool CalcIVPSNRFlowOnly = true;

  //TODO: temp variables, to be changed later

  //readed from commandline/config 
  constexpr int32 NumInputsMax = 3;

  std::string InputFile [NumInputsMax];
  int32       StartFrame[2];
              InputFile[0]       = CfgParser.getParam1stArg("InputFile0"      , std::string(""));
              InputFile[1]       = CfgParser.getParam1stArg("InputFile1"      , std::string(""));  
  int32       PictureWidth       = CfgParser.getParam1stArg("PictureWidth"    , NOT_VALID      );
  int32       PictureHeight      = CfgParser.getParam1stArg("PictureHeight"   , NOT_VALID      );
  int32       BitDepth           = CfgParser.getParam1stArg("BitDepth"        , 8              );  
  int32       ChromaFormat       = CfgParser.getParam1stArg("ChromaFormat"    , 420            );
              StartFrame[0]      = CfgParser.getParam1stArg("StartFrame0"     , 0              );
              StartFrame[1]      = CfgParser.getParam1stArg("StartFrame1"     , 0              );
  int32       NumberOfFrames     = CfgParser.getParam1stArg("NumberOfFrames"  , -1             );  
  std::string OutputFile         = CfgParser.getParam1stArg("OutputFile"      , std::string(""));
              InputFile[2]       = CfgParser.getParam1stArg("InputFileM"      , std::string(""));
  int32       BitDepthM          = CfgParser.getParam1stArg("BitDepthM"       , BitDepth       );
  int32       ChromaFormatM      = CfgParser.getParam1stArg("ChromaFormatM"   , ChromaFormat   );              
  bool        Calc__PSNR         = CfgParser.getParam1stArg("Calc__PSNR"      , true);
  bool        CalcWSPSNR         = CfgParser.getParam1stArg("CalcWSPSNR"      , true);
  bool        CalcIVPSNR         = CfgParser.getParam1stArg("CalcIVPSNR"      , true);
  bool        IsEquirectangular  = CfgParser.getParam1stArg("Equirectangular" , false          );
  int32       LonRangeDeg        = CfgParser.getParam1stArg("LonRangeDeg"     , 360            );
  int32       LatRangeDeg        = CfgParser.getParam1stArg("LatRangeDeg"     , 180            );
  int32       SearchRange        = CfgParser.getParam1stArg("SearchRange"     , xIVPSNR::c_DefaultSearchRange);
  std::string ComponentWeightsS  = CfgParser.getParam1stArg("ComponentWeights", xString::formatIntWeights(xIVPSNR::c_DefaultCmpWeights));
  int32V4     ComponentWeights   = xString::scanIntWeights(ComponentWeightsS);
  std::string UnnoticeableCoefS  = CfgParser.getParam1stArg("UnnoticeableCoef", xString::formatFltWeights(xIVPSNR::c_DefaultUnntcbCoef));
  flt32V4     UnnoticeableCoef   = xString::scanFltWeights(UnnoticeableCoefS);
  bool        Legacy8bitWSPSNR   = CfgParser.getParam1stArg("Legacy8bitWSPSNR", true           );
  int32       NumberOfThreads    = CfgParser.getParam1stArg("NumberOfThreads" , NOT_VALID      );
  bool        InterleavedPic     = CfgParser.getParam1stArg("InterleavedPic"  , true           );
  int32       VerboseLevel       = CfgParser.getParam1stArg("VerboseLevel"    , 1              );

  if(VerboseLevel >= 2) { fmt::printf("Commandline args:\n");  xCfgINI::printCommandlineArgs(argc, argv); }

  //derrived
  const int32V2 PictureSize   = { PictureWidth, PictureHeight };
  const int32   PictureMargin = xRoundUpToNearestMultiple(SearchRange, 2);
  const int32   WindowSize    = 2 * SearchRange + 1;

  const bool    UseMask       = !InputFile[2].empty();
  const int32   NumInputsCur  = !UseMask ? 2 : 3;
  const std::string Suffix    = !UseMask ? "" : "-M";

  //print compile time setup
  if (VerboseLevel >= 1)
  {
    fmt::printf("Compile-time configuration:\n");
    fmt::printf("USE_SIMD               = %d\n", USE_SIMD                 );
    fmt::printf("USE_KBNS               = %d\n", xc_USE_KBNS              );
    fmt::printf("USE_RUNTIME_CMPWEIGHTS = %d\n", xc_USE_RUNTIME_CMPWEIGHTS);
    fmt::printf("\n");
  }



  //print config
  if(VerboseLevel >= 1)
  {
    fmt::printf("Run-time configuration:\n");
    fmt::printf("InputFile0       = %s\n"  , InputFile[0]     );
    fmt::printf("InputFile1       = %s\n"  , InputFile[1]     );    
    fmt::printf("PictureWidth     = %d\n"  , PictureWidth     );
    fmt::printf("PictureHeight    = %d\n"  , PictureHeight    );
    fmt::printf("BitDepth         = %d\n"  , BitDepth         );
    fmt::printf("ChromaFormat     = %d\n"  , ChromaFormat     );
    fmt::printf("StartFrame0      = %d\n"  , StartFrame[0]    );
    fmt::printf("StartFrame1      = %d\n"  , StartFrame[1]    );
    fmt::printf("NumberOfFrames   = %d%s\n", NumberOfFrames, NumberOfFrames==NOT_VALID ? "  (all)" : "");
    fmt::printf("OutputFile       = %s\n"  , OutputFile.empty() ? "(unused)" : OutputFile);
    fmt::printf("InputFileM       = %s\n"  , InputFile[2].empty() ? "(unused)" : InputFile[2]);
    fmt::printf("BitDepthM        = %d\n"  , BitDepthM        );
    fmt::printf("ChromaFormatM    = %d\n"  , ChromaFormatM    );
    fmt::printf("Calc__PSNR       = %d\n"  , Calc__PSNR       );
    fmt::printf("CalcWSPSNR       = %d\n"  , CalcWSPSNR       );
    fmt::printf("CalcIVPSNR       = %d\n"  , CalcIVPSNR       );
    fmt::printf("Equirectangular  = %d\n"  , IsEquirectangular);
    fmt::printf("LonRangeDeg      = %d\n"  , LonRangeDeg      );
    fmt::printf("LatRangeDeg      = %d\n"  , LatRangeDeg      );
    fmt::printf("SearchRange      = %d%s\n", SearchRange, SearchRange == xIVPSNR::c_DefaultSearchRange ? "  (default)" : "  (custom)");
    fmt::printf("ComponentWeights = %s%s\n", xString::formatIntWeights(ComponentWeights), ComponentWeights == xIVPSNR::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
    fmt::printf("UnnoticeableCoef = %s%s\n", xString::formatFltWeights(UnnoticeableCoef), UnnoticeableCoef == xIVPSNR::c_DefaultUnntcbCoef ? "  (default)" : "  (custom)");
    fmt::printf("Legacy8bitWSPSNR = %d\n"  , Legacy8bitWSPSNR );
    fmt::printf("NumberOfThreads  = %d%s\n", NumberOfThreads, NumberOfThreads == NOT_VALID ? "  (all)" : "");
    fmt::printf("InterleavedPic   = %d\n"  , InterleavedPic   );
    fmt::printf("VerboseLevel     = %d\n"  , VerboseLevel     );    
    fmt::printf("\n");
    fmt::printf("Run-time derrived parameters:\n");
    fmt::printf("WindowSize       = %dx%d\n", WindowSize, WindowSize);
    fmt::printf("PictureMargin    = %d\n"   , PictureMargin);
    fmt::printf("UseMask          = %d\n"   , UseMask);
    fmt::printf("\n");
  }

  //check hardware concurrency
  int32 HardwareConcurency  = std::thread::hardware_concurrency();
  int32 NumberOfThreadsUsed = NumberOfThreads < 0 ? HardwareConcurency : std::min(NumberOfThreads, HardwareConcurency);
  if (VerboseLevel >= 1)
  {
    fmt::printf("Multithreading:\n");
    fmt::printf("HardwareConcurency  = %d\n", HardwareConcurency );
    fmt::printf("NumberOfThreadsUsed = %d\n", NumberOfThreadsUsed);
    fmt::printf("\n");
  }

  //check config
  std::string CfgMsg;
  if (InputFile[0].empty()              ) { CfgMsg += "CONFIGURATION ERROR: InputFile0 is empty                 \n"; }
  if (InputFile[1].empty()              ) { CfgMsg += "CONFIGURATION ERROR: InputFile1 is empty                 \n"; }
  if (PictureWidth <= 0                 ) { CfgMsg += "CONFIGURATION ERROR: Invalid PictureWidth value          \n"; }
  if (PictureHeight <= 0                ) { CfgMsg += "CONFIGURATION ERROR: Invalid PictureHeight value         \n"; }
  if (BitDepth < 8 || BitDepth > 14     ) { CfgMsg += "CONFIGURATION ERROR: Invalid or unsuported BitDepth value\n"; }
  if (StartFrame[0]<0 || StartFrame[1]<0) { CfgMsg += "CONFIGURATION ERROR: StartFrame value cannot be negative \n"; }
  if (!CfgMsg.empty()) { xCfgINI::printErrorMessage(std::string("! Invalid parameters\n") + CfgMsg, HelpString); return EXIT_FAILURE; }


  //check weights
  if (!xc_USE_RUNTIME_CMPWEIGHTS && ComponentWeights != xIVPSNR::c_DefaultCmpWeights)
  {
    fmt::printf("CONFIGURATION ERROR: Config parameter ComponentWeights is different than default one but Software was build with USE_RUNTIME_CMPWEIGHTS option disabled. To enable run-time ComponentWeights selection build the software with USE_RUNTIME_CMPWEIGHTS option enabled.\n\n");
    return EXIT_FAILURE;
  }

  //check conformance
  if(SearchRange != xIVPSNR::c_DefaultSearchRange)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with SearchRange different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default range is DefaultSearchRange=%d.\n\n", xIVPSNR::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && ComponentWeights != xIVPSNR::c_DefaultCmpWeights)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with ComponentWeights different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default weights are DefaultCmpWeights=%s.\n\n", xString::formatIntWeights(xIVPSNR::c_DefaultCmpWeights));
  }
  if(UnnoticeableCoef != xIVPSNR::c_DefaultUnntcbCoef)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with UnnoticeableCoef different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default coeffs are DefaultUnnoticeableCoef=%s.\n\n", xString::formatFltWeights(xIVPSNR::c_DefaultUnntcbCoef));
  }

  //check performance
  if(SearchRange > xIVPSNR::c_DefaultSearchRange)
  {
    fmt::printf("PERFORMANCE WARNING: Software was executed with SearchRange wider than default one. This leads to higher computational complexity and longer calculation time. The default range is DefaultSearchRange=%d.\n\n", xIVPSNR::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && ComponentWeights == xIVPSNR::c_DefaultCmpWeights && !X_SSE_ALL)
  {
    fmt::printf("PERFORMANCE WARNING: Software was build with USE_RUNTIME_CMPWEIGHTS option enabled and default weights was selected. To speed up computation of IV-PSNR with default weights - dissable USE_RUNTIME_CMPWEIGHTS option.\n\n");
  }

  fmt::printf("\n\n\n");


  

  //==============================================================================
  //preparation
  if(VerboseLevel >= 2) { fmt::printf("Initializing:\n"); }

  //file size
  int64 SizeOfInputFile[NumInputsMax] = { 0 };
  for(int32 i = 0; i < 2; i++)
  {
    if(!xFile::exist(InputFile[i])) { xPrintError(fmt::sprintf("ERROR --> InputFile does not exist (%s)", InputFile[i])); return EXIT_FAILURE; }
    SizeOfInputFile[i] = xFile::filesize(InputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("SizeOfInputFile%d = %d\n", i, SizeOfInputFile[i]); }
  }

  if(UseMask)
  {
    if(!xFile::exist(InputFile[2])) { xPrintError(fmt::sprintf("ERROR --> InputFile does not exist (%s)", InputFile[2])); return EXIT_FAILURE; }
    SizeOfInputFile[2] = xFile::filesize(InputFile[2]);
    if(VerboseLevel >= 1) { fmt::printf("SizeOfInputFileM = %d\n", SizeOfInputFile[2]); }
  }

  //num of frames
  int32 NumOfFrames[NumInputsMax] = { 0 };
  for(int32 i = 0; i < 2; i++)
  {
    NumOfFrames[i] = xSeq::calcNumFramesInFile(PictureSize, BitDepth, ChromaFormat, SizeOfInputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("DetectedFrames%d  = %d\n", i, NumOfFrames[i]); }
    if(StartFrame[i] >= NumOfFrames[i]) { xPrintError(fmt::sprintf("ERROR --> StartFrame%d >= DetectedFrames%d for (%s)", i, i, InputFile[i])); return EXIT_FAILURE; }
  }

  if(UseMask)
  {
    NumOfFrames[2] = xSeq::calcNumFramesInFile(PictureSize, BitDepthM, ChromaFormatM, SizeOfInputFile[2]);
    if(VerboseLevel >= 1) { fmt::printf("DetectedFramesM  = %d\n", NumOfFrames[2]); }
    for(int32 i = 0; i < 2; i++) { if(StartFrame[i] != 0) { xPrintError(fmt::sprintf("ERROR --> StartFrame%d != 0 in not supported in masked mode", i)); return EXIT_FAILURE; } }
  }

  int32 MinSeqNumFrames = xMin(NumOfFrames[0], NumOfFrames[1]);
  int32 MinSeqRemFrames = xMin(NumOfFrames[0] - StartFrame[0], NumOfFrames[1] - StartFrame[1]);
  int32 NumFrames       = xMin(NumberOfFrames > 0 ? NumberOfFrames : MinSeqNumFrames, MinSeqRemFrames);
  int32 FirstFrame[3] = { 0 };
  for(int32 i = 0; i < 2; i++) { FirstFrame[i] = xMin(StartFrame[i], NumOfFrames[i] - 1); }
  if(VerboseLevel >= 1) { fmt::printf("FramesToProcess  = %d\n", NumFrames); }
  fmt::printf("\n");

  if(!InputFile[2].empty() && (NumFrames > NumOfFrames[2])) { xPrintError(fmt::sprintf("ERROR --> FramesToProcess > NumOfFramesM")); return EXIT_FAILURE; }
  
  const int32 BDs[NumInputsMax] = { BitDepth    , BitDepth    , BitDepthM };
  const int32 CFs[NumInputsMax] = { ChromaFormat, ChromaFormat, ChromaFormatM };
  std::vector<xSeq> Sequence(NumInputsCur);
  for(int32 i = 0; i < NumInputsCur; i++) { Sequence[i].create(PictureSize, BDs[i], CFs[i]); }
  std::vector<xPicP> PictureP(NumInputsCur);
  for(int32 i = 0; i < NumInputsCur; i++) { PictureP[i].create(PictureSize, BDs[i], PictureMargin); }
  std::vector<xPicI> PictureI(2);
  if (InterleavedPic && CalcIVPSNR) { for (int32 i = 0; i < 2; i++) { PictureI[i].create(PictureSize, BitDepth, PictureMargin); } }

  for(int32 i = 0; i < NumInputsCur; i++)
  {
    bool OpenSucces = (bool)(Sequence[i].openFile(InputFile[i], xSeq::eMode::Read));
    if(!OpenSucces) { xPrintError(fmt::sprintf("ERROR --> InputFile opening failure (%s)", InputFile[i])); return EXIT_FAILURE; }
    if(FirstFrame[i] != 0) { Sequence[i].seekFrame(FirstFrame[i]); }
  }

  xThreadPool*         ThreadPool = nullptr;
  xThreadPoolInterface ThreadPoolIf;
  if(NumberOfThreadsUsed > 0)
  { 
    ThreadPool = new xThreadPool;
    ThreadPool->create(NumberOfThreadsUsed, PictureHeight+1);
    ThreadPoolIf.init(ThreadPool, 2);
  }  

  xTIVPSNR Processor;
  Processor.setLegacyWS8bit(Legacy8bitWSPSNR);
  Processor.setSearchRange (SearchRange     );
  Processor.setCmpWeights  (ComponentWeights);
  Processor.setUnntcbCoef  (UnnoticeableCoef);
  if(NumberOfThreadsUsed > 0) { Processor.initThreadPool(ThreadPool, PictureHeight); }
  Processor.init(PictureHeight);
  if(IsEquirectangular) { Processor.initWS(true, PictureWidth, PictureHeight, BitDepth, LonRangeDeg, LatRangeDeg); }

  //IVPSNR debug data
  int32V4 LastGCS = xMakeVec4(0);
  flt64   LastR2T = 0;
  flt64   LastT2R = 0;
  int32   NumNonMasked = 0;
  if(VerboseLevel >= 4)
  {
    Processor.setDebugCallbackGCS([&LastGCS          ](const int32V4& GCS  ) { LastGCS = GCS;                });
    Processor.setDebugCallbackQAP([&LastR2T, &LastT2R](flt64 R2T, flt64 T2R) { LastR2T = R2T; LastT2R = T2R; });
    Processor.setDebugCallbackMSK([&NumNonMasked     ](int32 NNM           ) { NumNonMasked = NNM;           });
  }

  //==============================================================================
  //running
  if(VerboseLevel >= 2) { fmt::printf("Running:\n"); }
  tTimePoint ProcessingBeg = tClock::now();

  tDuration Duration__Load = tDuration(0);
  tDuration Duration__Prep = tDuration(0);
  tDuration Duration__PSNR = tDuration(0);
  tDuration DurationWSPSNR = tDuration(0);
  tDuration DurationIVPSNR = tDuration(0);
  tDuration DurationFlowCalc = tDuration(0);
  tDuration DurationCalcFlow = tDuration(0);
  tDuration DurationIVPSNRFlowCheck = tDuration(0);
  tDuration DurationPSNRFlow = tDuration(0);
  tDuration DurationIVPSNRFlow = tDuration(0);
  tDuration DurationIVPSNROnlyFlow = tDuration(0);

  std::vector<flt64> Frame__PSNR[4];
  std::vector<flt64> FrameWSPSNR[4];
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++)
  {
    Frame__PSNR[CmpIdx].resize(NumFrames);
    FrameWSPSNR[CmpIdx].resize(NumFrames);
  }
  std::vector<flt64> FrameIVPSNR(NumFrames);
  std::vector<flt64> FrameR2T(VerboseLevel >= 4 ? NumFrames : 0);
  std::vector<flt64> FrameT2R(VerboseLevel >= 4 ? NumFrames : 0);

  std::vector<flt64> FrameIVPSNRFlowCheck(NumFrames);
  std::vector<flt64> FramePSNRFlow(NumFrames);
  std::vector<flt64> FrameIVPSNRFlow(NumFrames);
  std::vector<flt64> FrameIVPSNROnlyFlow(NumFrames);

  bool AllExact = true;
  bool AnyFake  = false;

  cv::Mat prev[2];
  cv::Mat next[2];

  for(int32 f = 0; f < NumFrames; f++)
  {
    tTimePoint T0 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    std::vector<bool> ReadOK(NumInputsCur, true);
    if(ThreadPoolIf.isActive())
    {
      for(int32 i = 0; i < NumInputsCur; i++) { ThreadPoolIf.addWaitingTask([&Sequence, &PictureP, &ReadOK, i](int32 /*ThreadIdx*/) { ReadOK[i] = (bool)Sequence[i].readFrame(&(PictureP[i])); }); }
      ThreadPoolIf.waitUntilTasksFinished(NumInputsCur);
    }
    else
    {
      for(int32 i = 0; i < NumInputsCur; i++) { ReadOK[i] = (bool)(Sequence[i].readFrame(&(PictureP[i]))); }
    }
    for(int32 i = 0; i < NumInputsCur; i++) { if(!ReadOK[i]) { xPrintError(fmt::sprintf("ERROR --> InputFile read error (%s)", InputFile[i])); return EXIT_FAILURE; } }

    tTimePoint T1 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    std::vector<bool> CheckOK(NumInputsCur, true);
    if(ThreadPoolIf.isActive())
    {
      for(int32 i = 0; i < NumInputsCur; i++)
      {
        ThreadPoolIf.addWaitingTask(
          [&PictureP, &PictureI, &CheckOK, &InputFile, InterleavedPic, i](int32 /*ThreadIdx*/)
          {
            CheckOK[i] = PictureP[i].check(InputFile[i]);
            PictureP[i].extend();
            if(InterleavedPic && i<2) { PictureI[i].rearrangeFromPlanar(&PictureP[i]); }
          }
        );
      }
      ThreadPoolIf.waitUntilTasksFinished(NumInputsCur);
    }
    else
    {
      for(int32 i = 0; i < NumInputsCur; i++)
      {
        CheckOK[i] = PictureP[i].check(InputFile[i]);
        PictureP[i].extend();
        if(InterleavedPic && i < 2) { PictureI[i].rearrangeFromPlanar(&PictureP[i]); }
      }
    }
    

    tTimePoint T2 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(Calc__PSNR)
    {
      flt64V4 PSNR  = xMakeVec4(0.0  );
      boolV4  Exact = xMakeVec4(false);
      if(UseMask) { std::tie(PSNR, Exact) = Processor.calcPicPSNRM(&PictureP[0], &PictureP[1], &PictureP[2]); }
      else        { std::tie(PSNR, Exact) = Processor.calcPicPSNR (&PictureP[0], &PictureP[1]              ); }
      
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        if(Exact[CmpIdx]) { AnyFake  = true ; }
        else              { AllExact = false; }
        Frame__PSNR[CmpIdx][f] = PSNR[CmpIdx];        
      }

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d   PSNR%s %8.4f %8.4f %8.4f", f, Suffix, PSNR[0], PSNR[1], PSNR[2]);
        if(Exact[0]) { fmt::printf(" ExactY"); }
        if(Exact[1]) { fmt::printf(" ExactU"); }
        if(Exact[2]) { fmt::printf(" ExactV"); }
        if(VerboseLevel >= 4 && UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        fmt::printf("\n");
      }
    }

    tTimePoint T3 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(CalcWSPSNR)
    {
      flt64V4 WSPSNR = xMakeVec4(0.0  );
      boolV4  Exact  = xMakeVec4(false);

      if(UseMask) { std::tie(WSPSNR, Exact) = Processor.calcPicWSPSNRM(&PictureP[0], &PictureP[1], &PictureP[2]); }
      else        { std::tie(WSPSNR, Exact) = Processor.calcPicWSPSNR (&PictureP[0], &PictureP[1]              ); }

      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        FrameWSPSNR[CmpIdx][f] = WSPSNR[CmpIdx];
      }

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d WSPSNR%s %8.4f %8.4f %8.4f", f, Suffix, WSPSNR[0], WSPSNR[1], WSPSNR[2]);
        if(Exact[0]) { fmt::printf(" ExactY"); }
        if(Exact[1]) { fmt::printf(" ExactU"); }
        if(Exact[2]) { fmt::printf(" ExactV"); }
        if(VerboseLevel >= 4 && UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        fmt::printf("\n");
      }
    }

    tTimePoint T4 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(CalcIVPSNR)
    {
      flt64 IVPSNR = 0.0;
      if(!InputFile[2].empty())
      {
        IVPSNR = Processor.calcPicIVPSNRM(&PictureP[0], &PictureP[1], &PictureP[2], &PictureI[0], &PictureI[1]);
      }
      else
      {
        if  (InterleavedPic) { IVPSNR = Processor.calcPicIVPSNR(&PictureP[0], &PictureP[1], &PictureI[0], &PictureI[1]); }
        else                 { IVPSNR = Processor.calcPicIVPSNR(&PictureP[0], &PictureP[1]                            ); }
      }
      FrameIVPSNR[f] = IVPSNR;

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d IVPSNR%s %8.4f", f, Suffix, IVPSNR);
        if(VerboseLevel >= 4)
        { 
          FrameR2T[f] = LastR2T;
          FrameT2R[f] = LastT2R;
          fmt::printf("   GCS %d %d %d    R2T %7.4f  T2R %7.4f", LastGCS[0], LastGCS[1], LastGCS[2], LastR2T, LastT2R);
          if(UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        }
        fmt::printf("\n");
      }
    }

    tTimePoint T5 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    /*=============*/
    /*OPTICAL FLOW*/
    /*=============*/

    tTimePoint T6 = tTimePoint::min();
    tTimePoint T7 = tTimePoint::min();
    tTimePoint T8 = tTimePoint::min();
    tTimePoint T9 = tTimePoint::min();
    tTimePoint T10 = tTimePoint::min();

    if (CalcCheckFlow || CalcPSNRFlow || CalcIVPSNRFlow || CalcIVPSNRFlowOnly) {
        flt64 IVPSNRFlowCheck = 0.0;
        flt64 PSNRFlow = 0.0;
        flt64 IVPSNRFlow = 0.0;
        flt64 IVPSNROnlyFlow = 0.0;
        std::vector<xPlane<flt32V2>>flowPlane(2);
        for (int32 i = 0; i < NumInputsCur; i++) { flowPlane[i].create(PictureSize, BitDepth, PictureMargin); }

        double pyr_scale = 0.5;
        int levels = 2;
        int winsize = 10;
        int iterations = 2;
        int poly_n = 5;
        double poly_sigma = 1.2;

        if (f == 0) {

            prev[0] = cv::Mat(PictureHeight, PictureWidth, CV_16UC1);
            prev[1] = cv::Mat(PictureHeight, PictureWidth, CV_16UC1);

            next[0] = cv::Mat(PictureHeight, PictureWidth, CV_16UC1);
            next[1] = cv::Mat(PictureHeight, PictureWidth, CV_16UC1);

            xUtilsOCV::xPic2Mat(PictureP[0], prev[0], 1);
            xUtilsOCV::xPic2Mat(PictureP[1], prev[1], 1);
            FrameIVPSNRFlowCheck[f] = 0.0;
            FramePSNRFlow[f] = 0.0;
            FrameIVPSNRFlow[f] = 0.0;
            FrameIVPSNROnlyFlow[f] = 0.0;
        }
        else {
            
            cv::Mat flow[2];
            if (ThreadPoolIf.isActive())
            {
                for (int32 i = 0; i < 2; i++) {
                    ThreadPoolIf.addWaitingTask([&prev, &next, &flow, &PictureP, &flowPlane, &pyr_scale, &levels, &winsize, &iterations, &poly_n, &poly_sigma, i](int32 /*ThreadIdx*/) {
                        xUtilsOCV::xPic2Mat(PictureP[i], next[i], 1);
                        flow[i] = (prev[i].size(), CV_32FC2);
                        cv::calcOpticalFlowFarneback(prev[i], next[i], flow[i], pyr_scale, levels, winsize, iterations, poly_n, poly_sigma, 0);
                        xUtilsOCV::Mat2xPlane(flow[i], flowPlane[i]);
                        flowPlane[i].extend();
                        });
                }
                ThreadPoolIf.waitUntilTasksFinished(2);
            }
            else
            {
                for (int32 i = 0; i < 2; i++) {
                    xUtilsOCV::xPic2Mat(PictureP[i], next[i], 1);
                    flow[i] = (prev[i].size(), CV_32FC2);
                    cv::calcOpticalFlowFarneback(prev[i], next[i], flow[i], pyr_scale, levels, winsize, iterations, poly_n, poly_sigma, 0);
                    xUtilsOCV::Mat2xPlane(flow[i], flowPlane[i]);
                    flowPlane[i].extend();
                }
            }

            T6 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

            if (CalcCheckFlow) {
                IVPSNRFlowCheck = Processor.calcPicIVPSNRFlowCheck(&PictureP[0], &PictureP[1], &flowPlane[0], &flowPlane[1]);
                FrameIVPSNRFlowCheck[f] = IVPSNRFlowCheck;
                if (VerboseLevel >= 2) {
                    fmt::printf("Frame %08d IV-PSNR-Flow-Check %8.4f", f, IVPSNRFlowCheck);
                    fmt::printf("\n");
                }
            }

            T7 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

            if (CalcPSNRFlow) {
                PSNRFlow = Processor.calcPicPSNRFlow(&flowPlane[0], &flowPlane[1]);
                FramePSNRFlow[f] = PSNRFlow;
                if (VerboseLevel >= 2) {
                    fmt::printf("Frame %08d PSNR-Flow %8.4f", f, PSNRFlow);
                    fmt::printf("\n");
                }
            }

            T8 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

            if (CalcIVPSNRFlow) {
                IVPSNRFlow = Processor.calcPicIVPSNRFlowUse(&PictureP[0], &PictureP[1], &flowPlane[0], &flowPlane[1]);
                FrameIVPSNRFlow[f] = IVPSNRFlow;
                if (VerboseLevel >= 2) {
                    fmt::printf("Frame %08d IV-PSNR-Flow %8.4f", f, IVPSNRFlow);
                    fmt::printf("\n");
                }
            }

            T9 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

            if (CalcIVPSNRFlowOnly) {
                IVPSNROnlyFlow = Processor.calcPicIVPSNROnlyFlow(&flowPlane[0], &flowPlane[1]);
                FrameIVPSNROnlyFlow[f] = IVPSNROnlyFlow;
                if (VerboseLevel >= 2) {
                    fmt::printf("Frame %08d IV-PSNR-Only-Flow %8.4f", f, IVPSNROnlyFlow);
                    fmt::printf("\n");
                }
            }

            T10 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
        }
    }

    Duration__Load += (T1 - T0);
    Duration__Prep += (T2 - T1);
    Duration__PSNR += (T3 - T2);
    DurationWSPSNR += (T4 - T3);
    DurationIVPSNR += (T5 - T4);
    DurationCalcFlow += (T6 - T5);
    DurationIVPSNRFlowCheck += (T7 - T6);
    DurationPSNRFlow += (T8 - T7);
    DurationIVPSNRFlow += (T9 - T8);
    DurationIVPSNROnlyFlow += (T10 - T9);
  }
  
  //==============================================================================
  //finalizing
  
  //summary
  flt64V4 Sum__PSNR = xMakeVec4(0.0);
  flt64V4 SumWSPSNR = xMakeVec4(0.0);

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    Sum__PSNR[CmpIdx] = xPSNR::Accumulate(Frame__PSNR[CmpIdx]);
    SumWSPSNR[CmpIdx] = xPSNR::Accumulate(FrameWSPSNR[CmpIdx]);
  }
  flt64 SumIVPSNR = xPSNR::Accumulate(FrameIVPSNR);
  flt64 SumIVPSNRFlowCheck = xPSNR::Accumulate(FrameIVPSNRFlowCheck);
  flt64 Sum__PSNRFlow = xPSNR::Accumulate(FramePSNRFlow);
  flt64 SumIVPSNRFlow = xPSNR::Accumulate(FrameIVPSNRFlow);
  flt64 SumIVPSNROnlyFlow = xPSNR::Accumulate(FrameIVPSNROnlyFlow);

  flt64V4 Avg__PSNR = Sum__PSNR / NumFrames;
  flt64V4 AvgWSPSNR = SumWSPSNR / NumFrames;
  flt64   AvgIVPSNR = SumIVPSNR / NumFrames;
  flt64   AvgIVPSNRFlowCheck = SumIVPSNRFlowCheck / (NumFrames - 1);
  flt64   Avg__PSNRFlow = Sum__PSNRFlow / (NumFrames-1);
  flt64   AvgIVPSNRFlow = SumIVPSNRFlow / (NumFrames - 1);
  flt64   AvgIVPSNROnlyFlow = SumIVPSNROnlyFlow / (NumFrames - 1);

  tTimePoint  ProcessingEnd  = tClock::now();

  //cleanup
  for(int32 i = 0; i < 2; i++) { Sequence[i].closeFile(); }
  for(int32 i = 0; i < 2; i++) { Sequence[i].destroy(); }
  for(int32 i = 0; i < 2; i++) { PictureP[i].destroy(); }
  if (InterleavedPic) { for(int32 i = 0; i < 2; i++) { PictureI[i].destroy(); } }
  if(ThreadPool) { ThreadPool->destroy(); }

  //output file
  if(!OutputFile.empty())
  {
    std::ofstream OutputStream(OutputFile, std::ios::app);
    std::time_t   TimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    OutputStream                  << fmt::sprintf("FILE0  \"%s\"\n", InputFile[0]);
    OutputStream                  << fmt::sprintf("FILE1  \"%s\"\n", InputFile[1]);
    OutputStream                  << fmt::format ("TIME   {:%Y-%m-%d  %H:%M:%S}\n", fmt::localtime(TimeStamp));
    
    if(Calc__PSNR)                  { OutputStream << fmt::sprintf("PSNR%s           %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2]); }
    if(CalcWSPSNR)                  { OutputStream << fmt::sprintf("WSPSNR%s         %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, AvgWSPSNR[0], AvgWSPSNR[1], AvgWSPSNR[2]); }
    if(CalcIVPSNR)                  { OutputStream << fmt::sprintf("IVPSNR%s         %8.4f dB                    \n", Suffix, AvgIVPSNR                               ); }
    if(CalcCheckFlow)               { OutputStream << fmt::sprintf("IVPSNRCheck%s    %8.4f\n",Suffix, AvgIVPSNRFlowCheck); }
    if(CalcPSNRFlow)                { OutputStream << fmt::sprintf("PSNRFlow%s       %8.4f\n",Suffix, Avg__PSNRFlow); }
    if(CalcPSNRFlow && Calc__PSNR)  { OutputStream << fmt::sprintf("PSNRWFlow%s      %8.4f dB  %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2], Avg__PSNRFlow); }
    if(CalcIVPSNRFlow)              { OutputStream << fmt::sprintf("IVPSNRFlow%s     %8.4f dB                \n", Suffix, AvgIVPSNRFlow); }
    if(CalcIVPSNRFlowOnly)          { OutputStream << fmt::sprintf("IVPSNROnlyFlow%s %8.4f dB                \n", Suffix, AvgIVPSNROnlyFlow); }
    OutputStream.close();
  }

  //==============================================================================
  //printout results
  fmt::printf("\n\n");
  
  if(Calc__PSNR)                    { fmt::printf("Average          PSNR%s %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2]); }
  if(CalcWSPSNR)                    { fmt::printf("Average          WSPSNR%s %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, AvgWSPSNR[0], AvgWSPSNR[1], AvgWSPSNR[2]); }
  if(CalcIVPSNR)                    { fmt::printf("Average          IVPSNR%s %8.4f dB                    \n", Suffix, AvgIVPSNR                               ); }
  if(CalcCheckFlow)                 { fmt::printf("Average          IV-PSNRCheck%s   %8.4f dB\n",Suffix, AvgIVPSNRFlowCheck); }
  if(CalcPSNRFlow)                  { fmt::printf("Average          PSNRFlow%s   %8.4f dB\n",Suffix, Avg__PSNRFlow); }
  if(CalcPSNRFlow && Calc__PSNR)    { fmt::printf("Average          PSNRWFlow%s   %8.4f dB  %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2], Avg__PSNRFlow); }
  if(CalcIVPSNRFlow)                { fmt::printf("Average          IVPSNRFlow%s %8.4f dB                \n", Suffix, AvgIVPSNRFlow); }
  if(CalcIVPSNRFlowOnly)            { fmt::printf("Average          IVPSNROnlyFlow%s %8.4f dB                \n", Suffix, AvgIVPSNROnlyFlow); }
  fmt::printf("\n");
  if(AnyFake ) { fmt::printf("FakePSNR\n"); }
  if(AllExact) { fmt::printf("ExactSequences\n"); }
  fmt::printf("\n");
  if(VerboseLevel >= 4 && CalcIVPSNR)
  {
    flt64 SumR2T = xPSNR::Accumulate(FrameR2T);
    flt64 SumT2R = xPSNR::Accumulate(FrameT2R);
    flt64 AvgR2T = SumR2T / NumFrames;
    flt64 AvgT2R = SumT2R / NumFrames;
    fmt::printf("Average        AC-R2T%s %8.4f dB                    \n", Suffix, AvgR2T);
    fmt::printf("Average        AC-T2R%s %8.4f dB                    \n", Suffix, AvgT2R);
    fmt::printf("\n");
  }
  if(VerboseLevel >= 3)
  {
    if(true      )                  { fmt::printf("AvgTime           LOAD %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__Load).count() / NumFrames); }
    if(true      )                  { fmt::printf("AvgTime           PREP %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__Prep).count() / NumFrames); }
    if(Calc__PSNR)                  { fmt::printf("AvgTime           PSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__PSNR).count() / NumFrames); }
    if(CalcWSPSNR)                  { fmt::printf("AvgTime         WSPSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationWSPSNR).count() / NumFrames); }
    if(CalcIVPSNR)                  { fmt::printf("AvgTime         IVPSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationIVPSNR).count() / NumFrames); }
    if(CalcCheckFlow || CalcPSNRFlow || CalcIVPSNRFlow || CalcIVPSNRFlowOnly)             
								    { fmt::printf("AvgTime       CalcFlow %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationFlowCalc).count() / NumFrames); }
    if(CalcPSNRFlow)              	{ fmt::printf("AvgTime       PSNRFlow %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationFlowCalc).count() / NumFrames); }
    if(CalcIVPSNRFlow)            	{ fmt::printf("AvgTime     IVPSNRFlow %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationFlowCalc).count() / NumFrames); }
    if(CalcIVPSNRFlowOnly)        	{ fmt::printf("AvgTime IVPSNRFlowOnly %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationFlowCalc).count() / NumFrames); }
  }
  fmt::printf("\n");
  fmt::printf("TotalTime %.2f s\n", std::chrono::duration_cast<tDurationS>(ProcessingEnd - ProcessingBeg).count());
  fmt::printf("NumFrames %d\n", NumFrames);
  fmt::printf("END-OF-LOG\n");
  fflush(stdout);
  
  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
