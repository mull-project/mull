#!/usr/bin/bash

set -e
set -x

if [[ -f /etc/redhat-release ]]; then
bazelisk_version=v1.27.0

arch=$(uname -m)
case "$arch" in
    x86_64)
        bazelisk_arch=amd64
        ;;
    aarch64)
        bazelisk_arch=arm64
        ;;
    *)
        echo "Unsupported architecture: $arch"
        exit 1
        ;;
esac
echo $PATH
sudo curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/$bazelisk_version/bazelisk-linux-$bazelisk_arch -o /usr/local/bin/bazel
sudo chmod +x /usr/local/bin/bazel
fi
