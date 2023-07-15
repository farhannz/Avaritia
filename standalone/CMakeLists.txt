cmake_minimum_required(VERSION 3.14...3.22)

project(AvaritiaLauncher LANGUAGES CXX)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_FLAGS "-Wl,-subsystem,windows")

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)
CPMAddPackage(
  NAME nlohmann_json
  VERSION 3.11.2
  GITHUB_REPOSITORY nlohmann/json
  OPTIONS
    "JSON_BuildTests OFF"
)
CPMAddPackage("gh:gabime/spdlog@1.12.0")
CPMAddPackage("gh:libcpr/cpr#1.10.4")

CPMAddPackage(
        NAME GLFW
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.2
        OPTIONS
          "GLFW_BUILD_TESTS OFF"
          "GLFW_BUILD_EXAMPLES OFF"
          "GLFW_BULID_DOCS OFF"
)
CPMAddPackage(
  NAME fmt
  GIT_TAG 9.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)
CPMAddPackage(NAME Avaritia SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
# add_library(Avaritia SHARED IMPORTED)
# set_property(TARGET Avaritia PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/libAvaritia.dll")
# set_property(TARGET Avaritia PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../lib/libAvaritia.dll.a")

file(COPY source/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
# ---- Create standalone executable ----
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
include(CMakePrintHelpers)
cmake_print_variables(${sources})

add_executable(${PROJECT_NAME} ${sources})


add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/../build/libAvaritia.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path

        add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/../bin"      # <--this is in-file
        $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path

# add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
#         "${PROJECT_SOURCE_DIR}/build/libAvaritia.dll.a"      # <--this is in-file
#         $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "Avaritia")

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json cpr::cpr spdlog::spdlog Avaritia cxxopts)


