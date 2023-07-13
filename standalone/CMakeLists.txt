cmake_minimum_required(VERSION 3.14...3.22)

project(AvaritiaLauncher LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)
CPMAddPackage("gh:nlohmann/json@3.10.5")
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

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "Avaritia")

target_link_libraries(${PROJECT_NAME} Avaritia::Avaritia cxxopts)
