#include <avaritia/launcher.h>

namespace avaritia{

    PROCESS_INFORMATION startup(LPCTSTR lpApplicationName, std::string playCode, int cases)
    {
        // additional information
        STARTUPINFO si;     
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        std::string paramString = "";
        switch(cases){
            case 0:
                paramString = " -code ";
                paramString.append(playCode);
                break;
            default:
                paramString = " --shortcut %1";
                break;
        }
        spdlog::debug("{} {}", lpApplicationName, paramString);
        LPSTR param = const_cast<LPSTR>(paramString.c_str());
        // start the program up
        CreateProcess( lpApplicationName,   // the path
        param,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );
        return pi;
    }
}