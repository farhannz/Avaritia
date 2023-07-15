#include <iostream>
#include <string>
#include <avaritia/crypto.h>
#include <avaritia/request.h>
#include <application.h>

auto main(int argc, char** argv) -> int {
  AvaritiaLauncher mainApp = AvaritiaLauncher("Avaritia Launcher", 600, 300);
  mainApp.Init();
  
  mainApp.Run();

  mainApp.~AvaritiaLauncher();
  return 0;
}
