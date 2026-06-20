FROM debian:stable-slim


RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-mingw-w64-x86-64 \
    cmake \
    make \
    git \
    ca-certificates \
    libssl-dev   


RUN rm -rf /var/lib/apt/lists/*


COPY toolchain-mingw64.cmake /scripts/toolchain-mingw64.cmake

RUN git clone --depth 1 --branch openssl-3.3.1 https://github.com/openssl/openssl.git /tmp/openssl && \
    cd /tmp/openssl && \
    ./Configure mingw64 no-shared --prefix=/usr/local/mingw --cross-compile-prefix=x86_64-w64-mingw32- && \
    make -j$(nproc) && make install_sw && \
    rm -rf /tmp/openssl


RUN git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/paullouisageneau/libdatachannel.git /tmp/libdatachannel && \
    cd /tmp/libdatachannel && \
    mkdir build && cd build && \
    CFLAGS="-Wno-error" cmake .. \
      -DCMAKE_TOOLCHAIN_FILE=/scripts/toolchain-mingw64.cmake \
      -DCMAKE_INSTALL_PREFIX=/usr/local/mingw \
      -DOPENSSL_ROOT_DIR=/usr/local/mingw \
      -DOPENSSL_USE_STATIC_LIBS=ON \
      -DUSE_SYSTEM_PLOG=OFF -DUSE_SYSTEM_USRSCTP=OFF -DUSE_SYSTEM_SRTP=OFF \
      -DCMAKE_C_FLAGS="-Wno-error=format" \
      -DCMAKE_C_FLAGS="-Wno-error" \
      && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/libdatachannel


RUN git clone --depth 1 https://github.com/nlohmann/json.git /tmp/json && \
    cp -r /tmp/json/single_include/nlohmann /usr/local/mingw/include/ && \
    rm -rf /tmp/json

WORKDIR /workspace
