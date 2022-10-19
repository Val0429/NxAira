cmake --build ./build/. --config Debug
call stop.bat
cp -f ./build/Debug/NxAiraPlugin.* "C:/Program Files/Network Optix/Nx Witness/MediaServer/plugins"
call start.bat