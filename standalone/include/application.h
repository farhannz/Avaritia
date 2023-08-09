#pragma once

#include <avaritia/baseapp.h>
#include <avaritia/request.h>
#include <avaritia/launcher.h>
#include <setting.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <nfd.hpp>
#include <string>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <imgui_format.h>
#define AVARITIA_DEBUG false

// helper
std::string getFromCookies(const std::string &key, const cpr::Cookies &cookie){
    for(auto &c : cookie){
        if(c.GetName() == key) return c.GetValue();
    }
    return "";
}

class AvaritiaLauncher : public avaritia::BaseApp{
    private:
        bool mainWindow = true;
        bool loginPopupWindow = false;
        bool isMaintenance = false;
        bool settingWarning = false;
        bool setting_window = false;
        bool saveAccount = false;
        bool isAutoLogin = false;
        bool isConfigExist = false;
        bool isWrongPassword = false;
        bool invalidEmail = false;
        bool needAuth = false;
        bool isAuthOk = false;
        bool startGame = false;
        imgui_addons::ImGuiFileBrowser file_dialog;
        nlohmann::json ipApiResponseJson;
        std::string location = "Getting location....";
        std::string gameState = "Getting game state...";
        std::string gamePath = "";
        std::string m_email , m_pw, m_auth;
        std::string hashPassword = "placeholder";
        std::string tempLoginToken = "placeholder";
        std::string tempLoginCode = "placeholder";
        std::string ocean_session, accId, playCode;
        std::vector<cpr::AsyncResponse> responses;
        GLuint githubTexture;
        avaritia::setting::GlobalSetting config;
        std::vector<std::string> exceptionList;
        int githubTextureW;
        int githubTextureH;
        avaritia::LoginState loginState = static_cast<avaritia::LoginState>(127);
        cpr::Cookies currentCookies;
        PROCESS_INFORMATION uc_pi, game_pi;
    public:
    AvaritiaLauncher(std::string title, int w, int h) : avaritia::BaseApp(title, w , h){

    }
    void Start();
    void Shutdown();
    void Update();
};

void AvaritiaLauncher::Shutdown(){
    NFD::Quit();
}
void AvaritiaLauncher::Start(){
    spdlog::set_level(spdlog::level::debug);
    setWindowIcon("resources/icon8.png");
    NFD::Init();
    responses.emplace_back(cpr::GetAsync(cpr::Url("http://ip-api.com/json")));
    responses.emplace_back(cpr::GetAsync(cpr::Url("https://flg-main.aws.blue-protocol.com/flagfile/app_status")));
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
    io.Fonts->AddFontFromFileTTF( "resources/fonts/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/fonts/fa-v4compatibility.ttf", iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "resources/fonts/" FONT_ICON_FILE_NAME_FAB, iconFontSize, &icons_config, icons_ranges );
    isConfigExist = config.isConfigFileExist();
    if(!isConfigExist){
        config.setGuId(avaritia::setting::DeviceID().ToString());
        config.saveConfigToFile();
    }
    config.loadConfig();
    if(config.getGuId().empty()){
        config.setGuId(avaritia::setting::DeviceID().ToString());
        config.saveConfigToFile();
    }
    gamePath = config.getGamePath();
}

