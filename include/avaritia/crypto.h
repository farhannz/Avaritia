#pragma once
#include <sodium.h>
#include <string>
namespace avaritia{
    class Crypto{
        
        public:
        Crypto();
        static std::string Encrypt(std::string);
        static bool Verify(std::string,std::string);
    };
}