FROM ubuntu:22.04 AS base

ARG DEBIAN_FRONTEND=noninteractive

# For some reason, adding the linux/arm64 variant causes issues with libc-bin
# The patch below is based on the following Stack Overflow posts:
# https://stackoverflow.com/questions/78105004/docker-build-fails-because-unable-to-install-libc-bin
# https://stackoverflow.com/questions/73710118/trying-to-update-libc-bin-error-state-134-cant-sudo-install-net-tools/76260513#76260513
RUN rm /var/lib/dpkg/info/libc-bin.* \
  && apt clean \
  && apt update \
  && apt install -y apt-utils \
  && apt install -f \
  && dpkg --configure -a \
  && apt install -y libc-bin

RUN apt update \
  && apt upgrade -y

RUN apt install -y \
  make \
  cmake \
  build-essential \
  makeself \
  git \
  libgoogle-glog-dev \
  libsndfile1-dev \
  libgps-dev \
  libghc-zlib-dev \
  libjpeg-dev \
  python3-dev \
  python3-numpy \
  python3-scipy \
  python3-pip

RUN python3 -m pip install -U pip

RUN python3 -m pip install \
  Cython \
  netifaces \
  cryptocode \
  PyInstaller
  # mypy \ # installs stubgen for pyi (interface info file)

FROM base AS add_git

# Package version of Eigen tries to use CPU optimizations that may fail!
# Instead, we build from source for docker image
# RUN apt install -y \
#   libeigen3-dev
RUN git clone -b 3.4 https://gitlab.com/libeigen/eigen.git /tmp/eigen3
RUN  mkdir /tmp/eigen3/build
WORKDIR /tmp/eigen3/build
RUN cmake .. && make install
RUN ln -s /usr/local/include/eigen3/Eigen /usr/local/include/Eigen

WORKDIR /
COPY ./ /tmp/acbotics_interface
WORKDIR /tmp/acbotics_interface
RUN ./build.sh -c && PYTHONPATH=/tmp/acbotics_interface/_install/python ./build.sh
RUN cp -r /tmp/acbotics_interface/_install /acbotics_interface
ARG PYTHONPATH=/acbotics_interface/python

FROM scratch AS dist
# This "FROM scratch" stage allows exporting the final *.run file via
# local fs output mode in buildx, without preserving OS files that are
# no longer needed
# COPY --from=base /acbotics_interface /
COPY --from=add_git /acbotics_interface /
