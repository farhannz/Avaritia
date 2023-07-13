#include <iostream>
#include <string>
#include <avaritia/crypto.h>
#include <avaritia/request.h>

auto main(int argc, char** argv) -> int {
  
  avaritia::Crypto test = avaritia::Crypto();
  std::cout << "Hello world\n";
  std::cout << avaritia::Request::GET("http://ip-api.com/json");
  return 0;
}
