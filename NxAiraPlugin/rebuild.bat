rm -rf build
mkdir build
cd build
cmake ..
cd ..

cmake --build ./build/.
call stop.bat
cp -f ./build/Debug/NxAiraPlugin.dll "C:/Program Files/Network Optix/Nx Witness/MediaServer/plugins"
call start.bat