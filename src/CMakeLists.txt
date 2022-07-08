# Add source files and add platform-dependent subdirectory

cmake_minimum_required(VERSION 3.2)

# Specify all platform-independent source files
set(SOURCE
	../DMG.cpp
	../Util.cpp
	../CPU.cpp
	../PPU.cpp
	../MMU.cpp
	../APU.cpp
	../Cartridge.cpp
	../MBC.cpp
	../MBC0.cpp
	../MBC1.cpp
	../MBC3.cpp
	../main.cpp
)	

# Add platform-dependent subdirectories
if (${VITA})
	add_subdirectory("vita")
else()
	add_subdirectory("windows")
endif()
