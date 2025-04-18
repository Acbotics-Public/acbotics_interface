#!/usr/bin/env bash

ROOTDIR=$(realpath $(dirname $0))

CLEAN=false

BUILD_DIR=_build
INSTALL_DIR=_install
BUILD_TYPE=Release
CMAKE_FLAGS=""

DO_LOCAL_INSTALL=true

for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
        echo $0 " [OPTIONS]"
        echo "Options:"
        echo "  --help, -h"
        echo "  --clean, -c, clean"
        echo "    Remove all output files from ${BUILD_DIR}, ${INSTALL_DIR}"
        echo "  --debug, -d, debug"
        echo "    Build in DEBUG mode"

        echo "  --apps"
        echo "    Build stand-alone apps"
        echo "  --tests"
        echo "    Build executable with basic tests"

        exit 0
    elif [ "${ARGI}" = "--clean" -o "${ARGI}" = "-c" -o "${ARGI}" = "clean" ]; then
        CLEAN=true
    elif [ "${ARGI}" = "--debug" -o "${ARGI}" = "-d" -o "${ARGI}" = "debug" ]; then
        BUILD_TYPE=Debug
    elif [ "${ARGI}" = "--system" ]; then
        DO_LOCAL_INSTALL=false

    elif [ "${ARGI}" = "--apps" ]; then
        CMAKE_FLAGS+=" -DBUILD_APPS=true"
    elif [ "${ARGI}" = "--tests" ]; then
        CMAKE_FLAGS+=" -DBUILD_TESTS=true"
    fi
done

if ${DO_LOCAL_INSTALL}; then
    CMAKE_FLAGS+=" -DCMAKE_INSTALL_PREFIX:PATH=${ROOTDIR}/${INSTALL_DIR}"
fi
CMAKE_FLAGS+=" -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"


if ${CLEAN}; then
    rm -rf ${BUILD_DIR}/*
    rm -rf ${INSTALL_DIR}/*
    exit 0
fi


git submodule init
git submodule update

if [ ! -d ${BUILD_DIR} ]; then
    mkdir -p ${BUILD_DIR}
fi


printf "Building with CMAKE_FLAGS : $(echo ${CMAKE_FLAGS} | sed 's/-D/\n - /g')\n\n"

pushd ${BUILD_DIR} >& /dev/null;
cmake ${CMAKE_FLAGS} .. || exit 1;
make -j$(nproc) || exit 1;
make install || exit 1;
popd >& /dev/null;

if $(python3 -c "import acbotics_interface") && $(command -v stubgen); then
    STUB_TARGET=$(python3 -c \
    "import acbotics_interface as ac; import os; print(os.path.dirname(ac.__file__))")
    stubgen -m acbotics_interface -o ${STUB_TARGET}
fi
