mkdir apple_tools
cd apple_tools
cmake ..\..\ThirdParty\bgfx.cmake -DBGFX_BUILD_EXAMPLES=OFF -DBGFX_INSTALL=OFF
make shaderc
cd ..

rm -rf ios
mkdir ios 
rm -rf tvos
mkdir tvos
rm -rf osx
mkdir osx
(cd ios && cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=iOS)
(cd tvos && cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=tvOS)
(cd osx && cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=Darwin)

