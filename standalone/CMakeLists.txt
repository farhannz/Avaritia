cmake_minimum_required(VERSION 3.16...3.22)
cmake_policy(SET CMP0091 NEW)
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
project(AvaritiaLauncher LANGUAGES CXX)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(CURL_ZLIB OFF CACHE STRING "" FORCE)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wl,-subsystem,windows -s")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-static-libstdc++")
endif()

# # --- Import tools ----

include(../cmake/tools.cmake)

# # ---- Dependencies ----

include(../cmake/CPM.cmake)

include(CMakePrintHelpers)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../build/vcpkg_installed/${VCPKG_TARGET_TRIPLET})
  message(STATUS "Avaritia Launcher")
  include_directories(${CMAKE_CURRENT_LIST_DIR}/../build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)
  set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_LIST_DIR}/../build/vcpkg_installed/${VCPKG_TARGET_TRIPLET})
else()
  message(FATAL_ERROR "Must compile `avaritia` before compiling this launcher")
endif()
  
CPMAddPackage("gh:juliettef/IconFontCppHeaders#90da8021ec7c7792c454c3f43516595754a91765")
CPMAddPackage("gh:gallickgunner/ImGui-Addons#8a3c2702e0b690e46e7a21266f5d0af4b99d410d")
CPMAddPackage("gh:btzy/nativefiledialog-extended#v1.1.0")

include(CMakePrintHelpers)
# cmake_print_variables(Avaritia)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)
# # target_include_directories(Avaritia INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
# # # ---- Create standalone executable ----
# # # include_directories(${stb_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
list(APPEND sources "${ImGui-Addons_SOURCE_DIR}/FileBrowser/ImGuiFileBrowser.cpp")

add_library(Avaritia STATIC IMPORTED)
cmake_print_variables(${CMAKE_CXX_COMPILER_ID})
# add_library(cpr SHARED IMPORTED)
# add_library(fmt STATIC IMPORTED)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
set_property(TARGET Avaritia PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../lib/libavaritia.a")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
cmake_print_variables(${CMAKE_BUILD_TYPE})
set_target_properties(Avaritia PROPERTIES IMPORTED_LOCATION_RELEASE "${PROJECT_SOURCE_DIR}/../lib/Release/Avaritia.lib" IMPORTED_LOCATION_DEBUG "${PROJECT_SOURCE_DIR}/../lib/Debug/Avaritia.lib")
endif()
# set_property(TARGET Avaritia PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../lib/libAvaritia.a")
# set_property(TARGET cpr PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/")
# set_property(TARGET cpr PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../lib/libcpr.dll.a")
# set_property(TARGET fmt PROPERTY IMPORTED_LOCATION "${VCPKG_INSTALLED_DIR}/lib/libfmt.a")
# set_property(TARGET fmt PROPERTY IMPORTED_IMPLIB "${VCPKG_INSTALLED_DIR}/lib/libfmt.a")

SET(CURL_ZLIB OFF CACHE STRING "" FORCE)
SET(ZLIB_INCLUDE_DIR ${VCPKG_INSTALLED_DIR}/include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
SET(ZLIB_LIBRARY ${VCPKG_INSTALLED_DIR}/lib/libzlib.a)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
SET(ZLIB_LIBRARY ${VCPKG_INSTALLED_DIR}/lib/zlib.lib)
endif()
SET(CURL_INCLUDE_DIR ${VCPKG_INSTALLED_DIR}/include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
SET(CURL_LIBRARY ${VCPKG_INSTALLED_DIR}/lib/libcurl.a)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
SET(CURL_LIBRARY ${VCPKG_INSTALLED_DIR}/lib/libcurl.lib)
endif()
find_package(ZLIB)
find_package(curl CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(cpr CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(fmt CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(glfw3 CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(glm CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(imgui CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(nlohmann_json CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(spdlog CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
find_package(unofficial-sodium CONFIG REQUIRED HINTS ${VCPKG_INSTALLED_DIR})
# set(OPENGL_DIR ${VCPKG_INSTALLED_DIR})
include(CMakePrintHelpers)
# cmake_print_variables(${stb_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
file(GLOB ThirdPartyDLLS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/../bin/*.dll")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
file(GLOB ThirdPartyDLLS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}/*.dll")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  cmake_print_variables(CMAKE_CXX_COMPILER_ID)
  set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(${PROJECT_NAME} ${sources})
if(MSVC)
target_compile_options( ${PROJECT_NAME} PUBLIC "/MT$<$<CONFIG:DEBUG>:d>" "/SUBSYSTEM:windows /ENTRY:mainCRTStartup" )
endif()
foreach(_files  ${ThirdPartyDLLS})
  cmake_print_variables(${_files})
  add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
  COMMAND ${CMAKE_COMMAND} -E copy  # which executes "cmake - E copy_if_different..."
  "${_files}"      # <--this is in-file
  $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path
endforeach(_files)

# file(COPY "${PROJECT_SOURCE_DIR}/source/resources" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)


add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_directory  # which executes "cmake - E copy_if_different..."
      "${PROJECT_SOURCE_DIR}/source/resources"      # <--this is in-file
        $<TARGET_FILE_DIR:AvaritiaLauncher>/resources)                 # <--this is out-file path
# add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
#         "${PROJECT_SOURCE_DIR}/../bin/*"      # <--this is in-file
#         $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path

# # add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
# #     COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
# #         "${PROJECT_SOURCE_DIR}/build/libAvaritia.dll.a"      # <--this is in-file
# #         $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path
# add_custom_command(TARGET AvaritiaLauncher POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:AvaritiaLauncher> $<TARGET_RUNTIME_DLLS:AvaritiaLauncher>
#   COMMAND_EXPAND_LISTS
#   )


set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "Avaritia Launcher")

target_include_directories(${PROJECT_NAME} PUBLIC ${Stb_INCLUDE_DIR} "${ImGui-Addons_SOURCE_DIR}/FileBrowser" "${ImGui-Addons_SOURCE_DIR}/FileBrowser/Dirent"
${IconFontCppHeaders_SOURCE_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  cmake_print_variables(CMAKE_CXX_COMPILER_ID)
  set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
endif()

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source/main.rc")

target_link_libraries(${PROJECT_NAME} PUBLIC Avaritia nfd glfw glm::glm 
opengl32
nlohmann_json::nlohmann_json fmt::fmt 
spdlog::spdlog_header_only cpr::cpr
imgui::imgui  unofficial-sodium::sodium
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(${PROJECT_NAME} PUBLIC unofficial-sodium::sodium_config_public)
endif()
