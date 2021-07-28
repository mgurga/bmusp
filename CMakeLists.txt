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

# using my own repository because my pull request has not been accepted yet
message("fetching raygui")
FetchContent_Declare(
    raygui
    GIT_REPOSITORY "https://github.com/mgurga/raygui.git"
    GIT_TAG        "45ee3893417947138c5a40b1a428f1686caf246c"
)
FetchContent_MakeAvailable(raygui)
include_directories(${raygui_SOURCE_DIR}/src)

file(COPY assets DESTINATION .)
add_executable( ${PROJECT_NAME}
                src/bmusp.cpp
                src/player.cpp
                src/library.cpp
                ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c)
target_link_libraries(${PROJECT_NAME} raylib tag SDL2 SDL2_mixer)