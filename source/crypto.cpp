#include <avaritia/crypto.h>
#include <avaritia/MurmurHash3.h>
#include <stdexcept>
#include <iostream>
namespace avaritia
{
    avaritia::Crypto::Crypto(){
        if(sodium_init() == -1) throw std::runtime_error("Failed on initializing Sodium");
    }

    std::string avaritia::Crypto::Encrypt(std::string password){
        char hashed_password[crypto_pwhash_STRBYTES];
        if(crypto_pwhash_str(hashed_password, const_cast<char*>(password.c_str()), password.size()+1, crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE) != 0){
            throw std::runtime_error("Out of memory");
        }
        return std::string(hashed_password);
    }

    bool avaritia::Crypto::Verify(std::string password, std::string hash){
        return (crypto_pwhash_str_verify(hash.c_str(), password.c_str(), password.size()+1) == 0); // true if success
    }
} // namespace avaritia
