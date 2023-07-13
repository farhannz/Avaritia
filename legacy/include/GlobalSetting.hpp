#if !defined(__AVARITIA__GLOBAL__SETTING_HPP)
#define __AVARITIA__GLOBAL__SETTING_HPP
#include <combaseapi.h>
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
extern byte SECRET_SALT[] = "ThisIsReallyASecretSalt!@#";
// #include <ofstream>
namespace avaritia
{
    namespace setting{



        // Declaration
        struct AccountInfo{
            std::string email;
            std::string password;
        };
        class GlobalSetting{
            private:
            std::string m_guid;
            std::string m_gamepath;
            bool m_isAutoLogin;
            AccountInfo m_accountInfo; // encrypted
            std::set<std::string, std::pair<std::string,std::string>> m_accountList;
            public:
            GlobalSetting();
            ~GlobalSetting() = default;
            void setGamePath(std::string path);
            void setGuId(std::string guidstring);
            void setAccountinfo(std::string email, std::string password);
            void setAccountList(std::set<std::string, std::pair<std::string,std::string>> accountList);
            std::string getGuId();
            std::string getGamePath();
            AccountInfo getAccountInfo(); //
            std::set<std::string, std::pair<std::string,std::string>> getAccountList();
            void saveConfigToFile();
            bool isConfigFileExist();
            bool loadConfig();
            void setAutoLogin(bool login);
            bool isAutoLogin();
        };
        class DeviceID{
            GUID guid;
            public:
            DeviceID();
            ~DeviceID() = default;
            GUID getGuid();
            std::string ToString();
        };
        // definition

        avaritia::setting::GlobalSetting::GlobalSetting(){
            this->m_isAutoLogin = false;
        }
        void avaritia::setting::GlobalSetting::setGamePath(std::string path){
            this->m_gamepath = path;
        }
        void avaritia::setting::GlobalSetting::setGuId(std::string guid){
            this->m_guid = guid;
        }
        void avaritia::setting::GlobalSetting::setAccountinfo(std::string email, std::string password){
            this->m_accountInfo.email = email;
            this->m_accountInfo.password = password;
        }

        std::string avaritia::setting::GlobalSetting::getGamePath(){
            return this->m_gamepath;
        }
        std::string avaritia::setting::GlobalSetting::getGuId(){
            return this->m_guid;
        }
        AccountInfo avaritia::setting::GlobalSetting::getAccountInfo(){
            return this->m_accountInfo;
        }

        std::set<std::string, std::pair<std::string,std::string>> avaritia::setting::GlobalSetting::getAccountList(){
            return this->m_accountList;
        }

        void avaritia::setting::GlobalSetting::setAccountList(std::set<std::string, std::pair<std::string,std::string>> accountList){
            this->m_accountList = accountList;
        }
        
        void avaritia::setting::GlobalSetting::saveConfigToFile(){
            nlohmann::json file{
                {"device_id", this->m_guid},
                {"auto_login", this->m_isAutoLogin},
                {"gamepath", this->m_gamepath}
            };
            nlohmann::json accounts;
            std::set<std::string, std::pair<std::string,std::string>>::iterator it;
            std::ofstream of("config.json");
            of << file;
            of.flush();
            of.close();
        }

        bool avaritia::setting::GlobalSetting::loadConfig(){
            std::ifstream file("config.json");
            if(!file) return false;
            nlohmann::json config = nlohmann::json::parse(file);
            file.close();
            this->m_guid=config["device_id"];
            this->m_gamepath = config["gamepath"];
            this->m_isAutoLogin = config["auto_login"];
            return true;
        }

        bool avaritia::setting::GlobalSetting::isConfigFileExist(){
            try{
                return std::filesystem::exists("config.json");
            }
            catch(...){
                return false;
            }
        }

        void avaritia::setting::GlobalSetting::setAutoLogin(bool val){
            this->m_isAutoLogin = val;
        }
        bool avaritia::setting::GlobalSetting::isAutoLogin(){
            return this->m_isAutoLogin;
        }


        avaritia::setting::DeviceID::DeviceID(){
            HRESULT hr = CoCreateGuid(&this->guid);
            assert(hr == S_OK);
        }

        GUID avaritia::setting::DeviceID::getGuid(){
            return this->guid;
        }

        std::string avaritia::setting::DeviceID::ToString(){
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
    }
}; // namespace avaritia


#endif // __AVARITIA__GLOBAL__SETTING_HPP

