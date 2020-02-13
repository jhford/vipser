ARG BASE=fedora:31

FROM "$BASE" AS build

# For playing nicely with apt-based distributions which still
# use yum.  For example, amazonlinux:2 and derived images
ARG DNF=dnf

ARG PREFIX=/usr/local
ARG DNF=dnf
ARG BUILD_TYPE=Release
#ARG CC=clang
#ARG CXX=clang++
ARG CC=gcc
ARG CXX=g++

# OS provided build dependencies
# TODO: Figure out why there's a duplicate symbol breaking
# the compilation when orc-devel is installed at build
RUN sudo "$DNF" install -y \
    {expat,glib2,libpng,libtiff,libjpeg-turbo,libexif,fftw,file}-devel \
    automake libtool diffutils file-libs file cmake make \
    clang clang gcc gcc-c++

# Control the build and install prefix for the dependencies
# and ensure that they get
ENV PKG_CONFIG_PATH="$PREFIX/lib64/pkgconfig:$PREFIX/lib/pkgconfig"
ENV CFLAGS="-O2"
ENV LDFLAGS="-Wl,-rpath,$PREFIX/lib64/"
ENV BUILDROOT=/tmp/buildroot

# Copy the build script for dependencies.  This is done
# first so that the build cache is only invalidated this
# early if the dependencies have changed.
COPY build-deps.sh build-deps.sh
RUN J=$(nproc) ./build-deps.sh

# Now copy the rest of the program source over
COPY . .

# Create the build system
RUN cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_COMPILER="$CXX" \
    -DCMAKE_CC_COMPILER="$CC"

# Build it!
RUN make "-j$(nproc)"

# The final container
FROM $BASE
ARG DNF=dnf
ARG PREFIX=/usr/local
COPY --from=build "$PREFIX" "$PREFIX"
RUN sudo "$DNF" install -y \
    {expat,glib2,libpng,libtiff,libjpeg-turbo,libexif,orc,fftw,file-libs}
RUN ldconfig