#!/bin/bash
# The purpose of this script is to contain the logic needed to correctly
# build the docker image.  It is not mandatory, but building the image
# another way might be suboptimal

set -e

DOCKER=${DOCKER:-docker}
TAR=${TAR:-tar}
GIT=${GIT:-git}
IMAGE=${IMAGE:-vipser}
TAG=${TAG:-latest}

build () {
  tar c $("${GIT}" ls-files) | docker build -t "$IMAGE:$TAG" -
}

build
