#include <avaritia/request.h>
#include <string>
#include <avaritia/MurmurHash3.hpp>
#include <spdlog/spdlog.h>




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
        return std::string(result);
    }


    std::string avaritia::Request::GET(std::string url){
        cpr::Response response = cpr::Get(cpr::Url(url));
        if(response.status_code == cpr::status::HTTP_OK){
            auto responseJson = nlohmann::json::parse(response.text);
            return responseJson.dump();
        }
        return response.text;
    }

    std::string avaritia::Request::getLoginToken(LoginRequest request, std::string deviceId, std::string accId, std::string ocean_session){
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
            std::string ocean_hash = getOceanHash(ocean_session,param);
            currentHeader["Cookie"] = "ocean_session"+ocean_session+";"+"ocean_hash="+ocean_hash;
        }
        currentHeader["Content-Type"] = "application/json";
        currentHeader["Origin"] =  std::string("https://").append(API_LAUNCHER);
        std::string endpoint = (request == Launcher) ? "/api/login/url" : "/api/content/login/url";
        cpr::Response resp = cpr::Post(
                cpr::Url{API_LAUNCHER + endpoint},
                cpr::Body{param.dump()},
                currentHeader);
        if(resp.status_code != cpr::status::HTTP_OK) return "";
        nlohmann::json responseJson = nlohmann::json::parse(resp.text);
        if(responseJson["error_code"]==0){
            return responseJson["access_token"];
        }
        throw std::runtime_error("Undefined request");
    }
}