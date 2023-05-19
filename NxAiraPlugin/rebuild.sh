rm -rf build
mkdir build
cd build
cmake ..
cd ..

cmake --build ./build/. --config Release
