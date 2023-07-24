cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----
if (DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
  set(VCPKG_DEFAULT_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET})
else()
  message(FATAL_ERROR "Please define VCPKG_DEFAULT_TRIPLET on your environment variable")
endif()
if (DEFINED ENV{VCPKG_TARGET_TRIPLET})
  set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_TARGET_TRIPLET})
else()
  message(FATAL_ERROR "Please define VCPKG_TARGET_TRIPLET on your environment variable")
endif()
if (DEFINED ENV{VCPKG_DEFAULT_HOST_TRIPLET})
  set(VCPKG_DEFAULT_HOST_TRIPLET $ENV{VCPKG_DEFAULT_HOST_TRIPLET})
else()
  message(FATAL_ERROR "Please define VCPKG_DEFAULT_HOST_TRIPLET on your environment variable")
endif()
# Note: update this to your new project's name and version
project(Avaritia VERSION 0.1 LANGUAGES CXX)



# ---- Include guards ----
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
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
SET(CURL_ZLIB OFF CACHE STRING "" FORCE)
# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")
CPMAddPackage("gh:juliettef/IconFontCppHeaders#90da8021ec7c7792c454c3f43516595754a91765")
CPMAddPackage("gh:gallickgunner/ImGui-Addons#8a3c2702e0b690e46e7a21266f5d0af4b99d410d")
CPMAddPackage("gh:libcpr/cpr#1.10.4")

include(CMakePrintHelpers)
cmake_print_variables(${CMAKE_CXX_COMPILER})


find_package(Stb REQUIRED)
# find_package(cpr CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
# find_package(quill CONFIG REQUIRED)
# target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR} ) 
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-Wl,-subsystem,windows -s")
endif()
# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" )
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" )


# ---- Create library ----

add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
                        #  ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends
                         "${ImGui-Addons_SOURCE_DIR}/FileBrowser" "${ImGui-Addons_SOURCE_DIR}/FileBrowser/Dirent"
                         ${IconFontCppHeaders_SOURCE_DIR}
                         ${Stb_INCLUDE_DIR}
                        #  ${stb_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
OpenGL::GL glfw glm::glm 
nlohmann_json::nlohmann_json fmt::fmt 
cpr::cpr spdlog::spdlog_header_only
imgui::imgui  unofficial-sodium::sodium 
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(${PROJECT_NAME} PUBLIC unofficial-sodium::sodium_config_public)
endif()


# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
#   COMMAND_EXPAND_LISTS
#   )

# # ---- Create an installable target ----
# # this allows users to install and find the library via `find_package()`.

# # the location where the project's version header will be placed should match the project's regular
# # header paths
# string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)