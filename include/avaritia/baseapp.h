#pragma once
#include <windows.h>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileBrowser.h>
#include <imgui_stdlib.h>
#include <imgui_spectrum.h>
#include <vector>
#include <stdio.h>
#include <cctype>
#include <regex>
#include <cassert>
#define GL_SILENCE_DEPRECATION
#define IMGUI_USER_CONFIG "imgui_config.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <iostream>
#include <IconsFontAwesome6.h>
#include <IconsFontAwesome6Brands.h>
#include <shellapi.h>
// #include <IconsFontAwesome5.h>
// #include <IconsFontAwesome4.h>
// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description);
void HelpMarker(const char* desc);
void OsOpenInShell(const char* path);
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
namespace avaritia{
    class BaseApp{
        private:
        std::string m_windowTitle;
        int m_width, m_height;
        protected:
        GLFWwindow *m_window = nullptr;
        public:
        BaseApp();
        BaseApp(std::string title, int width, int height);
        ~BaseApp();
        void Init();
        void Run();
        GLFWwindow *getWindow();
        void setWindowIcon(const std::string &path);
        void closeWindow();
        void virtual Start()=0;
        void virtual Update()=0;
    };
}