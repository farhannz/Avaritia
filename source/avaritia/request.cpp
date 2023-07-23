#include <avaritia/request.h>
#include <string>
#include <MurmurHash3.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
std::string getOceanHash(const std::string &ocean_session, const nlohmann::json &data){
    uint32_t seed = 2929;
    std::string s = data.dump() + "@@@" + ocean_session;
    // if(DEBUG_MODE) std::cout << data.dump() << std::endl;
    // quill::Logger* logger = quill::get_logger();
    // LOG_DEBUG(logger, "(getOceanHash) {}", s);
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
    spdlog::debug("(getOceanHash) {} {}", s, result);
    return std::string(result);
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

namespace avaritia{
    
    const std::string USER_AGENT      = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) bandai_namco_launcher/2.0.24 Chrome/78.0.3904.113 Electron/7.1.2 Safari/537.36";
    const std::string API_LAUNCHER    = "api-bnolauncher.bandainamco-ol.jp";
    const std::string API_ACCOUNT     = "account-api.bandainamcoid.com";
    const cpr::Header launcherHeader  = cpr::Header{{"User-Agent", USER_AGENT}, {"Host",API_LAUNCHER}};
    const cpr::Header accountHeader   = cpr::Header{{"User-Agent", USER_AGENT}, {"Host",API_ACCOUNT}};
    const std::string BPCuID          = "ELrmVeJRPRZMwrerwsJWkTxpKQABgNw3";

    class MasterDataInterceptor : public cpr::Interceptor{
        public:
            cpr::Response intercept(cpr::Session& session) override {
            // Log the request URL
            spdlog::debug("Requets url : {}", session.GetFullRequestUrl());
            // Proceed the request and save the response
            cpr::Response response = proceed(session);
            // Log response status code
            spdlog::debug("Response status code: {}",response.status_code);
            // Return the stored response
            return response;
        }
    };
    std::string avaritia::Request::GET(std::string url){
        cpr::Response response = cpr::Get(cpr::Url(url));
        if(response.status_code == cpr::status::HTTP_OK){
            auto responseJson = nlohmann::json::parse(response.text);
            return responseJson.dump();
        }
        return response.text;
    }

    LoginTokenArgs avaritia::Request::getLoginToken(LoginTokenArgs args){
        spdlog::set_level(spdlog::level::debug);
        std::string ocean_session, ocean_hash;
        for(auto &c : args.cookies){
            spdlog::debug("{} : {}",c.GetName(), c.GetValue());
            if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
        }
        spdlog::debug("(getLoginToken) Input {}, {}, {}",args.accId,args.deviceId, ocean_session);
        cpr::Header currentHeader = launcherHeader;
        currentHeader["Content-Type"] = "application/json";
        currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        nlohmann::json param{
            {"acc_id",""},
            {"auto_login","true"},
            {"debug_user","false"},
            {"device_id",args.deviceId}
        };
        if(args.request == Content){
             param = {
                {"acc_id", args.accId},
                {"auto_login",true},
                {"cuid", BPCuID}
            };
            ocean_hash = getOceanHash(ocean_session,param);
            currentHeader = cpr::Header{
                {"User-Agent", USER_AGENT}, 
                {"Host",API_LAUNCHER},
                {"Content-Type", "application/json"},
                {"Origin", "https://" + API_LAUNCHER}
            };
            spdlog::debug("(getLoginToken) {} {}", ocean_session, ocean_hash);
        }
        
        std::string endpoint = (args.request == Launcher) ? "/api/login/url" : "/api/content/login/url";
        spdlog::debug("{}", endpoint);
        cpr::Response resp = cpr::Post(
                cpr::Url{API_LAUNCHER + endpoint},
                cpr::Body{param.dump()},
                cpr::Cookies{
                    {"ocean_session",ocean_session},
                    {"ocean_hash",ocean_hash}},
                currentHeader);
        spdlog::debug("{}", resp.text);
        if(resp.status_code != cpr::status::HTTP_OK){
            args.returnValue = "";
            return args;
        }
        args.cookies = resp.cookies;
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(responseJson["error_code"]==0){
            args.returnValue = responseJson["access_token"];
            return args;
        }
        throw std::runtime_error("Undefined request");
    }

    LoginCodeArgs avaritia::Request::getLoginCode(LoginCodeArgs args){
        // spdlog::set_level(spdlog::level::info);
        cpr::Header currentHeader = accountHeader;
        // currentHeader["Content-Type"] = "aplication/x-www-form-urlencoded";
        // currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        nlohmann::json paramJson{ // aplication/x-www-form-urlencoded
            {"client_id","bno_ocean"},
            {"redirect_uri",std::string("https://") + API_LAUNCHER + "/api/login/bana/result/"+ args.accessToken},
            {"customize_id","launcher"},
            {"login_id",args.email},
            {"password",args.password},
            {"language","en"},
            {"shortcut",0},
            {"retention","\0"},
            {"cookie",nlohmann::json::parse("{}")},
            {"prompt","\0"}
        };
        if(args.request == Content){
            paramJson["client_id"] = "bp_service";
            paramJson["customize_id"] = "";
            paramJson["redirect_uri"] = std::string("https://") + API_LAUNCHER + "/api/content/bana/result/"+ args.accessToken;
        }
        // spdlog::debug("{}",jsonToQueryString(paramJson));
        std::string endpoint = "/v3/login/idpw";
        cpr::Response resp = cpr::Post(
                cpr::Url{"https://" + API_ACCOUNT + endpoint},
                cpr::Body{jsonToQueryString(paramJson)},
                currentHeader);
        spdlog::debug("{}", resp.text);
        if(resp.status_code != cpr::status::HTTP_OK) {
            args.returnValue = "";
            return args;    
        }
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(responseJson.find("input_error")!= responseJson.end()){
            args.returnValue = "wrong";
            return args;
        }
        if(responseJson.find("redirect")!=responseJson.end()){
            currentHeader.erase("Content-Type");
            currentHeader.erase("Origin");
            resp = cpr::Get(
                cpr::Url{responseJson["redirect"]},
                currentHeader);
            spdlog::debug("{}", resp.text);
            if(resp.status_code != cpr::status::HTTP_OK){
                args.returnValue = "";
                return args;
            }
            args.returnValue = splitByDelimiter(responseJson["redirect"], '=').back();
            return args;
        }
        throw std::runtime_error("Undefined request");
    }
    LoginStateArgs avaritia::Request::getLoginState(LoginStateArgs args){
        cpr::Header currentHeader = launcherHeader;
        std::string ocean_session, ocean_hash;
        for(auto &c : args.cookies){
            if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
        }
        currentHeader["Content-Type"] = "application/json";
        currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        nlohmann::json param = {
                {"acc_id",""},
                {"access_token",args.accessToken},
        };
        std::string endpoint = (args.request == Launcher) ? "/api/login/state" : "/api/content/login/state";
        if(args.request == Content){
            ocean_hash = getOceanHash(ocean_session,param);
            currentHeader = cpr::Header{
                {"User-Agent", USER_AGENT}, 
                {"Host",API_LAUNCHER},
                {"Content-Type", "application/json"},
                {"Origin", "https://" + API_LAUNCHER}
            };
        }
        cpr::Response resp = cpr::Post(
                cpr::Url{API_LAUNCHER + endpoint},
                cpr::Body{param.dump()},
                cpr::Cookies{
                    {"ocean_session",ocean_session},
                    {"ocean_hash",ocean_hash}},
                currentHeader);
        spdlog::debug("{}", resp.text);
        spdlog::debug("{}", resp.status_code);
        if(resp.status_code != cpr::status::HTTP_OK){
            args.returnValue = LoginState::LoginError;
            return args;
        }
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(args.request != Content){
            args.accId_out = responseJson["acc_id"];
        }
        else{
            args.playCode = responseJson["code"];
        }
        args.cookies = resp.cookies;
        args.returnValue = static_cast<LoginState>(responseJson["login_state"]);
        return args;
        // throw std::runtime_error("Undefined request");
    }

    AuthArgs avaritia::Request::sendAuthCode(AuthArgs args){
        cpr::Header currentHeader = launcherHeader;
        std::string ocean_session;
        for(auto &c : args.cookies){
            if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
        }
        currentHeader["Content-Type"] = "application/json";
        nlohmann::json param = {
            {"acc_id", args.acc_id},
            {"auth_code", args.auth_code},
            {"device_id", args.deviceId}
        };
        // currentHeader["Cookie"] = "ocean_session="+ocean_session+";"+"ocean_hash="+getOceanHash(ocean_session, param);
        std::string endpoint = "/api/login/authcode/check";
        cpr::Response resp = cpr::Post(
            cpr::Url{"https://"+API_LAUNCHER+endpoint},
             cpr::Body{param.dump()},
             cpr::Cookies{
                    {"ocean_session",ocean_session},
                    {"ocean_hash",getOceanHash(ocean_session, param)}},
             currentHeader
             );
        if(resp.status_code != cpr::status::HTTP_OK) throw std::runtime_error("Failed on sending authentication code");
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        // Will return false if success, and true if failed on authentication
        args.cookies = resp.cookies;
        args.returnValue = false;
        if(responseJson["error_code"] == 0 && responseJson["failed_count"]==0) return args;
        args.returnValue = true;
        return args;
    }

    std::string avaritia::Request::getLoginToken(LoginRequest request, std::string deviceId, std::string ocean_session,cpr::Cookies &cookie_out,  std::string accId){
        spdlog::set_level(spdlog::level::debug);
        cpr::Header currentHeader = launcherHeader;
        nlohmann::json param{
            {"acc_id",""},
            {"auto_login","true"},
            {"debug_user","false"},
            {"device_id",deviceId}
        };
        if(request == Content){
             param = {
                {"acc_id", accId},
                {"auto_login",true},
                {"cuid", BPCuID}
            };
            for(auto &c : cookie_out){
                if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
            }
            std::string ocean_hash = getOceanHash(ocean_session,param);
            currentHeader = cpr::Header{
                {"User-Agent", USER_AGENT}, 
                {"Host",API_LAUNCHER},
                {"Cookie", "ocean_session="+ocean_session+";"+"ocean_hash="+ocean_hash},
                {"Origin", "https://" + API_LAUNCHER}
            };
        }
        spdlog::debug("(getLoginToken) {}", ocean_session);
        currentHeader["Content-Type"] = "application/json";
        currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        std::string endpoint = (request == Launcher) ? "/api/login/url" : "/api/content/login/url";
        cpr::Response resp = cpr::Post(
                cpr::Url{API_LAUNCHER + endpoint},
                cpr::Body{param.dump()},
                currentHeader);
        spdlog::debug("{}", resp.text);
        if(resp.status_code != cpr::status::HTTP_OK) return "";
        for(auto &c : resp.cookies){
            spdlog::debug("(getLoginToken) {} {} ", c.GetName() , c.GetValue());
            if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
        }
        cookie_out = resp.cookies;
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(responseJson["error_code"]==0){
            cookie_out = resp.cookies;
            spdlog::debug("(getLoginToken) {}", ocean_session);
            return responseJson["access_token"];
        }
        throw std::runtime_error("Undefined request");
    }

    std::string avaritia::Request::getLoginCode(LoginRequest request, std::string email, std::string password, std::string accessToken){
        // spdlog::set_level(spdlog::level::info);
        cpr::Header currentHeader = accountHeader;
        // currentHeader["Content-Type"] = "aplication/x-www-form-urlencoded";
        // currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        nlohmann::json paramJson{ // aplication/x-www-form-urlencoded
            {"client_id","bno_ocean"},
            {"redirect_uri",std::string("https://") + API_LAUNCHER + "/api/login/bana/result/"+ accessToken},
            {"customize_id","launcher"},
            {"login_id",email},
            {"password",password},
            {"language","en"},
            {"shortcut",0},
            {"retention","\0"},
            {"cookie",nlohmann::json::parse("{}")},
            {"prompt","\0"}
        };
        if(request == Content){
            paramJson["client_id"] = "bp_service";
            paramJson["customize_id"] = "";
            paramJson["redirect_uri"] = std::string("https://") + API_LAUNCHER + "/api/content/bana/result/"+ accessToken;
        }
        // spdlog::debug("{}",jsonToQueryString(paramJson));
        std::string endpoint = "/v3/login/idpw";
        cpr::Response resp = cpr::Post(
                cpr::Url{"https://" + API_ACCOUNT + endpoint},
                cpr::Body{jsonToQueryString(paramJson)},
                currentHeader);
        spdlog::debug("{}", resp.text);
        if(resp.status_code != cpr::status::HTTP_OK) return "";
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(responseJson.find("input_error")!= responseJson.end()) return "wrong";
        if(responseJson.find("redirect")!=responseJson.end()){
            currentHeader.erase("Content-Type");
            currentHeader.erase("Origin");
            resp = cpr::Get(
                cpr::Url{responseJson["redirect"]},
                currentHeader);
            spdlog::debug("{}", resp.text);
            if(resp.status_code != cpr::status::HTTP_OK) return "";
            return splitByDelimiter(responseJson["redirect"], '=').back();    
        }
        throw std::runtime_error("Undefined request");
    }
    LoginState avaritia::Request::getLoginState(LoginRequest request, std::string accessToken, cpr::Cookies &cookie_out, std::string &accId_out, std::string &playCode){
        cpr::Header currentHeader = launcherHeader;
        currentHeader["Content-Type"] = "application/json";
        currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        nlohmann::json param = {
                {"acc_id",""},
                {"access_token",accessToken},
        };
        std::string endpoint = (request == Launcher) ? "/api/login/state" : "/api/content/login/state";
        if(request == Content){
            std::string ocean_session;
            for(auto &c : cookie_out){
                if(c.GetName() == "ocean_session") ocean_session = c.GetValue();
            }
            std::string ocean_hash = getOceanHash(ocean_session,param);
            currentHeader = cpr::Header{
                {"User-Agent", USER_AGENT}, 
                {"Host",API_LAUNCHER},
                {"Cookie", "ocean_session="+ocean_session+";"+"ocean_hash="+ocean_hash},
                {"Content-Type", "application/json"},
                {"Origin", "https://" + API_LAUNCHER}
            };
        }
        cpr::Response resp = cpr::Post(
                cpr::Url{API_LAUNCHER + endpoint},
                cpr::Body{param.dump()},
                currentHeader);
        spdlog::debug("{}", resp.text);
        spdlog::debug("{}", resp.status_code);
        for(auto &c : resp.cookies){
            spdlog::debug("{} : {}", c.GetName(), c.GetValue());
        }
        if(resp.status_code != cpr::status::HTTP_OK) return LoginState::LoginError;
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(request != Content){
            accId_out = responseJson["acc_id"];
        }
        else{
            playCode = responseJson["code"];
        }
        cookie_out = resp.cookies;
        return static_cast<LoginState>(responseJson["login_state"]);
        // throw std::runtime_error("Undefined request");
    }

    bool avaritia::Request::sendAuthCode(const std::string &acc_id, const std::string &auth_code, const std::string &deviceId, const std::string &ocean_session, cpr::Cookies &cookie_out){
        cpr::Header currentHeader = launcherHeader;
        currentHeader["Content-Type"] = "application/json";
        nlohmann::json param = {
            {"acc_id", acc_id},
            {"auth_code", auth_code},
            {"device_id", deviceId}
        };
        currentHeader["Cookie"] = "ocean_session="+ocean_session+";"+"ocean_hash="+getOceanHash(ocean_session, param);
        std::string endpoint = "/api/login/authcode/check";
        cpr::Response resp = cpr::Post(
            cpr::Url{"https://"+API_LAUNCHER+endpoint},
             cpr::Body{param.dump()},
             currentHeader
             );
        if(resp.status_code != cpr::status::HTTP_OK) throw std::runtime_error("Failed on sending authentication code");
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        // Will return false if success, and true if failed on authentication
        cookie_out = resp.cookies;
        if(responseJson["error_code"] == 0 && responseJson["failed_count"]==0) return false;
        return true;
    }
    
}