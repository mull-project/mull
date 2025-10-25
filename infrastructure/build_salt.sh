#!/usr/bin/env bash

set -e

case $OSTYPE in
  linux*)
    echo $(cat /etc/os-release) $(ldd --version) | sha256sum | awk ' { print $1 } '
    ;;

  darwin*)
    echo $(sw_vers) | sha256sum | awk ' { print $1 } '
    ;;

  *)
    echo "Unkown OS type: $OSTYPE"
    exit 1
    ;;
esac
