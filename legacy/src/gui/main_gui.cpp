// #define CPPHTTPLIB_OPENSSL_SUPPORT
// #include <winbase.h>
// #include <httplib.h>
#include <cpr/cpr.h>
#include <vector>
#include <gui/BaseApp.hpp>
#include <GlobalSetting.hpp>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>
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
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */

    
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return -1;
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return -1;
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;
    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return -1;
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        return -1;
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(const char* src, unsigned char* target)
{
  while(*src && src[1])
  {
    *(target++) = char2int(*src)*16 + char2int(src[1]);
    src += 2;
  }
}


std::string bin2hex(unsigned char *src){
    std::stringstream ss;
    ss << std::hex;
    for(int i = 0;src[i] != '\0';++i){
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(src[i]);
    }
    return ss.str();
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
    std::map<std::string, std::pair<std::string, std::string>> accountList;
    bool accountSelected[accountList.size()];
    avaritia::setting::AccountInfo defaultAccount;
    // bool isAccountSaved = false;
    if(!isConfigExist || (isConfigExist && config.getGuId().empty())) config.setGuId(avaritia::setting::DeviceID().ToString());
    if(isConfigExist) {
        config.loadConfig();
        gamepath = config.getGamePath();
    }
    std::string location = "placeholder";
    std::string url = "http://ip-api.com";
    // httplib::Client ipApi(url);
    // auto res = ipApi.Get("/");
    cpr::Response ipApiResponse = cpr::Get(cpr::Url("http://ip-api.com/json"));
    auto ipApiResponseJson = nlohmann::json::parse(ipApiResponse.text);
    cpr::Response checkMaintenance = cpr::Get(cpr::Url("https://flg-main.aws.blue-protocol.com/flagfile/app_status"));
    bool isMaintenance = (checkMaintenance.text == "maintenance");
    location = ipApiResponseJson["country"];
    float baseFontSize = 18.0f; 
    float iconFontSize = baseFontSize * 2.0f / 3.0f;
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true; 
    icons_config.GlyphMinAdvanceX = iconFontSize;
    bool defaultCheck = false;
    // io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    // const char* fontPath = "resources/font";
    ImGuiIO io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF( "resources/font/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/font/fa-v4compatibility.ttf", iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/font/" FONT_ICON_FILE_NAME_FAB, iconFontSize, &icons_config, icons_ranges );
    // unsigned char salt[] = "salt";
    // const char test[] = "ae3fe5f5707e07f3e7c117fb885cd052a6fcd77a";
    // unsigned char test2[20];
    // hex2bin(test, test2);
    
    unsigned char encrypted[256];
    unsigned char decrypted[256];
    // unsigned char pw[32] = "password";
    // EVP_Cipher{same};
    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));
    // const char test[]= "091aa072c3e87fcf45022784a0ca31";
    // unsigned char test2[30];
    // unsigned char pw[] = "password";
    // encrypt(pw,8,SECRET_SALT,iv,encrypted);
    
    // decrypt(encrypted,sizeof(encrypted)/sizeof(unsigned char),SECRET_SALT,iv,decrypted);
    // AES_set_decrypt_key(SECRET_SALT,128,&key);
    // AES_decrypt(encrypted,decrypted,&key);
    // AES_set_decrypt_key(SECRET_SALT,128,&key);
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

            // for(int i =0;iv[i] != '\0';++i){
            //     ImGui::Text("%02x", iv[i]);
            //     ImGui::SameLine();
            // }
            // ImGui::Text("-Dummy-");
            // for(int i =0;encrypted[i] != '\0';++i){
            //     ImGui::Text("%02x", encrypted[i]);
            //     ImGui::SameLine();
            // }


            // ImGui::Text("%s",std::string(reinterpret_cast<char *>(decrypted)).c_str());
            // ImGui::NextColumn();
            if(login_popup_window) ImGui::BeginDisabled();
            // static float f = 0.0f;
            // ImGui::Text("%s %s",encrypted, decrypted);
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
                // ImGui::Indent(ImGui::GetContentRegionAvail().x * .125f);
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Text(ICON_FA_ENVELOPE " Email:");
                bool isEmailEnter = ImGui::InputText("##email", &m_email, ImGuiInputTextFlags_None | ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::Text(ICON_FA_KEY " Password:");
                bool isPasswordEnter = ImGui::InputText("##password", &m_pw,ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::PopItemWidth();
                if(isMaintenance) ImGui::BeginDisabled();
                if( ImGui::Button(ICON_FA_RIGHT_TO_BRACKET " Login") || isEmailEnter || isPasswordEnter){
                    for(int i = 0;i<(int)m_email.size();++i){
                        m_email[i] = tolower(m_email[i]);
                    }
                    std::regex emailValidator("^[-!#$%&'*+/0-9=?A-Z^_a-z{|}~](\\.?[-!#$%&'*+/0-9=?A-Z^_a-z{|}~])*@[a-zA-Z](-?[a-zA-Z0-9])*(\\.[a-zA-Z](-?[a-zA-Z0-9])*)+$");
                    if(std::regex_match(m_email, emailValidator) && !std::string_view(m_pw).empty()){
                        // m_email = email;
                        // m_pw = password;
                        if(saveAccount){
                            if(!accountList.empty() && (accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")){
                                accountList.clear();
                            }
                            if((accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")) continue;
                            int size = static_cast<int>(m_pw.size());
                            unsigned char tmp[size +1];
                            memcpy(tmp, m_pw.c_str(),size+1);
                            // // PKCS5_PBKDF2_HMAC_SHA1(m_pw.c_str(),static_cast<int>(m_pw.size()),SECRET_SALT,sizeof(SECRET_SALT)/sizeof(byte),10000,32,out);
                            encrypt(tmp,sizeof(tmp)/sizeof(byte),SECRET_SALT,iv,encrypted);
                            std::pair<std::string,std::string> merged;
                            merged.first = bin2hex(iv);
                            merged.second = bin2hex(encrypted);
                            // ImGui::Text("%s", tmp);
                            // ImGui::Text("%s", merged.c_str());
                            accountList.insert({m_email, merged});
                            saveAccount = false;
                            // accountList.insert({m_email,std::string(reinterpret_cast<char *>(out))});
                            if(accountList.size()==1) defaultAccount = {accountList.begin()->first, "encrypted"};
                            // free(tmp);
                        }
                        login_popup_window = true;
                    }
                }
                if(isMaintenance) ImGui::EndDisabled();
                // AES_set_decrypt_key(SECRET_SALT,128,&key);
                //             AES_decrypt(encrypted,decrypted,&key);
                // Right Col
                // ImGui::SameLine();
                ImGui::SameLine();
                if(ImGui::Button(ICON_FA_GEAR " Setting")) setting_window=true;
                ImGui::SameLine();
                ImGui::Checkbox("Auto Login", &isAutoLogin); ImGui::SameLine(); HelpMarker("If this box is ticked, this account will be set as default");
                config.setAutoLogin(isAutoLogin);
                // ImGui::Text("%s",ipApiResponseJson.dump(4).c_str());
                for(int i = 0;i<4;++i){
                    ImGui::Spacing();
                }
                // ImGui::Text("Your Location : Japan");
                if(isMaintenance){
                    ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION " The game is currently on maintenance, please wait until the maintenance is over!");
                }
                ImGui::Text("Game status : %s", checkMaintenance.text.c_str());
                ImGui::Text("Your Location %s : %s", ICON_FA_THUMBTACK, location.c_str());
                ImGui::EndChild();
            }

            ImGui::SameLine();
            // Child 2: rounded border
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                ImGui::BeginChild("ChildR", ImVec2(0, 260), false, window_flags);
                ImGui::Text("Account switcher: "); ImGui::SameLine(); HelpMarker("Double click on the selected account to login directly"); 
                ImGui::BeginChild("TableContainer", ImVec2(0, 170), true, window_flags);
                static float lWidth, rWidth;
                if (ImGui::BeginTable("accountId", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersOuterV, { 0, 0 }))
                {
                    ImGui::TableSetupColumn("Account");
                    ImGui::TableSetupColumn("Default",ImGuiTableColumnFlags_WidthStretch,0.25f);
                    ImGui::TableHeadersRow();
                    static int selected = -1;
                    static int defaultSelected = -1;
                    std::map<std::string, std::pair<std::string, std::string>>::iterator itr;
                    int idx = 0;
                    for(itr = accountList.begin(); itr != accountList.end(); itr++,idx++){
                            ImGui::TableNextColumn();
                            lWidth = ImGui::GetColumnWidth();   
                            if(ImGui::Selectable(itr->first.c_str(),selected == idx,ImGuiSelectableFlags_AllowDoubleClick)) 
                            {
                                selected = idx;
                                if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)){
                                    if((accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")) continue;
                                    m_email = itr->first;
                                    int sizeIv = static_cast<int>(itr->second.first.size());
                                    int sizeKey = static_cast<int>(itr->second.second.size());
                                    unsigned char thisIv[sizeIv+1];
                                    unsigned char thisKey[sizeKey+1];
                                    hex2bin(itr->second.first.c_str(), thisIv);
                                    hex2bin(itr->second.second.c_str(), thisKey);
                                    decrypt(thisKey, sizeof(thisKey)/sizeof(byte),SECRET_SALT,thisIv,decrypted);
                                    m_pw = std::string(reinterpret_cast<char *>(decrypted)); // decrypted
                                }
                            };
                            ImGui::TableNextColumn();
                            rWidth = ImGui::GetColumnWidth();   
                            ImGui::Indent(rWidth * 0.25f);
                            if(ImGui::RadioButton("###radio" + char(idx),&defaultSelected,idx)){
                                defaultAccount = avaritia::setting::AccountInfo{itr->first,"encrypted"};
                            }
                            ImGui::Indent(-rWidth * 0.25f);
                    }
                    // for (int i = 0; i<(int)accountList.size();++i )
                    // {
                    //     ImGui::TableNextColumn();
                    //     lWidth = ImGui::GetColumnWidth();   
                    //     if(ImGui::Selectable(accountList[i].email.c_str(),selected == i,ImGuiSelectableFlags_AllowDoubleClick)) 
                    //     {
                    //         selected = i;
                    //         if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)){
                    //             m_email = accountList[];
                    //             m_pw = accountList[i].password; // decrypted
                    //         }
                           
                    //     };
                    //     ImGui::TableNextColumn();
                    //     rWidth = ImGui::GetColumnWidth();   
                    //     ImGui::Indent(rWidth * 0.25f);
                    //     if(ImGui::RadioButton("###radio" + char(i),&defaultSelected,i)){
                    //         defaultAccount = accountList[i];
                    //     }
                    //     ImGui::Indent(-rWidth * 0.25f);
                    // }
                    ImGui::EndTable();
                    ImGui::EndChild();
                }
                // ImGui::Indent(-(rWidth * 0.25f * accountList.size()));
                // ImGui::Text("%s", defaultAccount.email.c_str());
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
                startApplication(const_cast<char *>(std::string("main.exe").c_str()), service_started,si,pi,std::vector<std::string>{m_email,m_pw,config.getGuId(),config.getGamePath()});
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

            if(file_dialog.showFileDialog("Browse", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT,ImVec2(400,400))){
                gamepath = file_dialog.selected_path;
            }
            
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
