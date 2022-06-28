cmake_minimum_required (VERSION 3.2)

set(SOURCE
	#src/main.cpp
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
)	

if (${vita})
	message(STATUS "Vita")
	add_subdirectory("vita")
else()
	message(STATUS "Not Vita")
	add_subdirectory("windows")
endif()
