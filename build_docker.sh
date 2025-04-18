#!/usr/bin/env bash

DOCKER_IMAGE_NAME=acbotics_interface

BUILD_ALL=false
BUILD_OS=()
BUILD_ARCH=''
BUILD_SINGLE_ARCH=false

BUILD_OS_DEFAULT=('debian_bookworm')
BUILD_ARCH_DEFAULT='linux/arm64'

USE_CACHE=true

# 1- HANDLE ARGS
# ==============
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
        echo $0 " [OPTIONS]"
        echo "Options:"
        echo "  --help, -h"
        echo ""
        echo "  --no-cache, -nc, -n"
        echo "    Skip local registry cache"
        echo ""
        echo "  --arch-typ"
        echo "    Build typical architectures: amd64 (PC), arm64 (Raspberry Pi)"
        echo "  --os-typ"
        echo "    Build typical operating systems: Ubuntu 24.04, Debian Bookworm"
        echo ""
        echo "  --amd64, --amd, --x86"
        echo "  --arm64"
        echo "  --armv8"
        echo "  --armv7"
        echo "    Build for the specified architectures"
        echo ""
        echo "  --22"
        echo "  --24"
        echo "  --bookworm"
        echo "    Build for the specified operating systems (22, 24: Ubuntu xx.04)"
        exit 0

    elif [ "${ARGI}" = "--no-cache" -o "${ARGI}" = "-nc" -o "${ARGI}" = "-n"  ]; then
        USE_CACHE=false
    elif [ "${ARGI}" = "--arch-typ" ]; then
        BUILD_ARCH+=',linux/amd64'
        BUILD_ARCH+=',linux/arm64'
    elif [ "${ARGI}" = "--os-typ" ]; then
        BUILD_OS+=('ubuntu2404')
        BUILD_OS+=('debian_bookworm')

    elif [ "${ARGI}" = "--amd64" -o "${ARGI}" = "--amd" -o "${ARGI}" = "--x86" ]; then
        BUILD_ARCH+=',linux/amd64'
    elif [ "${ARGI}" = "--arm64" ]; then
        BUILD_ARCH+=',linux/arm64'
    elif [ "${ARGI}" = "--armv8" ]; then
        BUILD_ARCH+=',linux/arm/v8'
    elif [ "${ARGI}" = "--armv7" ]; then
        BUILD_ARCH+=',linux/arm/v7'

    elif [ "${ARGI}" = "--24" ]; then
        BUILD_OS+=('ubuntu2404')
    elif [ "${ARGI}" = "--22" ]; then
        BUILD_OS+=('ubuntu2204')
    elif [ "${ARGI}" = "--bookworm" ]; then
        BUILD_OS+=('debian_bookworm')

    fi
done

BUILD_ARCH="${BUILD_ARCH#,}"

if [ "${#BUILD_OS[@]}" -eq 0 ]; then
    BUILD_OS=(${BUILD_OS_DEFAULT[@]})
fi
if [ "${BUILD_ARCH}" = '' ]; then
    BUILD_ARCH=$BUILD_ARCH_DEFAULT
fi

if [ "$(echo $BUILD_ARCH | tr ',' '\n' | wc -l)" -eq "1" ]; then
    BUILD_SINGLE_ARCH=true
fi


# 2- SETUP ENV
# ============
git submodule update --init --recursive
# docker build -t acbotics/acsense:$(arch) .

if [[ ! $(docker buildx ls | grep multi-platform-builder ) ]]; then

    docker buildx create \
        --use --platform=linux/arm64,linux/amd64,linux/arm/v8,linux/arm/v7,linux/arm/v6 \
        --name multi-platform-builder \
        --driver-opt network=host \
        --config /etc/buildkit/buildkitd.toml \
        --bootstrap
    # docker buildx inspect --bootstrap

fi


# 3- EXECUTE BUILD LOOP
# =====================

DO_DOCKER_RESET=true
for _BUILD_OS in ${BUILD_OS[@]}; do
    if $DO_DOCKER_RESET; then
        echo "Resetting qemu links";
        docker run --rm --privileged multiarch/qemu-user-static:register --reset 2&> /dev/null
        sleep 2
        DO_DOCKER_RESET=false
    fi
    if [ "${_BUILD_OS}" = "ubuntu2204" ]; then
        echo "Setting qemu links for 22.04";
        docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes 2&> /dev/null
        sleep 2
        DO_DOCKER_RESET=true
    fi
    if ${USE_CACHE}; then
        CACHE='--cache-to=type=registry,ref=127.0.0.1:5000/'${DOCKER_IMAGE_NAME}'_'${_BUILD_OS}':build_cache'
        CACHE+=' --cache-from=type=registry,ref=127.0.0.1:5000/'${DOCKER_IMAGE_NAME}'_'${_BUILD_OS}':build_cache'
    else
        CACHE=''
    fi
    if $BUILD_SINGLE_ARCH; then
        echo "Building ${_BUILD_OS} : ${BUILD_ARCH}";
        docker buildx build \
            --platform $BUILD_ARCH \
            ${CACHE} \
            --output type=local,dest=dist/dist_${_BUILD_OS}/${BUILD_ARCH//\//_} \
            --file docker/${_BUILD_OS}.dockerfile \
            . \
            || exit 1
    else
        echo "Building ${_BUILD_OS} : ${BUILD_ARCH}";
        docker buildx build \
            --platform $BUILD_ARCH  \
            ${CACHE} \
            --output type=local,dest=dist/dist_${_BUILD_OS} \
            --file docker/${_BUILD_OS}.dockerfile \
            . \
            || exit 2
    fi
done

if $DO_DOCKER_RESET; then
    echo "Resetting qemu links";
    docker run --rm --privileged multiarch/qemu-user-static:register --reset 2&> /dev/null
    sleep 1
    DO_DOCKER_RESET=false
fi

echo "Done"
