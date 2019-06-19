xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-tvOS -sdk appletvos archive
xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-tvOS -sdk appletvsimulator archive

lipo -create ../ThirdParty/SDL2/Xcode-iOS/SDL/build/Release-appletvos/libSDL2.a ../ThirdParty/SDL2/Xcode-iOS/SDL/build/Release-appletvsimulator/libSDL2.a -output ../ThirdParty/SDL2/Xcode-iOS/SDL/build/libSDL2_appletv.a

mkdir tvos 
cd tvos
cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=tvOS
