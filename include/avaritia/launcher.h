#pragma once
#include <windows.h>
#include <string>
#include <spdlog/spdlog.h>

namespace avaritia{
    PROCESS_INFORMATION startup(LPCTSTR lpApplicationName, std::string playCode, int cases);
}