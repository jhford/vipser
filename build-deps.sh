#!/bin/bash
set -e

DEP_ROOT=$PWD/deps
START_DIR=$PWD/deps-build

export PKG_CONFIG_PATH="$DEP_ROOT/lib/pkgconfig:$DEP_ROOT/lib64/pkgconfig"

mkdir -p $START_DIR
cd $START_DIR

# TODO: libmagic
# TODO: libpng
# TODO: locked down imagemagick
VIPS_VER=8.9.1
ORC_VER=0.4.31
FFTW_VER=3.3.8
HEIF_VER=1.6.2
GIF_VER=5.2.1
EXIF_VER=0_6_21
JPEG_VER=2.0.4
WEBP_VER=1.1.0
MAGICK_VER=7.0.9-20

download() {
  outname="$1"
  shift
  if [ ! -f $outname ]; then
    url="$1"
    curl --fail -o "$outname" -L "$url"
  fi
}

install_pkgs() {
  sudo ${DNF:-dnf} install expat-static glib2-static libpng-static libtiff-static meson
}

install_orc() {
  cd "$START_DIR"
  download orc-${ORC_VER}.tar.gz "https://github.com/GStreamer/orc/archive/0.4.31.tar.gz"
  tar zxf orc-${ORC_VER}.tar.gz
  cd orc-${ORC_VER}
  rm -rf build
  meson build --prefix=$DEP_ROOT --buildtype release --strip --optimization 3 --default-library both
  (cd build && ninja && ninja install)
}

install_fftw3() {
  cd "$START_DIR"
  download fftw-${FFTW_VER}.tar.gz "http://www.fftw.org/fftw-3.3.8.tar.gz"
  tar zxf fftw-${FFTW_VER}.tar.gz
  cd fftw-${FFTW_VER}
  ./configure --prefix=$DEP_ROOT --enable-shared --enable-static
  make -s -j12
  make -s install
}

install_libexif() {
  download libexif-${EXIF_VER}-release.tar.gz  "https://github.com/libexif/libexif/archive/libexif-${EXIF_VER}-release.tar.gz"
  tar zxf libexif-${EXIF_VER}-release.tar.gz
  rm -rf libexif-${EXIF_VER}
  mv libexif-libexif-${EXIF_VER}-release libexif-${EXIF_VER}
  cd libexif-${EXIF_VER}
  autoreconf --install
  ./configure --enable-static --enable-shared --prefix=${DEP_ROOT}
  make -s -j12
  make -s install
}

install_giflib() {
  cd "$START_DIR"
  download giflib-${GIF_VER}.tar.gz "https://netix.dl.sourceforge.net/project/giflib/giflib-${GIF_VER}.tar.gz"
  tar zxf giflib-${GIF_VER}.tar.gz
  cd giflib-${GIF_VER}
  make -s -j12 libgif.a
  make install PREFIX=$DEP_ROOT LIBDIR=$DEP_ROOT/lib64
  install -m 644 libgif.a $DEP_ROOT/lib/
  install -m 644 gif_lib.h $DEP_ROOT/include/
}

install_libjpeg() {
  cd "$START_DIR"
  download libjpeg-turbo-${JPEG_VER}.tar.gz "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/${JPEG_VER}.tar.gz"
  tar zxf libjpeg-turbo-${JPEG_VER}.tar.gz
  cd libjpeg-turbo-${JPEG_VER}
  cmake -G"Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$DEP_ROOT \
    -DENABLE_STATIC:BOOL=YES \
    -DENABLE_SHARED:BOOL=YES
  make DESTDIR=$DEP_ROOT
  make install
}

install_libwebp() {
  cd "$START_DIR"
  download libwebp-${WEBP_VER}.tar.gz "https://chromium.googlesource.com/webm/libwebp/+archive/refs/heads/${WEBP_VER}.tar.gz"
  mkdir -p libwebp-${WEBP_VER}
  cd libwebp-${WEBP_VER}
  tar zxf ../libwebp-${WEBP_VER}.tar.gz
  ./autogen.sh
  ./configure --enable-static --enable-shared --prefix=$DEP_ROOT
  make -j12
  make install
}

install_magick() {
  cd "$START_DIR"
  download ImageMagick-${MAGICK_VER}.tar.gz "https://github.com/ImageMagick/ImageMagick/archive/${MAGICK_VER}.tar.gz"
  tar zxf ImageMagick-${MAGICK_VER}.tar.gz
  cd ImageMagick-${MAGICK_VER}
  ./configure \
    --prefix=${DEP_ROOT} \
    --enable-static \
    --enable-shared \
    --with-{magick-plus-plus,utilities,modules,fontconfig,freetype,pango,xml}=no \
    --with-modules=no \
    --with-quantum-depth=8 \
    --enable-hdri=no \
    --disable-installed \
    --without-threads \
    --with-{autotrace,djvu,dps,flif,fpx,gslib,gvc,heic,jbig,jpeg,jxl}=no \
    --with-{lqr,ltdl,openexr,openjp2,png,raqm,rsvg,raw,tiff,webp,wmf,x}=no

  make -s -j12
  make install
}

install_vips() {
  cd "$START_DIR"
  download vips-${VIPS_VER}.tar.gz "https://github.com/libvips/libvips/releases/download/v${VIPS_VER}/vips-${VIPS_VER}.tar.gz"
  tar zxf vips-${VIPS_VER}.tar.gz

  cd vips-${VIPS_VER}
  PKG_CONFIG_PATH=$PKG_CONFIG_PATH ./configure \
    --prefix="$DEP_ROOT" \
    --enable-shared \
    --disable-static \
    --without-{x,heif,OpenEXR,pdfium,poppler,rsvg,matio,ppm,pangoft2,python} \
    --with-magick \
    --with-{libwebp,tiff,giflib,png,jpeg,libexif} \
    --with-giflib-includes=$DEP_ROOT/include/ \
    --with-giflib-libraries=$DEP_ROOT/lib/

  make -s -j16
  make -s install
}

post () {
  rm -rf $DEP_ROOT/share/{locale,doc,gtk-doc,man,info}
}

install_pkgs
install_orc
install_fftw3
install_giflib
install_libexif
install_libwebp
install_libjpeg
install_magick
install_vips
post
