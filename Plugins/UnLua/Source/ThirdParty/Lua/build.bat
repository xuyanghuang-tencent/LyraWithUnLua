rmdir /s /q "lua-5.4.3/build" &mkdir "lua-5.4.3/build" &pushd "lua-5.4.3/build"
cmake ../..  -DLUA_VERSION="5.4.3" -DLUA_COMPILE_AS_CPP="0" -DCMAKE_TOOLCHAIN_FILE="D:\env\SCE\Prospero\Tools\CMake\PS5.cmake" -DPS5="1"
popd
cmake --build lua-5.4.3/build --config Release
pause