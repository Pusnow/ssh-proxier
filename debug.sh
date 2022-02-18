#!/bin/sh
set -ex
export PATH="/usr/local/opt/llvm/bin:$PATH"
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang \
    -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ \
    -DCMAKE_OBJC_COMPILER=/usr/local/opt/llvm/bin/clang \
    -DCMAKE_OBJCXX_COMPILER=/usr/local/opt/llvm/bin/clang++
make VERBOSE=1
