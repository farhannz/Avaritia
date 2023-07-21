#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>



namespace avaritia{
    enum LoginState{
        CheckAuth   =1,
        Unknonw_1   =2,
        Success     =3,
        Unknown_2   =4,
        LoginError  =999
    };

    enum LoginRequest{
        Launcher,
        Content
    };
    class Request{
        // protected:
        public:
        static std::string getDefaultUserAgent();
        static std::string getLauncherApiUrl();
        static std::string getAccountApiUrl();
        static std::string getDefaultHeader(char c);
        static std::string GET(std::string url);
        static std::string POST(std::string url);
        static std::string PUT(std::string url);
        static std::string getLoginToken(LoginRequest request, std::string deviceId, std::string accId="", std::string ocean_session = "");
        static std::string getLoginCode(LoginRequest request, std::string email,  std::string password, std::string accessToken);
        static LoginState getLoginState(LoginRequest request, std::string accessToken, cpr::Cookies &out_cookies, std::string &out_accid);
        static bool sendAuthCode(const std::string &acc_id, const std::string &auth_code, const std::string &deviceId, const std::string &ocean_session);
    };
}