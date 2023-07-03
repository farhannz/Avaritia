
// #include <winbase.h>
#include <gui/BaseApp.hpp>
#include <GlobalSetting.hpp>

void StartService(LPSTR lpApplicationName, bool &started, STARTUPINFO &si, PROCESS_INFORMATION &pi, std::vector<std::string> all_args){
    if(started) return;

    started = true;
    // additional information
    // STARTUPINFO si;     
    // PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    std::string paramString = "";
    for(auto s : all_args){
        paramString.append(s + " ");
    }
    // spdlog::debug("{} {}", lpApplicationName, paramString);
    LPSTR param = const_cast<LPSTR>(paramString.c_str());
    // ImGui::Text("%s", param);
    // start the program up
    CreateProcess( lpApplicationName,   // the path
    param,        // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    FALSE,          // Set handle inheritance to FALSE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory 
    &si,            // Pointer to STARTUPINFO structure
    &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}



// Main code
int main(int, char**)
{
    avaritia::gui::BaseApp app("Avaritia Launcher", 600,300);
    app.init();

    // Our state
    // bool show_demo_window = false;
    bool login_popup_window = false;
    bool setting_window = false;
    static std::string gamepath;
    static std::string _email, _pw;
    imgui_addons::ImGuiFileBrowser file_dialog;
    bool settingWarning=false;
    bool service_started = false; // Service flag
    STARTUPINFO si;               // Service info
    PROCESS_INFORMATION pi;       // Service Process Info
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    DWORD exitCode;
    avaritia::setting::GlobalSetting config;
    bool isConfigExist = config.isConfigFileExist();
    bool isAutoLogin = config.isAutoLogin();
    if(!isConfigExist || (isConfigExist && config.getGuId().empty())) config.setGuId(avaritia::setting::DeviceID().ToString());
    if(isConfigExist) {
        config.loadConfig();
    }
    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(app.getWindow()))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO io = ImGui::GetIO();
        {
            if(login_popup_window) ImGui::BeginDisabled();
            // static float f = 0.0f;
            ImGuiIO io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Always, ImVec2(0,0));
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size,ImGuiCond_Always);
            ImGui::Begin("##begin",nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);                          // Create a window called "Hello, world!" and append into it.

            // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            
            static char email[64];
            static char password[32];
            
            ImGui::Text("Email:");
            bool isEmailEnter = ImGui::InputText("##email", email,IM_ARRAYSIZE(email),ImGuiInputTextFlags_None | ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Text("Password:");
            bool isPasswordEnter = ImGui::InputText("##password", password,IM_ARRAYSIZE(password),ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);
            if( ImGui::Button("Login") || isEmailEnter || isPasswordEnter){
                for(int i = 0;i<64;++i){
                    email[i] = tolower(email[i]);
                }
                std::regex emailValidator("^[-!#$%&'*+/0-9=?A-Z^_a-z{|}~](\\.?[-!#$%&'*+/0-9=?A-Z^_a-z{|}~])*@[a-zA-Z](-?[a-zA-Z0-9])*(\\.[a-zA-Z](-?[a-zA-Z0-9])*)+$");
                if(std::regex_match(email, emailValidator) && !std::string_view(password).empty()){
                    _email = email;
                    _pw = password;
                    login_popup_window = true;
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Setting")) setting_window=true;
            ImGui::SameLine();
            ImGui::SameLine();
            ImGui::SameLine();
            ImGui::Checkbox("Save Account", &isAutoLogin); 
            config.setAutoLogin(isAutoLogin);

            ImGui::End();
            ImGui::EndDisabled();
        }

        if (login_popup_window){
            ImVec2 sz = ImGui::GetMainViewport()->Size;
            ImVec2 pos = ImGui::GetMainViewport()->Pos;
            sz.x *= .5f; sz.y *= .5f;
            pos.x = (pos.x + sz.x); pos.y = (pos.y + sz.y);
            sz.x *= .5f;
            ImGui::SetNextWindowSize(sz,ImGuiCond_Always);
            ImGui::SetNextWindowPos(pos,ImGuiCond_Always, ImVec2(0.0f,0.0f));
            ImGui::BeginPopupContextVoid("##login_notif");
            

            if(!service_started){
                StartService(const_cast<char *>(std::string("main.exe").c_str()), service_started,si,pi,std::vector<std::string>{_email,_pw});
                config.saveConfigToFile();
            }
            if (ImGui::Button("Close Me")){
                login_popup_window = false;
            }
            ImGui::EndPopup();
        }

        if (setting_window || !isConfigExist){
            bool browse = false;
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Appearing, ImVec2(0,0));
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size,ImGuiCond_Always);
            ImGui::Begin("Setting window",&setting_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
            ImGui::Text("Game path :"); ImGui::SameLine();
            ImGui::InputText("##gamepath", const_cast<char*>(gamepath.c_str()),IM_ARRAYSIZE(gamepath.c_str()),ImGuiInputTextFlags_None,nullptr); ImGui::SameLine();
            if(ImGui::Button("Browse##gamepath")){
                browse = true;
            }
            if(browse){
                ImGui::OpenPopup("Browse");
            }
            file_dialog.showFileDialog("Browse", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT,ImVec2(400,400));
            gamepath = file_dialog.selected_path.c_str();
            
            if(ImGui::Button("Save")){
                if(!gamepath.empty()){
                    setting_window = false;
                    browse = false;
                    config.setGamePath(gamepath);
                    config.setAutoLogin(isAutoLogin);
                    config.saveConfigToFile();
                    isConfigExist = true;
                    settingWarning =false;
                }
                else{
                    settingWarning = true;
                }
            }
            if(settingWarning){
                ImGui::Text("Please enter your game directory");
            }
            ImGui::End();
            // setting_window=false;
        }
        {
            GetExitCodeProcess(pi.hProcess, &exitCode);
            // ImGui::Text("%ld", exitCode);
            if(service_started && exitCode == 0){
                break;
            }
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(app.getWindow(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(app.getWindow());
    }

    app.~BaseApp();
    return 0;
}
