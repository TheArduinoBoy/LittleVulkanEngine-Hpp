cmake -G Ninja -S. -B ./build -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build