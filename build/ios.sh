xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -sdk iphoneos
xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -sdk iphonesimulator

lipo -create ../ThirdParty/SDL2/Xcode-iOS/SDL/build/Release-iphoneos/libSDL2.a ../ThirdParty/SDL2/Xcode-iOS/SDL/build/Release-iphonesimulator/libSDL2.a -output ../ThirdParty/SDL2/Xcode-iOS/SDL/build/libSDL2_iphone.a

mkdir ios 
cd ios 
cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=iOS
