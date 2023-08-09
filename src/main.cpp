#if !defined(__AVARITIA__MAIN)
#define __AVARITIA__MAIN
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <httplib.h>
#include <GlobalSetting.hpp>
#include <Common.hpp>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <hash/MurmurHash3.hpp>
#include <cctype>
#include <spdlog/spdlog.h>
#define endl '\n'


// POST https://api-bnolauncher.bandainamco-ol.jp/api/login/authcode/check
// Use ocean_session and ocean_hash
// {"acc_id":"Vp4DYRxRnNf7RWNVfOPX","auth_code":"059348","device_id":"b6bef253cc564fe8abb991e02ee7732b"}
// Response
// {"failed_count":0,"error_code":0}


std::string getOceanHash(std::string ocean_session, nlohmann::json data){
    uint32_t seed = 2929;

    std::string s = data.dump() + "@@@" + ocean_session;

    // if(DEBUG_MODE) std::cout << data.dump() << std::endl;
    spdlog::debug("(getOceanHash) {}", s);
    byte hash_otpt[128]= {};
    MurmurHash3_x86_128(s.c_str(),s.length(),seed,hash_otpt);
    int i = 0;
    int j = 0;
    byte b = hash_otpt[i++];
    byte dst[sizeof(hash_otpt)/sizeof(byte) * 3 - 1];
    const char* HexValue = "0123456789ABCDEF";
    dst[j++] = HexValue[b >> 4];
    dst[j++] = HexValue[b & 0xf];

    while(i < sizeof(hash_otpt)/sizeof(byte)){
        byte b = hash_otpt[i++];
        // dst[j++] = '-';
        dst[j++] = HexValue[b >> 4];
        dst[j++] = HexValue[b & 0xf];
    }
    for(int i = 0;i<((sizeof(dst)/sizeof(byte)));++i){
        char tmp = tolower(dst[i]);
        dst[i] = tmp;
    }
    for(int i = 32;i<((sizeof(dst)/sizeof(byte)));++i){
        dst[i] = '\0';
    }
    char *result = (char*)dst;
    return std::string(result);
}

std::string jsonToQueryString(nlohmann::json data){
    std::string query;
    for(auto& [key, val] : data.items()){
        query.append(key);
        query.append(std::string("="));
        if(!val.is_string()) val = val.dump();
        query.append(val);
        query.append(std::string("&"));
    }
    query.erase(query.length()-1);
    spdlog::debug("(jsonToQueryString) {}", query);
    return query;
}


std::vector<std::string> splitByDelimiter(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        if (item.length() > 0) {
            elems.push_back(item);  
        }
    }
    return elems;
}

std::string getOceanSession(httplib::Headers headers){
    std::string oceanSession = "";
    for(auto &[key, val] : headers){
        spdlog::debug("{} {}", key, val);
        if(key == "Set-Cookie"){
            std::string cookie = val;
            auto valSplit = splitByDelimiter(val, '=');
            if(valSplit[0] == "ocean_session"){
                oceanSession = splitByDelimiter(valSplit[1],';')[0];
            }
        }
    }
    spdlog::debug("ocean_session {}", oceanSession);
    return oceanSession;
}


PROCESS_INFORMATION startup(LPCTSTR lpApplicationName, std::string playCode, int cases)
{
    // additional information
    STARTUPINFO si;     
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    std::string paramString = "";
    switch(cases){
        case 0:
            paramString = " -code ";
            paramString.append(playCode);
            break;
        default:
            paramString = " --shortcut %1";
            break;
    }
    spdlog::debug("{} {}", lpApplicationName, paramString);
    LPSTR param = const_cast<LPSTR>(paramString.c_str());
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
    return pi;
}

