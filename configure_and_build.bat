#rmdir /Q /S buildW
cmake -S ./ -B buildW -DCMAKE_BUILD_TYPE=Release
cmake --build buildW --clean-first --config Release --parallel