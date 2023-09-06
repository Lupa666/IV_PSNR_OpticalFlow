cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(PROJECT_NAME "IV_PSNR")
project(${PROJECT_NAME})

# force static runtime libraries for msvc builds
if(MSVC)
  set(variables CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
  foreach(variable ${variables})
    if(${variable} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
    endif()
  endforeach()
endif()

# set c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# compile everything position independent (even static libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# set verbose compile options
#set(CMAKE_VERBOSE_MAKEFILE ON)

# set lto
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_is_supported OUTPUT output)
if(lto_is_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# assuming x86-64 Microarchitecture Feature Level >= x86-64-v2
if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
  if((CMAKE_C++_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
  elseif(CMAKE_C++_COMPILER_ID STREQUAL "Clang" AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
  elseif(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=nehalem")
  endif()
endif()

# add OpenMP - since v3.0 OpenMP is no longer used
# find_package(OpenMP)
# if (OPENMP_FOUND)
#     set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#     set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
# endif()

# add threads lib
find_package (Threads REQUIRED)

#=========================================================================================================================================
# formatting library {fmt} (https://github.com/fmtlib/fmt) - headers only
#=========================================================================================================================================
set(LIB_FMT_LOCATION "src/lib_fmt")

#=========================================================================================================================================
# PUT Multimedia Building Blocks library
#=========================================================================================================================================
set(LIB_PMBB_NAME "lib_pmbb")
set(LIB_PMBB_LOCATION "src/lib_pmbb")
set(LIB_PMBB_SOURCES
  ${LIB_PMBB_LOCATION}/xCommonDefPMBB.h
  ${LIB_PMBB_LOCATION}/xVec.h
  ${LIB_PMBB_LOCATION}/xFile.h
  ${LIB_PMBB_LOCATION}/xString.h       ${LIB_PMBB_LOCATION}/xString.cpp
  ${LIB_PMBB_LOCATION}/xCfgINI.h       ${LIB_PMBB_LOCATION}/xCfgINI.cpp
  ${LIB_PMBB_LOCATION}/xEvent.h
  ${LIB_PMBB_LOCATION}/xQueue.h
  ${LIB_PMBB_LOCATION}/xThreadPool.h   ${LIB_PMBB_LOCATION}/xThreadPool.cpp
  ${LIB_PMBB_LOCATION}/xPic.h          ${LIB_PMBB_LOCATION}/xPic.cpp
  ${LIB_PMBB_LOCATION}/xSeq.h          ${LIB_PMBB_LOCATION}/xSeq.cpp
  ${LIB_PMBB_LOCATION}/xPixelOps.h  
  ${LIB_PMBB_LOCATION}/xPixelOpsSTD.h  ${LIB_PMBB_LOCATION}/xPixelOpsSTD.cpp
  ${LIB_PMBB_LOCATION}/xPixelOpsSSE.h  ${LIB_PMBB_LOCATION}/xPixelOpsSSE.cpp
  ${LIB_PMBB_LOCATION}/xPixelOpsAVX.h  ${LIB_PMBB_LOCATION}/xPixelOpsAVX.cpp
  ${LIB_PMBB_LOCATION}/xDistortion.h  
  ${LIB_PMBB_LOCATION}/xDistortionSTD.h  ${LIB_PMBB_LOCATION}/xDistortionSTD.cpp
  ${LIB_PMBB_LOCATION}/xDistortionSSE.h  ${LIB_PMBB_LOCATION}/xDistortionSSE.cpp
  ${LIB_PMBB_LOCATION}/xDistortionAVX.h  ${LIB_PMBB_LOCATION}/xDistortionAVX.cpp
  ${LIB_PMBB_LOCATION}/xMathUtils.h      ${LIB_PMBB_LOCATION}/xMathUtils.cpp
)
source_group("Source Files" FILES ${LIB_PMBB_SOURCES})
add_library(${LIB_PMBB_NAME} ${LIB_PMBB_SOURCES})
target_include_directories(${LIB_PMBB_NAME} PRIVATE ${LIB_FMT_LOCATION})

#=========================================================================================================================================
# IV-PSNR
#=========================================================================================================================================
set(PROJECT_LOCATION "src/IVPSNR")
set(PROJECT_SOURCES  
  ${PROJECT_LOCATION}/xCommonDefIVPSNR.h
  ${PROJECT_LOCATION}/xPSNR.h      ${PROJECT_LOCATION}/xPSNR.cpp
  ${PROJECT_LOCATION}/xWSPSNR.h    ${PROJECT_LOCATION}/xWSPSNR.cpp
  ${PROJECT_LOCATION}/xPlane.h     ${PROJECT_LOCATION}/xPlane.cpp
  ${PROJECT_LOCATION}/xIVPSNR.h    ${PROJECT_LOCATION}/xIVPSNR.cpp ${PROJECT_LOCATION}/xIVPSNRM.cpp
  ${PROJECT_LOCATION}/main.cpp
)

source_group("Source Files" FILES ${PROJECT_SOURCES})
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_FMT_LOCATION})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_PMBB_LOCATION})
target_link_libraries (${PROJECT_NAME} PRIVATE ${LIB_PMBB_NAME} Threads::Threads)

#=========================================================================================================================================


