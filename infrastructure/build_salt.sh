#!/usr/bin/env bash

set -e

arch=$(uname -m)

case $OSTYPE in
  linux*)
    os_id=$(grep "^ID=" /etc/os-release | cut -d= -f2 | tr -d '"')
    os_version=$(grep "^VERSION_ID=" /etc/os-release | cut -d= -f2 | tr -d '"')
    glibc=$(ldd --version | head -1 | grep -oE '[0-9]+\.[0-9]+$')
    echo "${os_id}-${os_version}-${arch}-glibc${glibc}"
    ;;

  darwin*)
    os_version=$(sw_vers --productVersion)
    echo "macos-${os_version}-${arch}"
    ;;

  *)
    echo "Unknown OS type: $OSTYPE"
    exit 1
    ;;
esac
