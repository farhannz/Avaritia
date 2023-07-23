#include <avaritia/baseapp.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
void OsOpenInShell(const char* path)
{
#ifdef _WIN32
    // Note: executable path must use backslashes!
    ::ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#else
#if __APPLE__
    const char* open_executable = "open";
#else
    const char* open_executable = "xdg-open";
#endif
    char command[256];
    snprintf(command, 256, "%s \"%s\"", open_executable, path);
    system(command);
#endif
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled(ICON_FA_QUESTION "");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


namespace avaritia{

    
    avaritia::BaseApp::BaseApp(std::string title, int width, int height){
        this->m_windowTitle = title;
        this->m_width = width;
        this->m_height = height;
    }
    avaritia::BaseApp::BaseApp(){
        this->m_windowTitle = "Default";
        this->m_width = 600;
        this->m_height = 400;
    }

    void avaritia::BaseApp::Init(){
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return;
        // Decide GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

        // Create window with graphics context
        this->m_window = glfwCreateWindow(this->m_width, this->m_height,this->m_windowTitle.c_str(), nullptr, nullptr);
        glfwSetWindowAttrib(this->m_window, GLFW_RESIZABLE, false);
        if (this->m_window == nullptr) throw std::runtime_error("GLFW Window should not be nullptr");
        // return;
        // GLFWimage windowIcon[1];
        // windowIcon[0].pixels = stbi_load("icon.png", &windowIcon[0].width, &windowIcon[0].height,0, STBI_rgb_alpha);
        // if(windowIcon[0].pixels != nullptr){
        //     glfwSetWindowIcon(m_window,1, windowIcon);
        //     stbi_image_free(windowIcon[0].pixels);
        // }
        glfwMakeContextCurrent(this->m_window);
        glfwSwapInterval(1); // Enable vsync

        // App will launch at the center of the screen

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;
        {
            int width, height;
            const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwGetWindowSize(this->m_window, &width, &height);
            glfwSetWindowPos(this->m_window, (vidmode->width - width)/2, (vidmode->height - height)/2);
        }
        
        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();
        ImGui::Spectrum::StyleColorsSpectrum();
        float baseFontSize = 18.0f; 
        float iconFontSize = baseFontSize * 2.0f / 3.0f;
        ImGui::Spectrum::LoadFont(baseFontSize);
        io.IniFilename = nullptr;
        
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

    }

    void avaritia::BaseApp::Run(){
        this->Start();
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        int idle_frames = 0;
        const int configFps = 15;
        while(!glfwWindowShouldClose(this->m_window)){            
            glfwPollEvents();
            glfwWaitEventsTimeout(1/configFps);
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            this->Update();
            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(this->m_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            } 
            glfwSwapBuffers(this->m_window);
        }
    }

    void BaseApp::closeWindow(){
        glfwSetWindowShouldClose(this->m_window,GL_TRUE);
    }
    void BaseApp::setWindowIcon(const std::string &path){
        GLFWwindow* currentWindow = getWindow();
        int width, height, channels;
        unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            std::cerr << "Failed to load image\n";
        }
        GLFWimage icon[1];
        icon[0].width = width;
        icon[0].height = height;
        icon[0].pixels = pixels;
        glfwSetWindowIcon(currentWindow,1,icon);
        stbi_image_free(pixels);
    }
    GLFWwindow *BaseApp::getWindow(){
        return this->m_window;
    }
    avaritia::BaseApp::~BaseApp(){
        #ifdef __EMSCRIPTEN__
            EMSCRIPTEN_MAINLOOP_END;
        #endif

            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(this->m_window);
            glfwTerminate();
    }
}