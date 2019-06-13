xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -sdk iphoneos
xcodebuild -project ../ThirdParty/SDL2/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -sdk iphonesimulator

mkdir ios 
cd ios 
cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=iOS
