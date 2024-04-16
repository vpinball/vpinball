FROM ubuntu:22.04

RUN apt-get update && apt-get upgrade -y 

# Upgrade CMake

RUN apt-get install -y ca-certificates gpg wget
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt-get update && apt-get install -y kitware-archive-keyring

# Install Oh My Zsh

RUN apt-get install -y zsh git curl && chsh -s $(which zsh)
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

# Install Dependencies

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y git unzip vim rsync build-essential autoconf automake libtool cmake nasm bison curl zlib1g-dev libdrm-dev libgbm-dev libglu1-mesa-dev libegl-dev libudev-dev libx11-dev libxrandr-dev

RUN mkdir -p /home/ubuntu && \
    { \
    echo '#!/bin/bash'; \
    echo 'set -e'; \
    echo 'USER=vpinball'; \
    echo 'BRANCH=standalone'; \
    echo 'git clone -b $BRANCH https://github.com/$USER/vpinball'; \
    echo 'cd vpinball/standalone/linux-x64'; \
    echo './external.sh'; \
    echo 'cd ../..'; \
    echo 'cp standalone/cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt'; \
    echo 'cmake -DCMAKE_BUILD_TYPE=Release -B build'; \
    echo 'cmake --build build -- -j$(nproc)'; \
    } > /home/ubuntu/build.sh && chmod +x /home/ubuntu/build.sh

WORKDIR /home/ubuntu
