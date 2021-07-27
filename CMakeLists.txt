cmake_minimum_required(VERSION 3.10)

project(bmusp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(raylib REQUIRED)
find_package(SDL2 REQUIRED)

file(COPY assets DESTINATION .)
add_executable( bmusp
                bmusp.cpp
                player.cpp
                library.cpp
                libs/tinyfiledialogs.c)
target_link_libraries(bmusp raylib tag SDL2 SDL2_mixer)