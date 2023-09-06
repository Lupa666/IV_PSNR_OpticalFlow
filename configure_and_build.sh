#!/usr/bin/env bash
#rm -rf buildL
cmake -S ./ -B buildL -DCMAKE_BUILD_TYPE=Release
cmake --build buildL --clean-first --config Release --parallel
