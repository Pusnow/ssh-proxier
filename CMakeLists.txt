cmake_minimum_required(VERSION 3.6)
project(
  ssh-proxier
  VERSION 0.1.0
  LANGUAGES C CXX OBJCXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_executable(ssh-proxier main.cpp tray.mm)

find_library(COCOA Cocoa REQUIRED)
find_library(SC SystemConfiguration REQUIRED)

target_link_libraries(ssh-proxier ${COCOA} ${SC})

set_target_properties(ssh-proxier PROPERTIES OSX_ARCHITECTURES "arm64;x86_64")
