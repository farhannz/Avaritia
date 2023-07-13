#include <avaritia/request.h>


namespace avaritia{

    std::string avaritia::Request::GET(std::string url){
        cpr::Response response = cpr::Get(cpr::Url(url));
        if(response.status_code == cpr::status::HTTP_OK){
            auto responseJson = nlohmann::json::parse(response.text);
            return responseJson.dump();
        }
        return response.text;
    }
}