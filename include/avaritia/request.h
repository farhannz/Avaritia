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
    class Request{
        public:
        static std::string GET(std::string url);
        static std::string POST(std::string url);
        static std::string PUT(std::string url);
        
    };
}