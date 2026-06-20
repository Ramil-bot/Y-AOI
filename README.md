
## Сборка под Windows
```
mkdir build && cd build 
cmake .. \ 
  -DCMAKE_TOOLCHAIN_FILE="$(realpath ../vcpkg/scripts/buildsystems/vcpkg.cmake)" \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_BUILD_TYPE=Release
```

## Сборка под Linux 
`````
```
mkdir build && cd build
cmake ..
```
