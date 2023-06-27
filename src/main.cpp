#if !defined(__AVARITIA__MAIN)
#define __AVARITIA__MAIN

#include <GlobalSetting.hpp>
#include <Common.hpp>
#include <sstream>
#include <iostream>


int main(int argc, char const *argv[])
{
    avaritia::DeviceID deviceId = avaritia::DeviceID();
    std::cout << deviceId.ToString() << std::endl;
    
    return 0;
}

#endif // __AVARITIA__MAIN
