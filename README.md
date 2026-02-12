# Azel: Reverse engineering of Panzer Dragoon Saga

# Build

## Game data
Extract the contents of Panzer Dragoon Saga CD1 into a `data/` folder in the project root. The folder should contain the raw filesystem files from the disc (e.g. using a Saturn disc tool or mounting the ISO and copying files out).

## Windows
* Install Visual Studio 2022 with CMake support
* Clone with submodules: `git clone --recursive`
* Run `vs2022.bat` to generate the VS2022 solution
* Build and run the `PDS` project

## Linux
* Install dependencies: `cmake`, `build-essential`, `libgl-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`, `libxkbcommon-dev`, `libwayland-dev`
* Clone with submodules: `git clone --recursive`
* Build:
```
mkdir build_linux && cd build_linux
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```
* Run from the project root (so it can find `data/`): `./build_linux/PDS/PDS`

# Status
* Still very early, but the beginning of the game is semi playable (with bugs) until the Arachnoth battle

# Screenshots
![image](https://github.com/user-attachments/assets/71f6a246-24cc-4d64-9673-53b9e4c3e4df)
![image](https://github.com/user-attachments/assets/adbfa9b0-bb1a-483a-8b15-4c7fdc198131)
![image](https://github.com/user-attachments/assets/864375eb-976a-403f-8d45-3dad03f83491)
![image](https://github.com/user-attachments/assets/4ec9797e-8bc2-4a00-8584-412fc0089c50)


