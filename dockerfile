FROM debian:stable-slim

# Установка MinGW-w64 и инструментов сборки
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-mingw-w64-x86-64 \
    cmake \
    make \
    git \
    ca-certificates \
    libssl-dev   # нужно для libdatachannel (OpenSSL для Windows будет загружен отдельно)

# Удаляем лишние файлы списков пакетов для уменьшения размера
RUN rm -rf /var/lib/apt/lists/*

# Создаём toolchain-файл
COPY toolchain-mingw64.cmake /scripts/toolchain-mingw64.cmake

# Сборка libdatachannel для Windows (с MinGW)
RUN git clone --depth 1 https://github.com/paullouisageneau/libdatachannel.git /tmp/libdatachannel && \
    cd /tmp/libdatachannel && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/scripts/toolchain-mingw64.cmake \
             -DCMAKE_INSTALL_PREFIX=/usr/local/mingw && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/libdatachannel

# Устанавливаем nlohmann_json (header-only, просто копируем)
RUN git clone --depth 1 https://github.com/nlohmann/json.git /tmp/json && \
    cp -r /tmp/json/single_include/nlohmann /usr/local/mingw/include/ && \
    rm -rf /tmp/json

WORKDIR /workspace
