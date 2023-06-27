#if !defined(AVARITIA_COMMON_HPP)
#define AVARITIA_COMMON_HPP
#include <string>

namespace avaritia{
    struct common{
        std::string USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) bandai_namco_launcher/2.0.24 Chrome/78.0.3904.113 Electron/7.1.2 Safari/537.36";
        std::string API_LAUNCHER = "https://api-bnolauncher.bandainamco-ol.jp";
        std::string API_ACCOUNT = "https://account-api.bandainamcoid.com";
    };
};

#endif // AVARITIA_COMMON_HPP
