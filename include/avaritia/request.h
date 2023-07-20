#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace avaritia{
    enum LoginState{
        CheckAuth   =1,
        Unknonw_1   =2,
        Success     =3,
        Unknown_2   =4
    };

    enum LoginRequest{
        Launcher,
        Content
    };
    class Request{
        private:
        static std::string getOceanHash(const std::string &ocean_session, const nlohmann::json &data);
        public:
        static std::string getDefaultUserAgent();
        static std::string getLauncherApiUrl();
        static std::string getAccountApiUrl();
        static std::string getDefaultHeader(char c);
        static std::string GET(std::string url);
        static std::string POST(std::string url);
        static std::string PUT(std::string url);
        static std::string getLoginToken(LoginRequest request, std::string deviceId, std::string accId="", std::string ocean_session = "");
    };
}