#define CPPHTTPLIB_OPENSSL_SUPPORT
// #include <winbase.h>
// #include <httplib.h>
#include <cpr/cpr.h>
#include <vector>
#include <gui/BaseApp.hpp>
#include <GlobalSetting.hpp>

void startApplication(LPSTR lpApplicationName, bool &started, STARTUPINFO &si, PROCESS_INFORMATION &pi, std::vector<std::string> all_args){
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
    std::string m_email, m_pw;
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
    bool saveAccount = false;
    // bool secondWindow = true;
    std::vector<avaritia::setting::AccountInfo> accountList{{"example@example.com","encryptedpw"},{"alt@alt.com","encrypted"}};
    bool accountSelected[accountList.size()] = {0};
    std::pair<avaritia::setting::AccountInfo, bool> defaultAccount{accountList[0],true};
    // bool isAccountSaved = false;
    if(!isConfigExist || (isConfigExist && config.getGuId().empty())) config.setGuId(avaritia::setting::DeviceID().ToString());
    if(isConfigExist) {
        config.loadConfig();
    }
    std::string location = "placeholder";
    std::string url = "http://ip-api.com";
    // httplib::Client ipApi(url);
    // auto res = ipApi.Get("/");
    cpr::Response ipApiResposne = cpr::Get(cpr::Url("http://ip-api.com/json"));
    auto ipApiResponseJson = nlohmann::json::parse(ipApiResposne.text);
    location = ipApiResponseJson["country"];
    float baseFontSize = 18.0f; 
    float iconFontSize = baseFontSize * 2.0f / 3.0f;
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true; 
    icons_config.GlyphMinAdvanceX = iconFontSize;
    // io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    // const char* fontPath = "resources/font";
    ImGuiIO io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF( "resources/font/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/font/fa-v4compatibility.ttf", iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/font/" FONT_ICON_FILE_NAME_FAB, iconFontSize, &icons_config, icons_ranges );
    assert(iconFont != nullptr);
    io.Fonts->Build();
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
        {
            ImGui::NextColumn();
            if(login_popup_window) ImGui::BeginDisabled();
            // static float f = 0.0f;
            ImGuiIO io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Always, ImVec2(0,0));
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size,ImGuiCond_Always);
            ImGui::Begin("##begin",nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);                          // Create a window called "Hello, world!" and append into it.



            // Child 1: no border, enable horizontal scrollbar
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), false, window_flags);
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Indent(ImGui::GetContentRegionAvail().x * .15f);
                ImGui::Text(ICON_FA_ENVELOPE " Email:");
                bool isEmailEnter = ImGui::InputText("##email", &m_email, ImGuiInputTextFlags_None | ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::Text(ICON_FA_KEY " Password:");
                bool isPasswordEnter = ImGui::InputText("##password", &m_pw,ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);
                if( ImGui::Button(ICON_FA_SIGN_IN " Login") || isEmailEnter || isPasswordEnter){
                    for(int i = 0;i<(int)m_email.size();++i){
                        m_email[i] = tolower(m_email[i]);
                    }
                    std::regex emailValidator("^[-!#$%&'*+/0-9=?A-Z^_a-z{|}~](\\.?[-!#$%&'*+/0-9=?A-Z^_a-z{|}~])*@[a-zA-Z](-?[a-zA-Z0-9])*(\\.[a-zA-Z](-?[a-zA-Z0-9])*)+$");
                    if(std::regex_match(m_email, emailValidator) && !std::string_view(m_pw).empty()){
                        // m_email = email;
                        // m_pw = password;
                        if(saveAccount){
                            accountList.push_back({m_email,"encrypted"});
                        }
                        // login_popup_window = true;
                    }
                }
                // Right Col
                // ImGui::SameLine();
                ImGui::SameLine();
                if(ImGui::Button(ICON_FA_GEAR " Setting")) setting_window=true;
                
                ImGui::SameLine();
                ImGui::SameLine();
                ImGui::SameLine();
                ImGui::Checkbox("Auto Login", &isAutoLogin); 
                config.setAutoLogin(isAutoLogin);
                // ImGui::Text("%s",ipApiResponseJson.dump(4).c_str());
                // for(int i = 0;i<16;++i){
                //     ImGui::Spacing();
                // }
                // ImGui::Text("Your Location : Japan");
                ImGui::Text("Your Location %s : %s",ICON_FA_MAP_LOCATION, location.c_str());
                ImGui::EndChild();
            }

            ImGui::SameLine();
            // Child 2: rounded border
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                ImGui::BeginChild("ChildR", ImVec2(0, 260), true, window_flags);
                static float lWidth, rWidth;
                ImGui::Text("Account switcher: "); ImGui::SameLine(); HelpMarker("Double click on the selected account to login directly"); 
                if (ImGui::BeginTable("accountId", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersOuterV, { 0, 0 }))
                {
                    ImGui::TableSetupColumn("Account");
                    ImGui::TableSetupColumn("Default",ImGuiTableColumnFlags_WidthStretch,0.25f);
                    ImGui::TableHeadersRow();
                    static int selected = -1;
                    for (int i = 0; i<(int)accountList.size();++i )
                    {
                        ImGui::TableNextColumn();
                        lWidth = ImGui::GetColumnWidth();   
                        if(ImGui::Selectable(accountList[i].email.c_str(),selected == i,ImGuiSelectableFlags_AllowDoubleClick)) selected = i;
                        ImGui::TableNextColumn();
                        bool defaultSelected = (accountList[i].email == defaultAccount.first.email) ? defaultAccount.second : false;
                        rWidth = ImGui::GetColumnWidth();   
                        ImGui::Indent(rWidth * 0.25f);
                        ImGui::Checkbox("##default",&defaultSelected);
                        ImGui::Indent(-rWidth * 0.25f);
                    }
                    ImGui::EndTable();
                }
                // ImGui::Indent(-(rWidth * 0.25f * accountList.size()));
                ImGui::Button(ICON_FA_PLUS" Add Account");
                ImGui::SameLine();
                ImGui::Checkbox("Save Account",&saveAccount);
                ImGui::SameLine();
                HelpMarker("When you login, your current account will be saved into the account list.");
                ImGui::EndChild();
                ImGui::PopStyleVar();
            }
            ImGui::End();
            ImGui::EndDisabled();
        }



        // PopUP Region
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
                startApplication(const_cast<char *>(std::string("main.exe").c_str()), service_started,si,pi,std::vector<std::string>{m_email,m_pw});
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
            ImGui::InputText("##gamepath", &gamepath, ImGuiInputTextFlags_None,nullptr); ImGui::SameLine();
            if(ImGui::Button("Browse##gamepath")){
                browse = true;
            }
            if(browse){
                ImGui::OpenPopup("Browse");
            }
            file_dialog.showFileDialog("Browse", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT,ImVec2(400,400));
            gamepath = file_dialog.selected_path;
            
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
        ImGui::Text("%f %f", ImGui::GetColumnOffset(0), ImGui::GetColumnOffset(1));

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
