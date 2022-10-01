cmake --build ./build/.
net stop "Network Optix Media Server"
cp -f ./build/Debug/NxAiraPlugin.dll "C:/Program Files/Network Optix/Nx Witness/MediaServer/plugins"
net start "Network Optix Media Server"