void AvaritiaLauncher::Update(){

     if(this->startGame){
        // BLUEPROTOCOL.exe .code <PlayCode>
        // ldr.exe --shortcut %1
        spdlog::info("Starting up BLUEPROTOCOL!");
        std::string bp = config.getGamePath() + "\\BLUEPROTOCOL.exe";
        this->game_pi = avaritia::startup(bp.c_str(),playCode,0);
        spdlog::info("Starting up UNCHEATER");
        std::string uncheater = config.getGamePath() + "\\BLUEPROTOCOL\\Binaries\\Win64\\ldr.exe";
        this->uc_pi = avaritia::startup(uncheater.c_str(),"",1);
        DWORD exitCode;
        GetExitCodeProcess(uc_pi.hProcess, &exitCode);
        WaitForSingleObject(uc_pi.hProcess,INFINITE);
        TerminateProcess(uc_pi.hProcess, exitCode);
        // }
        CloseHandle( game_pi.hProcess );
        CloseHandle( game_pi.hThread );
        CloseHandle( uc_pi.hProcess );
        CloseHandle( uc_pi.hThread );
        // Close process and thread handles.1
        this->closeWindow();
        return;
    }

    // Main Window Region
    {
        // BEGIN DEBUG ONLY
        if(AVARITIA_DEBUG){
            ImGui::TextFmt("Hash password : {}", this->hashPassword);
            ImGui::TextFmt("Login token : {}", this->tempLoginToken);
            ImGui::TextFmt("Login Code : {}", this->tempLoginCode);
            ImGui::TextFmt("Login State : {}", static_cast<int>(this->loginState));
            for(auto &c : currentCookies){
                ImGui::TextFmt("{} : {}", c.GetName(), c.GetValue());
            }

            for(auto &e : exceptionList){
                ImGui::TextFmt("{}",e);
            }
        }
        // END DEBUG ONLY
        // if(loginPopupWindow || setting_window) ImGui::BeginDisabled();
        // static float f = 0.0f;
        // ImGui::Text("%s %s",encrypted, decrypted);
        ImGuiIO io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Always, ImVec2(0,0));
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size,ImGuiCond_Always);
        ImGui::Begin("##begin",nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);

        // Left column - Login Column
        {
            this->startGame = false;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
            ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), false, window_flags);
            ImGui::Spacing();
            ImGui::Spacing();
            // ImGui::Indent(ImGui::GetContentRegionAvail().x * .125f);

            // Login Window
            if(needAuth) ImGui::BeginDisabled();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Text(ICON_FA_ENVELOPE " Email:");
            bool isEmailEnter = ImGui::InputText("##email", &this->m_email, ImGuiInputTextFlags_None | ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Text(ICON_FA_KEY " Password:");
            bool isPasswordEnter = ImGui::InputText("##password", &this->m_pw, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();
            if(needAuth) ImGui::EndDisabled();

            // Auth Code Input Text
            if(needAuth){
                ImGui::Text(ICON_FA_FINGERPRINT " Auth Code:");
                bool isAuthEnter = ImGui::InputText("##auth", &this->m_auth, ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::SameLine();
                if(ImGui::Button("Submit") || isAuthEnter){
                    // Send authentication code
                    try{
                        avaritia::AuthArgs authArgs{
                            this->accId,
                            this->m_auth,
                            config.getGuId(),
                            this->currentCookies,
                            false
                        };
                        authArgs = avaritia::Request::sendAuthCode(authArgs);
                        this->ocean_session = getFromCookies("ocean_session",currentCookies);
                        needAuth = authArgs.returnValue;
                        // needAuth = avaritia::Request::sendAuthCode(this->accId, this->m_auth,config.getGuId(),this->ocean_session, currentCookies);
                        // for(auto &c : currentCookies){
                        //     if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
                        // }
                        isAuthOk = !needAuth;
                    }
                    catch(std::exception &e){
                        exceptionList.push_back(e.what());
                    }
                }
            }
            
            
            if(this->isMaintenance) ImGui::BeginDisabled();
            if(this->invalidEmail)  ImGui::TextFmt("{} Invalid Email", ICON_FA_TRIANGLE_EXCLAMATION);
            if(needAuth) ImGui::BeginDisabled();
            if(ImGui::Button(ICON_FA_RIGHT_TO_BRACKET " Login") || isEmailEnter || isPasswordEnter || isAuthOk){
                for(int i = 0;i<(int)m_email.size();++i){
                    m_email[i] = tolower(m_email[i]);
                }
                std::regex emailValidator("^[-!#$%&'*+/0-9=?A-Z^_a-z{|}~](\\.?[-!#$%&'*+/0-9=?A-Z^_a-z{|}~])*@[a-zA-Z](-?[a-zA-Z0-9])*(\\.[a-zA-Z](-?[a-zA-Z0-9])*)+$");
                if(std::regex_match(m_email, emailValidator) && !std::string_view(m_pw).empty()){
                    // m_email = email;
                    // m_pw = password;
                    try{
                        // Launcher
                        // Ugly code
                        avaritia::LoginTokenArgs tokenArgs = {
                            avaritia::LoginRequest::Launcher,
                            this->accId,
                            config.getGuId(),
                            this->currentCookies,
                            ""
                        };
                        // tempLoginToken = avaritia::Request::getLoginToken(avaritia::LoginRequest::Launcher, config.getGuId(), this->ocean_session, currentCookies);
                        tokenArgs = avaritia::Request::getLoginToken(tokenArgs);
                        tempLoginToken = tokenArgs.returnValue;
                        currentCookies = tokenArgs.cookies;
                        // Ugly validation code
                        if(tempLoginToken.empty()){
                            ImGui::Text("Failed on retrieving login token");
                            return;
                        }


                        avaritia::LoginCodeArgs codeArgs = {
                            avaritia::LoginRequest::Launcher,
                            m_email,
                            m_pw,
                            tempLoginToken,
                            ""
                        };
                        // tempLoginCode = avaritia::Request::getLoginCode(avaritia::LoginRequest::Launcher, m_email, m_pw, tempLoginToken);
                        codeArgs = avaritia::Request::getLoginCode(codeArgs);
                        tempLoginCode = codeArgs.returnValue;
                        if(tempLoginCode.empty()){
                            ImGui::Text("Failed on retrieving login code"); 
                            return;
                        }
                        if(tempLoginCode == "wrong"){
                            isWrongPassword = true;
                            return;
                        }
                        isWrongPassword = false;
                        avaritia::LoginStateArgs stateArgs = {
                            avaritia::LoginRequest::Launcher,
                            tempLoginToken,
                            this->accId,
                            this->currentCookies,
                            this->playCode,
                            static_cast<avaritia::LoginState>(127)
                        };
                        stateArgs = avaritia::Request::getLoginState(stateArgs);
                        loginState = stateArgs.returnValue;
                        this->accId = stateArgs.accId_out;
                        currentCookies = stateArgs.cookies;
                        this->ocean_session = getFromCookies("ocean_session",currentCookies);
                        // loginState = avaritia::Request::getLoginState(avaritia::LoginRequest::Launcher, tempLoginToken, currentCookies, accId, playCode);
                        // for(auto &c : currentCookies){
                        //     if(c.GetName() == "ocean_session") this->ocean_session = c.GetValue();
                        // }
                        spdlog::info("Ocean session : {}", this->ocean_session);
                        if(loginState == avaritia::LoginState::CheckAuth){
                            this->needAuth = true;
                            return;
                        }

                        // Content 
                        tokenArgs = {
                            avaritia::LoginRequest::Content,
                            this->accId,    
                            config.getGuId(),
                            this->currentCookies,
                            ""
                        };
                        tokenArgs = avaritia::Request::getLoginToken(tokenArgs);
                        tempLoginToken = tokenArgs.returnValue;
                        this->currentCookies = tokenArgs.cookies;
                        this->ocean_session = getFromCookies("ocean_session",currentCookies);
                        // tempLoginToken = avaritia::Request::getLoginToken(avaritia::LoginRequest::Content, config.getGuId(), this->ocean_session, currentCookies, this->accId);
                        // for(auto &c : currentCookies){
                        //     if(c.GetName() == "ocean_session") this->ocean_session = c.GetValue();
                        // }
                        // Ugly validation code
                        if(tempLoginToken.empty()){
                            ImGui::Text("Failed on retrieving content login token");
                            this->isAuthOk = false;
                            return;
                        }
                        codeArgs = {
                            avaritia::LoginRequest::Content,
                            m_email,
                            m_pw,
                            tempLoginToken,
                            ""
                        };
                        // tempLoginCode = avaritia::Request::getLoginCode(avaritia::LoginRequest::Launcher, m_email, m_pw, tempLoginToken);
                        codeArgs = avaritia::Request::getLoginCode(codeArgs);
                        tempLoginCode = codeArgs.returnValue;
                        // tempLoginCode = avaritia::Request::getLoginCode(avaritia::LoginRequest::Content, m_email, m_pw, tempLoginToken);
                        if(tempLoginCode.empty()){
                            ImGui::Text("Failed on retrieving content login code");
                            this->isAuthOk = false;
                            return;
                        }
                        stateArgs = {
                            avaritia::LoginRequest::Content,
                            tempLoginToken,
                            this->accId,
                            this->currentCookies,
                            this->playCode,
                            static_cast<avaritia::LoginState>(127)
                        };
                        stateArgs = avaritia::Request::getLoginState(stateArgs);
                        loginState = stateArgs.returnValue;
                        this->accId = stateArgs.accId_out;
                        this->currentCookies = stateArgs.cookies;
                        this->ocean_session = getFromCookies("ocean_session",currentCookies);
                        this->playCode = stateArgs.playCode;
                        // loginState = avaritia::Request::getLoginState(avaritia::LoginRequest::Content, tempLoginToken, currentCookies, accId, playCode);
                        spdlog::debug("{}", static_cast<int>(loginState));
                        if(this->saveAccount){
                            // if(!accountList.empty() && (accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")){
                            //     accountList.clear();
                            // }
                            // if((accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")) 
                            // accountList.insert({m_email, merged});
                            // this->hashPassword = avaritia::Crypto::Encrypt(m_pw);
                            saveAccount = false;
                            // if(accountList.size()==1) defaultAccount = {accountList.begin()->first, "encrypted"};
                            // free(tmp);
                        }
                        this->isAuthOk = false;
                        this->startGame = true;
                        // ImGui::Text(loginToken.c_str());
                        // this->loginPopupWindow = false;
                    }
                    catch(std::exception &e){
                        exceptionList.push_back(fmt::format("(Login) {}",e.what()));
                    }
                    // this->loginPopupWindow = true;
                }
                else{
                    invalidEmail = true;
                }
            }
            if(needAuth) ImGui::EndDisabled();
            if(this->isMaintenance) ImGui::EndDisabled();
            // if(isMaintenance) ImGui::EndDisabled();
            // Right Col
            // ImGui::SameLine();
            ImGui::SameLine();
            if(ImGui::Button(ICON_FA_GEAR " Setting")) setting_window=true;
            ImGui::SameLine();
            // ImGui::Checkbox("Auto Login", &this->isAutoLogin); ImGui::SameLine(); HelpMarker("If this box is ticked, this account will be set as default");
            ImGui::Checkbox("Save Account",&this->saveAccount);
            ImGui::SameLine();
            HelpMarker("When you login, your current account will be saved into the account list.");
            // config.setAutoLogin(isAutoLogin);
            // ImGui::Text("%s",ipApiResponseJson.dump(4).c_str());
            if(this->isWrongPassword){
                ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION " Your email or password is either incorrect or unregistered!");
            }
            for(int i = 0;i<4;++i){
                ImGui::Spacing();
            }

            // GET IP GEOLOCATION, ASYNC
            if(this->responses[0].valid()){
                bool isReady = this->responses[0].wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                if(isReady) this->location = nlohmann::json::parse(this->responses[0].get().text)["country"];
            }
            // GET GAME STATE, ASYNC
            if(this->responses[1].valid()){
                bool isReady = this->responses[1].wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                if(isReady){
                    this->isMaintenance = (this->responses[1].get().text == "maintenance");
                    gameState = (this->isMaintenance) ? "Maintenance" : "Online";
                }
            }
            if(gameState != "Getting game state..."){
                if(this->isMaintenance){
                    ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION " The game is currently on maintenance, please wait until the maintenance is over!");
                }
            }
            // ImGui::TextFmt("Debug {} {} {}", this->responses[0].valid(), this->responses[1].valid(), this->isMaintenance);
            ImGui::TextFmt("Game State : {}", gameState);
            ImGui::TextFmt("Your Location {} : {}", ICON_FA_THUMBTACK, location);

            ImGui::Spacing();
            if(ImGui::Button(ICON_FA_GITHUB "##gh", ImVec2(30,30))) {
                OsOpenInShell("https://github.com/farhannz/avaritia");
            }
            if(ImGui::IsItemHovered()){
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();
        // Right column - Account switcher
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
                int idx = 0;
                ImGui::TableNextColumn();
                bool tmpSel = false;
                ImGui::Selectable("WIP", &tmpSel, ImGuiSelectableFlags_AllowDoubleClick);
                ImGui::TableNextColumn();
                rWidth = ImGui::GetColumnWidth();
                ImGui::Indent(rWidth * 0.25f);   
                ImGui::RadioButton("###wip", false);
                ImGui::Indent(-rWidth * 0.25f);
                // std::map<std::string, std::pair<std::string, std::string>>::iterator itr;
                // for(itr = accountList.begin(); itr != accountList.end(); itr++,idx++){
                //         ImGui::TableNextColumn();
                        // lWidth = ImGui::GetColumnWidth();   
                //         if(ImGui::Selectable(itr->first.c_str(),selected == idx,ImGuiSelectableFlags_AllowDoubleClick)) 
                //         {
                //             selected = idx;
                //             if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)){
                //                 if((accountList.begin()->first == "alt@alt.com" || accountList.begin()->first == "example@example.com")) continue;
                //                 m_email = itr->first;
                //                 int sizeIv = static_cast<int>(itr->second.first.size());
                //                 int sizeKey = static_cast<int>(itr->second.second.size());
                //                 unsigned char thisIv[sizeIv+1];
                //                 unsigned char thisKey[sizeKey+1];
                //                 hex2bin(itr->second.first.c_str(), thisIv);
                //                 hex2bin(itr->second.second.c_str(), thisKey);
                //                 decrypt(thisKey, sizeof(thisKey)/sizeof(byte),SECRET_SALT,thisIv,decrypted);
                //                 m_pw = std::string(reinterpret_cast<char *>(decrypted)); // decrypted
                //             }
                //         };
                //         ImGui::TableNextColumn();
                        // rWidth = ImGui::GetColumnWidth();   
                        // ImGui::Indent(rWidth * 0.25f);
                //         if(ImGui::RadioButton("###radio" + char(idx),&defaultSelected,idx)){
                //             defaultAccount = avaritia::setting::AccountInfo{itr->first,"encrypted"};
                //         }
                        // ImGui::Indent(-rWidth * 0.25f);
                // }
                ImGui::EndTable();
                ImGui::EndChild();
            }
            // ImGui::Indent(-(rWidth * 0.25f * accountList.size()));
            // ImGui::Text("%s", defaultAccount.email.c_str());
            ImGui::Button(ICON_FA_PLUS " Add Account"); ImGui::SameLine();
            // ImGui::Button(ICON_FA_MINUS " Remove Account"); ImGui::SameLine();
            // ImGui::SameLine();
            // ImGui::Checkbox("Save Account",&this->saveAccount);
            // ImGui::SameLine();
            // HelpMarker("When you login, your current account will be saved into the account list.");
            ImGui::EndChild();
            // ImGui::PopStyleVar();
        }
        // ImGui::EndDisabled();
        ImGui::End();
    }


    // Setting Window Popup
    if (setting_window || !isConfigExist){
        bool browse = false;
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_Appearing, ImVec2(0,0));
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size,ImGuiCond_Always);
        int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;
        if(!isConfigExist) flags |= ImGuiWindowFlags_NoMove;
        ImGui::Begin("Setting window",&setting_window, flags);
        ImGui::Text("Game path :"); ImGui::SameLine();
        nfdchar_t *outPath;
        ImGui::InputText("##gamepath", &gamePath, ImGuiInputTextFlags_None | ImGuiInputTextFlags_ReadOnly,nullptr); ImGui::SameLine();
        if(ImGui::Button("Browse##gamepath")){
            if(NFD::PickFolder(outPath, nullptr) == NFD_OKAY){
                gamePath = std::string(outPath);
            }
            // browse = true;
        }
        ImGui::TextFmt("Device ID : {}", config.getGuId());
        // if(browse){
        //     ImGui::OpenPopup("Browse");
        // }

        // if(file_dialog.showFileDialog("Browse", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT,ImVec2(400,400))){
        //     gamePath = file_dialog.selected_path;
        // }
        
        if(ImGui::Button("Save")){
            if(!gamePath.empty()){
                setting_window = false;
                browse = false;
                config.setGamePath(gamePath);
                // config.setAutoLogin(isAutoLogin);
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
}
