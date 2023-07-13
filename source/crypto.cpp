#include <avaritia/crypto.h>
#include <stdexcept>

namespace avaritia
{
    avaritia::Crypto::Crypto(){
        if(sodium_init() == -1) throw std::runtime_error("Failed on initializing Sodium");

    }
} // namespace avaritia
