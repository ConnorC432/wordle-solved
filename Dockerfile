FROM ubuntu:22.04
LABEL authors="connor"

RUN DEBIAN_FRONTEND="noninteractive" apt update && apt upgrade -y

RUN DEBIAN_FRONTEND="noninteractive" apt update \
    && apt install -y build-essential \
        gcc \
        g++ \
        gdb \
        clang \
        make \
        ninja-build \
        cmake \
        autoconf \
        automake \
        libtool \
        valgrind \
        locales-all \
        dos2unix \
        rsync \
        tar \
        python3 \
        python3-dev \
    && apt clean

WORKDIR /app