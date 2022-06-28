# CMakeList.txt : CMake project for dmg-emu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.2)

project ("dmg-emu")

include_directories("src")

add_subdirectory("src")
