

cmake_minimum_required(VERSION 3.24)
project(maingui)

set(IMGUI_DIR "../../submodules/imgui")
set(PARENT_DIR "../../")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wformat -Wl,-subsystem,windows -lgdi32 -limm32 ")
set(ImGuiAddons_DIR ${PARENT_DIR}/submodules/ImGui-Addons/FileBrowser)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
set(IconFont ${PARENT_DIR}/submodules/IconFontCppHeaders)

# make_directory(${CMAKE_BINARY_DIR}/resources)
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR}/bin/)

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ${PARENT_DIR}/include ${ImGuiAddons_DIR} ${ImGuiAddons_DIR}/Dirent ${IconFont})

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
# FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.12.6.zip)
# FetchContent_MakeAvailable(httplib)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba)
FetchContent_MakeAvailable(cpr)



# submodules\ImGui-Addons\FileBrowser\ImGuiFileBrowser.cpp

aux_source_directory(component COMPONENT_FILES)
aux_source_directory(${IMGUI_DIR} IMGUI_FILES)



add_executable(maingui ${COMPONENT_FILES} ${IMGUI_BACKENDS} ${IMGUI_FILES} ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp ${ImGuiAddons_DIR}/ImGuiFileBrowser.cpp imgui_spectrum.cpp main_gui.cpp)
target_link_libraries(maingui PRIVATE GLFW3 OpenSSL::SSL OpenSSL::Crypto OpenGL::GL  nlohmann_json::nlohmann_json cpr::cpr)