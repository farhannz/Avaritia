cmake_minimum_required(VERSION 3.24)
project(main)

include_directories(../include)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.12.6.zip)
FetchContent_MakeAvailable(httplib)
FetchContent_Declare(spdlog URL https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.zip)
FetchContent_MakeAvailable(spdlog)

# if (CMAKE_BUILD_TYPE EQUAL "Release")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-subsystem,windows")
# endif (CMAKE_BUILD_TYPE EQUAL "Release")

add_executable(main main.cpp)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json httplib::httplib spdlog::spdlog)

add_subdirectory(gui)