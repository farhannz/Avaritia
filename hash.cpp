#include "include/hash/MurmurHash3.hpp"
#include <iostream>
#include <string>
#include "build/_deps/json-src/single_include/nlohmann/json.hpp"
#include <cctype>
#include <windows.h>


std::string getOceanHash(std::string ocean_session, nlohmann::json data){
    uint32_t seed = 2929;

    std::string s = data.dump() + "@@@" + ocean_session;

    // if(DEBUG_MODE) std::cout << data.dump() << std::endl;
    // spdlog::debug("(getOceanHash) {}", s);
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

int main(int argc, char const *argv[])
{
    std::string ocean_session;
    nlohmann::json data;
    std::string jsonString;
    while(true){
        std::cout << "ocean session: ";
        std::cin >> ocean_session;
        std::cout << "jsonString: ";
        std::cin >> jsonString;
        data = nlohmann::json::parse(jsonString);
        std::cout << data.dump(4) << std::endl;
        std::cout << getOceanHash(ocean_session,data) << std::endl;
    }
    return 0;
}

// {"client_id":"bno_ocean","customize_id":"launcher","authcode":398913,"authcode_key":"","permit_token":"","cookie":"{}","language":"jp","backto":"https://account.bandainamcoid.com/authCode.html?client_id=idportal&redirect_uri=https%3A%2F%2Fwww.bandainamcoid.com%2Fv2%2Foauth2%2Fauth%3Fback%3Dv3%26client_id%3Didportal%26scope%3DJpGroupAll%26redirect_uri%3Dhttps%253A%252F%252Fwww.bandainamcoid.com%252F%26text%3D&customize_id="}
// 
