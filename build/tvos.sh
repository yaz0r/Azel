xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-tvOS -sdk appletvos
xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-tvOS -sdk appletvsimulator

mkdir tvos 
cd tvos
cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=tvOS
