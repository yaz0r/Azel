rm -rf ios
mkdir ios 
rm -rf tvos
mkdir tvos

(cd ios && cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=iOS)
(cd tvos && cmake -G Xcode ../.. -DCMAKE_SYSTEM_NAME=tvOS)

