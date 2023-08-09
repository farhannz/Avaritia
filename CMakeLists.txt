cmake_minimum_required(VERSION 3.24)
project(Avaritia)
include(FetchContent)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")

include_directories(include PUBLIC)
include_directories(submodules/imgui PUBLIC)
include_directories(submodules/imgui/backends PUBLIC)
include_directories(submodules/stb PUBLIC)
include_directories(submodules/ImGui-Addons/FileBrowser PUBLIC)
# target_link_libraries(Avaritia nlohmann_json)


add_subdirectory("src")