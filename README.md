# AVARITIA   ![MinGW64 Build](https://github.com/farhannz/Avaritia/actions/workflows/mingw64.yml/badge.svg) ![MSVC Build](https://github.com/farhannz/Avaritia/actions/workflows/msvc.yml/badge.svg)


## An alternative launcher for a certain MMORPG developed by Japanese developers.   

### Why?
This project aims to add some QoL features, such as:
* Auto Login (WIP)
* Account Switcher (WIP)
* IP Geolocation
* etc

### Installation
- Download the latest version from the [releases](https://github.com/farhannz/Avaritia/releases/latest) page.
- Extract to any folder
- Run "Avaritia Launcher.exe"

#### For first time usage:
- Select your BP installation folder
- When you login, it will count as a new device so it will ask for auth code.

#### For updating the application:
Currently there is no auto updater for the application.
- Download the latest version from the [releases](https://github.com/farhannz/Avaritia/releases/latest) page.
- Extract to your old avaritia folder
- Run "Avaritia Launcher.exe"

⚠️ Antivirus programs might detect this program as a false positive. You can check for yourself on VirusTotal.

### Build Prerequisites
> VCPKG for dependencies management

```
git clone https://github.com/farhannz/Avaritia.git
cd Avaritia
```
For MSVC:   
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=${{env.BUILD_TYPE}} -DCMAKE_TOOLCHAIN_FILE=${{env.VCPKG_ROOT}}\\scripts\\buildsystems\\vcpkg.cmake
cmake --build build --config ${{env.BUILD_TYPE}}
cmake -S standalone -B build\\standalone -DCMAKE_BUILD_TYPE=${{env.BUILD_TYPE}} -DCMAKE_TOOLCHAIN_FILE=${{env.VCPKG_ROOT}}\\scripts\\buildsystems\\vcpkg.cmake
cmake --build build\\standalone --config ${{env.BUILD_TYPE}} 
```

For MinGW64:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=${{env.BUILD_TYPE}} -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -G Ninja
cmake --build build --config ${{env.BUILD_TYPE}}
cmake -S standalone -B build/standalone -DCMAKE_BUILD_TYPE=${{env.BUILD_TYPE}} -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -G Ninja
cmake --build build/standalone --config ${{env.BUILD_TYPE}} 
```
You will find the launcher at `standalone/build/bin`

### Disclaimer   
This is a learning project of the author of this repository and a community fan project that has nothing to do with the creators of the game. This launcher does not modify any files from the game client. Using this launcher may violate the TOS of the game.
