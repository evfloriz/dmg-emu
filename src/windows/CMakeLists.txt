cmake_minimum_required (VERSION 3.2)


# move options.txt into binary dir on windows
file(COPY ${CMAKE_SOURCE_DIR}/options.txt
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/roms
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set(SDL_INCLUDE_DIRS $ENV{SDL_PATH}/include)
set(SDL_LIBRARIES $ENV{SDL_PATH}/lib/x64)

include_directories(
	${SDL_INCLUDE_DIRS}
)
link_directories(${SDL_LIBRARIES})

add_executable (${PROJECT_NAME}
	${SOURCE}
	main.cpp
)

target_link_libraries(${PROJECT_NAME} SDL2 SDL2main)