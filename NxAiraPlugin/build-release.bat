cmake --build ./build/. --config Release
call stop.bat
cp -f ./build/Release/NxAiraPlugin.dll "C:/Program Files/Network Optix/Nx Witness/MediaServer/plugins"
call start.bat