int main(int argc, char const *argv[])
{
    spdlog::set_level(spdlog::level::info); // Set global log level to debug
    std::string GAME_PATH = "G:\\FNZ\\BLUEPROTOCOL";
    // avaritia::DeviceID deviceId = avaritia::DeviceID();
    std::string deviceId = "8a06386b222e461dbac09934fec2527e"; // DEBUG ONLY
    // spdlog::debug("(main) DeviceID : {}",deviceId.ToString());
    spdlog::debug("(main) DeviceID : {}",deviceId);
    std::string ocean_session;
    httplib::Client launcherAPI("https://" + avaritia::common::url().API_LAUNCHER);
    httplib::Client accountAPI("https://" + avaritia::common::url().API_ACCOUNT);
    httplib::Headers launcherHeaders = {
        {"User-Agent" ,avaritia::common::url().USER_AGENT},
        {"Host", avaritia::common::url().API_LAUNCHER}
    };
    httplib::Headers accountHeaders = {
        {"User-Agent" ,avaritia::common::url().USER_AGENT},
        {"Host", avaritia::common::url().API_ACCOUNT}
    };
    launcherAPI.set_default_headers(launcherHeaders);
    accountAPI.set_default_headers(accountHeaders);
    // nlohmann::json dataLoginUrl{{"acc_id",""},{"auto_login",false},{"debug_user",false},{"device_id", deviceId.ToString()}};
    // launcherAPI.set_default_headers(launcherHeaders);
    // auto res = launcherAPI.Post("/api/content/login/url", dataLoginUrl.dump(), "application/json");
    // auto response = nlohmann::json(res->body);
    // std::cout << res->status << "\n" << response << std::endl;

    // while(true){
    // }
    // =====Access Token=====
    nlohmann::json getAccessTokenPayload{  // POST to /api/login/url application/json
        {"acc_id",""},
        {"auto_login",true},
        {"debug_user",false},
        {"device_id",deviceId}
    };
    spdlog::debug("Acess Token Payload : {}", getAccessTokenPayload.dump(4));
    // POST TO API_LAUNCHER/api/login/url
    auto response = launcherAPI.Post("/api/login/url",getAccessTokenPayload.dump(4),"application/json");
    spdlog::debug("Header : {}", nlohmann::json(response->headers).dump(4));
    spdlog::debug("Response : {}", response->body);
    // PARSE RESPONSE
    nlohmann::json responseJson = nlohmann::json::parse(response->body);
    std::string accessToken = responseJson["access_token"];
    spdlog::debug("Access token : {}", accessToken);


    // =======LOGIN========
    nlohmann::json getLoginCodePayload{ // aplication/x-www-form-urlencoded
        {"client_id","bno_ocean"},
        {"redirect_uri","https://api-bnolauncher.bandainamco-ol.jp/api/login/bana/result/"+accessToken},
        {"customize_id","launcher"},
        {"login_id","placeholder@email.com"},
        {"password","placeholderpassword"},
        {"language","en"},
        {"shortcut",0},
        {"retention","\0"},
        {"cookie",nlohmann::json::parse("{}")},
        {"prompt","\0"}
    };
    std::string tmp;
    if(argc < 2){
        spdlog::info("Enter your Email:");
        std::cin >> tmp;
        getLoginCodePayload["login_id"] = tmp;
        spdlog::info("Enter your Password:");
        std::cin >> tmp;
        getLoginCodePayload["password"] = tmp;
    }
    else{
        std::vector<std::string> all_args;
        all_args.assign(argv, argv + argc);
        if(all_args.size() < 4){
            
            spdlog::warn("Error too few arguments");
        }
        getLoginCodePayload["login_id"] = all_args[0];
        getLoginCodePayload["password"] = all_args[1];
        deviceId = all_args[2];
        GAME_PATH = all_args[3];
        tmp = all_args[1];
    }
    spdlog::info("Logging in....");
    // POST TO API_ACCOUNT/v3/api/login/idpw
    // tmp = "";
    response = accountAPI.Post("/v3/login/idpw", jsonToQueryString(getLoginCodePayload),"application/x-wwww-form-urlencoded");
    httplib::Headers loginResponseHeader = response->headers;
    getLoginCodePayload["password"] = "PASSWORD IS REMOVED";
    spdlog::debug("Login Code Payload : {}", getLoginCodePayload.dump());
    // jsonToQueryString(getLoginCodePayload);
    // spdlog::debug("Header : {}", nlohmann::json(response->headers).dump(4));
    spdlog::debug("Response : {}", response->body);
    if(response->status == 200){
        spdlog::info("Email : {}",getLoginCodePayload["login_id"]);
        spdlog::info("Password : {}",getLoginCodePayload["password"]);
        // PARSE RESPONSE
        responseJson = nlohmann::json::parse(response->body);
        std::string redirect = responseJson["redirect"];
        std::string loginCode = splitByDelimiter(redirect,'=').back();
        spdlog::debug("Redirect : {}",redirect);
        spdlog::debug("Login Code : {}",loginCode);
        // GET API_LAUNCHER/api/login/bana/result/{accesToken}?code={loginCode}
        response = launcherAPI.Get("/api/login/bana/result/"+accessToken+"?code="+loginCode);
        // nlohmann::json res {{"status_code",200}};
        if(response->status == 200){
            // CHECK LOGIN STATE
            // POST API_LAUNCHER/api/login/state
            nlohmann::json getLoginStateData = {
                {"acc_id",""},
                {"access_token",accessToken},
                };
            response = launcherAPI.Post("/api/login/state",getLoginStateData.dump(),"application/json");
            // responseJson = nlohmann::json::parse(response->body);
            spdlog::debug("Headers : {}", nlohmann::json(response->headers).dump(4));
            responseJson = nlohmann::json::parse(response->body);
            ocean_session = getOceanSession(response->headers);
            spdlog::debug("Response {}", responseJson.dump(4));
            int login_state = responseJson["login_state"];
            std::string authCode;
            switch(login_state){
                case 0:
                    break;
                case 1:
                    // spdlog::info("Authentication code needed!");
                    // spdlog::info("Please check your email!");
                    // spdlog::info("Input your authentication code :");
                    // std::cin >> authCode;
                    // nlohmann::json data{
                    //     {"acc_id", responseJson["acc_id"]},
                    //     {"auth_code",authCode},
                    //     {"device_id",deviceId}
                    // };
                    // std::string ocean_hash = getOceanHash(ocean_session, data);
                    // httplib::Headers postHeader = {
                    //     {"Cookie", "ocean_session="+ocean_session+";"+"ocean_hash=" + ocean_hash},
                    //     {"Origin", "https://api-bnolauncher.bandainamco-ol.jp"},
                    //     {"User-Agent" ,avaritia::common::url().USER_AGENT},
                    //     {"Host", avaritia::common::url().API_ACCOUNT}
                    // };
                    // launcherAPI.set_default_headers(postHeader);
                    // response = launcherAPI.Post("/api/login/authcode/check",data.dump(),"application/json");

                    // POST https://api-bnolauncher.bandainamco-ol.jp/api/login/authcode/check
                    // Use ocean_session and ocean_hash
                    // {"acc_id":"Vp4DYRxRnNf7RWNVfOPX","auth_code":"059348","device_id":"b6bef253cc564fe8abb991e02ee7732b"}
                    // Response
                    // {"failed_count":0,"error_code":0}
                    break;
                case 2:
                    break;
                case 3:
                    // POST /api/content/login/url
                    nlohmann::json data = {
                        {"acc_id",responseJson["acc_id"]},
                        {"auto_login",true},
                        {"cuid","ELrmVeJRPRZMwrerwsJWkTxpKQABgNw3"}
                        };
                    // std::cin >> ocean_session;
                    
                    std::string ocean_hash = getOceanHash(ocean_session,data);
                    spdlog::debug("Ocean hash {}", ocean_hash);
                    httplib::Headers postHeader = {
                        {"Cookie", "ocean_session="+ocean_session+";"+"ocean_hash=" + ocean_hash},
                        {"Origin", "https://api-bnolauncher.bandainamco-ol.jp"},
                        {"User-Agent" ,avaritia::common::url().USER_AGENT},
                        {"Host", avaritia::common::url().API_ACCOUNT}
                    };
                    launcherAPI.set_default_headers(postHeader);
                    response = launcherAPI.Post("/api/content/login/url",data.dump(),"application/json");
                    spdlog::debug(nlohmann::json(postHeader).dump(4));
                    spdlog::debug(response->body);
                    responseJson = nlohmann::json::parse(response->body);
                    if(responseJson["error_code"] == 0){
                        accessToken = responseJson["access_token"];
                        spdlog::debug("Access token : {}", accessToken);
                        ocean_session = getOceanSession(response->headers);
                        data = {
                                {"acc_id",""},
                            { "access_token",accessToken}
                            };
                        ocean_hash = getOceanHash(ocean_session,data);
                        spdlog::debug("ocean_hash {}",ocean_hash);
                        spdlog::debug(nlohmann::json(loginResponseHeader).dump(4));
                        // if(DEBUG_MODE)std::cout << ocean_hash << std::endl;
                        getLoginCodePayload["customize_id"] = "\0";
                        getLoginCodePayload["client_id"] = "bp_service";
                        getLoginCodePayload["redirect_uri"] = "https://api-bnolauncher.bandainamco-ol.jp/api/content/bana/result/"+accessToken;
                        getLoginCodePayload["password"] = tmp;
                        tmp = "";
                        response = accountAPI.Post("/v3/login/idpw", jsonToQueryString(getLoginCodePayload),"application/x-wwww-form-urlencoded");
                        responseJson = nlohmann::json::parse(response->body);
                        spdlog::debug(response->body);
                        if(response->status == 200){
                            redirect = responseJson["redirect"];
                            response = launcherAPI.Get(redirect);
                            spdlog::debug(response->body);
                            if(response->status == 200){
                                std::string uncheater = GAME_PATH + "\\BLUEPROTOCOL\\Binaries\\Win64\\ldr.exe";
                                // std::string playCode = splitByDelimiter(redirect,'=').back();
                                postHeader = {
                                    {"Cookie", "ocean_session="+ocean_session+";"+"ocean_hash=" + ocean_hash},
                                    {"Origin", "https://api-bnolauncher.bandainamco-ol.jp"},
                                    {"User-Agent" ,avaritia::common::url().USER_AGENT},
                                    {"Host", avaritia::common::url().API_ACCOUNT}
                                };
                                launcherAPI.set_default_headers(postHeader);
                                response = launcherAPI.Post("/api/content/login/state",data.dump(),"application/json");
                                responseJson = nlohmann::json::parse(response->body);
                                spdlog::debug(nlohmann::json(postHeader).dump(4));
                                spdlog::debug(response->body);
                                if(responseJson["error_code"] == 0){
                                    std::string playCode = responseJson["code"];
                                    spdlog::info("Starting up UNCHEATER");
                                    PROCESS_INFORMATION uc_pi = startup(uncheater.c_str(),"",1);
                                    // BLUEPROTOCOL.exe .code <PlayCode>
                                    // ldr.exe --shortcut %1
                                    spdlog::info("Starting up BLUEPROTOCOL!");
                                    std::string bp = GAME_PATH + "\\BLUEPROTOCOL.exe";
                                    PROCESS_INFORMATION game_pi = startup(bp.c_str(),playCode,0);
                                    // Close process and thread handles.
                                    DWORD exitCode;
                                    GetExitCodeProcess(game_pi.hProcess, &exitCode);
                                    WaitForSingleObject(game_pi.hProcess,INFINITE);
                                    TerminateProcess(uc_pi.hProcess, exitCode);
                                    // }
                                    CloseHandle( game_pi.hProcess );
                                    CloseHandle( game_pi.hThread );
                                    CloseHandle( uc_pi.hProcess );
                                    CloseHandle( uc_pi.hThread );
                                }
                            }
                        }
                    }
                    break;
            }
        }
        else{
            spdlog::info("Error : {}","placehodler");
        }
    }
    return 0;
}

#endif // __AVARITIA__MAIN
