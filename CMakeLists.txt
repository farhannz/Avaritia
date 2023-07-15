cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Avaritia
  VERSION 0.1
  LANGUAGES CXX
)



# ---- Include guards ----
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
message(
  FATAL_ERROR
  "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()


# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)
include(CMakePrintHelpers)
# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")
CPMAddPackage("gh:gabime/spdlog@1.12.0")
CPMAddPackage("gh:libcpr/cpr#1.10.4")
CPMAddPackage("gh:ocornut/imgui#docking")
CPMAddPackage("gh:juliettef/IconFontCppHeaders#90da8021ec7c7792c454c3f43516595754a91765")
CPMAddPackage("gh:nothings/stb#5736b15f7ea0ffb08dd38af21067c314d6a3aae9")
CPMAddPackage("gh:gallickgunner/ImGui-Addons#8a3c2702e0b690e46e7a21266f5d0af4b99d410d")

# GLM
CPMAddPackage(
  NAME glm
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 0.9.9.7
)

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
  NAME nlohmann_json
  VERSION 3.11.2
  GITHUB_REPOSITORY nlohmann/json
  OPTIONS
    "JSON_BuildTests OFF"
)
CPMAddPackage(
  NAME fmt
  GIT_TAG 9.1.0
GITHUB_REPOSITORY fmtlib/fmt
OPTIONS "FMT_INSTALL YES" # create an installable target
)
  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" )
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" )
cmake_print_variables(${sources})
aux_source_directory(${imgui_SOURCE_DIR} IMGUI_FILES)
list(APPEND IMGUI_FILES "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp" "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h")
set(IMGUI_BACKENDS "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp" "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
set(FILEBROWSER "${ImGui-Addons_SOURCE_DIR}/FileBrowser/ImGuiFileBrowser.cpp")
include(CMakePrintHelpers)
cmake_print_variables(IMGUI_FILES IMGUI_BACKENDS)
# set(headers "${CMAKE_CURRENT_SOURCE_DIR}/include/avaritia/crypto.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/avaritia/request.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/avaritia/request.h")
# set(sources "${CMAKE_CURRENT_SOURCE_DIR}/source/crypto.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/request.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/request.cpp" )

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME} ${headers} ${sources}  ${IMGUI_BACKENDS} ${IMGUI_FILES} ${FILEBROWSER})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt opengl32 glfw glm nlohmann_json::nlohmann_json cpr::cpr spdlog::spdlog sodium)

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
                         ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends
                         "${ImGui-Addons_SOURCE_DIR}/FileBrowser" "${ImGui-Addons_SOURCE_DIR}/FileBrowser/Dirent"
                         ${IconFontCppHeaders_SOURCE_DIR}
                         ${stb_SOURCE_DIR}
)


# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)


# packageProject(
#   NAME ${PROJECT_NAME}
#   VERSION ${PROJECT_VERSION}
#   NAMESPACE ${PROJECT_NAME}
#   BINARY_DIR ${PROJECT_BINARY_DIR}
#   INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
#   INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
#   VERSION_HEADER "${VERSION_HEADER_LOCATION}"
#   COMPATIBILITY SameMajorVersion
#   DEPENDENCIES "fmt 9.1.0;json 3.10.5;glfw;cpr 1.10.4"
# )
