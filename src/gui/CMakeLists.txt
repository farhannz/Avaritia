

cmake_minimum_required(VERSION 3.24)
project(maingui)

set(IMGUI_DIR "../../submodules/imgui")
set(PARENT_DIR "../../")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -Wl,-subsystem,windows -lgdi32 -limm32 ")
set(ImGuiAddons_DIR ${PARENT_DIR}/submodules/ImGui-Addons/FileBrowser)
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends PARENT_DIR/include ImGuiAddons_DIR ImGuiAddons_DIR/Dirent)

set(IMGUI_BACKENDS ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
# submodules\ImGui-Addons\FileBrowser\ImGuiFileBrowser.cpp

aux_source_directory(component COMPONENT_FILES)
aux_source_directory(${IMGUI_DIR} IMGUI_FILES)



add_executable(maingui ${COMPONENT_FILES} ${IMGUI_BACKENDS} ${IMGUI_FILES} ${ImGuiAddons_DIR}/ImGuiFileBrowser.cpp main_gui.cpp)
target_link_libraries(maingui GLFW3 OpenGL32)