#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace avaritia{
    class Request{
        public:
        static std::string GET(std::string url);
    };
}