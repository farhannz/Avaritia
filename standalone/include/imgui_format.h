#pragma once
 
#include <string>
#include <fmt/format.h>
#include <utility>
 
namespace ImGui
{
    template <typename T, typename... Args>
    IMGUI_API void  TextFmt(T&& fmt, const Args &... args) {
        std::string str = fmt::format(std::forward<T>(fmt), args...);
        ImGui::TextUnformatted(&*str.begin(), &*str.end());
    }
}