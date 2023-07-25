#include <iostream>
#include <string>
#include <avaritia/crypto.h>
#include <avaritia/request.h>
#include <application.h>
#include <spdlog/spdlog.h>
auto main(int argc, char** argv) -> int {
  AvaritiaLauncher mainApp = AvaritiaLauncher("Avaritia Launcher", 610, 320);
  spdlog::set_level(spdlog::level::debug);
  mainApp.Init();
  
  mainApp.Run();

  mainApp.Shutdown();
  // mainApp.~AvaritiaLauncher();
  return 0;
}
