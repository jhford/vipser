#!/bin/bash
set -xe

export PREFIX="${PREFIX:-$PWD/deps}"
export BUILDROOT="${BUILDROOT:-$PWD/buildroot}"
export J="${J:-4}"

if [ -z "$PKG_CONFIG_PATH" ] ; then
  export PKG_CONFIG_PATH="$PREFIX/lib64/pkgconfig:$PKG_CONFIG_PATH/lib/pkgconfig:/usr/local/lib/pkgconfig"
fi

mkdir -p "$BUILDROOT"
cd "$BUILDROOT"

MAGICK_VER=7.0.9-20
WEBP_VER=1.1.0
VIPS_VER=8.9.1

download() {
  outname="$1"
  shift
  if [ ! -f "$outname" ]; then
    url="$1"
    curl --fail -o "$outname" -L "$url"
  fi
}

install_libwebp() {
  cd "$BUILDROOT"
  download libwebp-${WEBP_VER}.tar.gz "https://chromium.googlesource.com/webm/libwebp/+archive/refs/heads/${WEBP_VER}.tar.gz"
  mkdir -p libwebp-${WEBP_VER}
  cd libwebp-${WEBP_VER}
  tar zxf ../libwebp-${WEBP_VER}.tar.gz
  ./autogen.sh
  ./configure -q \
    --enable-static \
    --enable-shared \
    --prefix="$PREFIX"

  make -s "-j$J"
  make -s install
}

install_magick() {
  cd "$BUILDROOT"
  download ImageMagick-${MAGICK_VER}.tar.gz "https://github.com/ImageMagick/ImageMagick/archive/${MAGICK_VER}.tar.gz"
  tar zxf ImageMagick-${MAGICK_VER}.tar.gz
  cd ImageMagick-${MAGICK_VER}
  ./configure -q \
    --prefix="${PREFIX}"\
    --enable-static \
    --enable-shared \
    --with-{magick-plus-plus,utilities,modules,fontconfig,freetype,pango,xml}=no \
    --with-modules=no \
    --with-quantum-depth=8 \
    --enable-hdri=no \
    --disable-installed \
    --without-threads \
    --with-{autotrace,djvu,dps,flif,fpx,gslib,gvc,heic,jbig,jpeg,jxl}=no \
    --with-{lqr,openexr,openjp2,png,raqm,rsvg,raw,tiff,webp,wmf,x}=no

  make -s "-j$J"
  make -s install
}

install_vips() {
  cd "$BUILDROOT"
  download "vips-${VIPS_VER}.tar.gz" "https://github.com/libvips/libvips/releases/download/v${VIPS_VER}/vips-${VIPS_VER}.tar.gz"
  tar zxf "vips-${VIPS_VER}.tar.gz"

  cd "vips-${VIPS_VER}"
  PKG_CONFIG_PATH="$PKG_CONFIG_PATH" ./configure -q \
    --prefix="$PREFIX" \
    --enable-shared \
    --disable-static \
    --without-{x,heif,OpenEXR,pdfium,poppler,rsvg,matio,ppm,pangoft2,python} \
    --with-magick \
    --with-{libwebp,tiff,giflib,png,jpeg,libexif}

  make -s "-j$J"
  make -s install
}

# These are functions so that you can disable specific build dependencies
# easily
install_libwebp
install_magick
install_vips
