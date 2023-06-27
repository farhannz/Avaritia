#if !defined(__AVARITIA__GLOBAL__SETTING_HPP)
#define __AVARITIA__GLOBAL__SETTING_HPP
#include <combaseapi.h>
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>
namespace avaritia
{
    class DeviceID{
        GUID guid;
        public:
        DeviceID();
        ~DeviceID() = default;

        GUID getGuid();
        std::string ToString();

    };

    avaritia::DeviceID::DeviceID(){
        HRESULT hr = CoCreateGuid(&this->guid);
        assert(hr == S_OK);
    }

    GUID avaritia::DeviceID::getGuid(){
        return this->guid;
    }

    std::string avaritia::DeviceID::ToString(){
        std::stringstream ss;
        std::string result;

        ss << std::setw(8) << std::setfill('0') << std::hex << this->guid.Data1;
        ss << std::setw(4) << std::setfill('0') << std::hex << this->guid.Data2;
        ss << std::setw(4) << std::setfill('0') << std::hex << this->guid.Data3;
        ss >> result;
        ss.clear();
        for(auto item : this->guid.Data4){
            std::string tmp;
            ss << std::setw(2) << std::setfill('0') << std::hex << u_int(item) << std::endl;
            ss >> tmp;
            ss.clear();
            result.append(tmp);
        }
        return result;
    }

}; // namespace avaritia


#endif // __AVARITIA__GLOBAL__SETTING_HPP

