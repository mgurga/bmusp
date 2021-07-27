cmake_minimum_required(VERSION 3.11)
include(FetchContent)

project(bmusp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(raylib REQUIRED)
find_package(SDL2 REQUIRED)
message("fetching tinyfiledialogs")
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY "https://github.com/native-toolkit/tinyfiledialogs.git"
    GIT_TAG        "cc6b593c029110af8045826ce691f540c85e850c"
)
FetchContent_MakeAvailable(tinyfiledialogs)
include_directories(${tinyfiledialogs_SOURCE_DIR})

file(COPY assets DESTINATION .)
add_executable( ${PROJECT_NAME}
                bmusp.cpp
                player.cpp
                library.cpp
                ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c)
target_link_libraries(${PROJECT_NAME} raylib tag SDL2 SDL2_mixer)