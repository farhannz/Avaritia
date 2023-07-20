cmake_minimum_required(VERSION 3.14...3.22)

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

set(CMAKE_CXX_FLAGS "-Wl,-subsystem,windows")

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

add_library(Avaritia SHARED IMPORTED)
add_library(cpr SHARED IMPORTED)
add_library(fmt SHARED IMPORTED)
set_property(TARGET Avaritia PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/")
set_property(TARGET Avaritia PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../lib/libAvaritia.dll.a")
set_property(TARGET cpr PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../bin/")
set_property(TARGET cpr PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../lib/libcpr.dll.a")
set_property(TARGET fmt PROPERTY IMPORTED_LOCATION "${VCPKG_INSTALLED_DIR}/bin/")
set_property(TARGET fmt PROPERTY IMPORTED_IMPLIB "${VCPKG_INSTALLED_DIR}/lib/libfmt.dll.a")



include(CMakePrintHelpers)
# cmake_print_variables(${stb_SOURCE_DIR})
file(GLOB ThirdPartyDLLS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/../bin/*.dll")
add_executable(${PROJECT_NAME} ${sources})

foreach(_files  ${ThirdPartyDLLS})
  cmake_print_variables(${_files})
  add_custom_command(TARGET AvaritiaLauncher POST_BUILD        # Adds a post-build event to MyTest
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
  "${_files}"      # <--this is in-file
  $<TARGET_FILE_DIR:AvaritiaLauncher>)                 # <--this is out-file path
endforeach(_files)

file(COPY source/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)


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

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source/main.rc")

target_link_libraries(${PROJECT_NAME} PUBLIC cpr nfd fmt Avaritia )


