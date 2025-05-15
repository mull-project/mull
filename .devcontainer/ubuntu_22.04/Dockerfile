FROM ubuntu:22.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client sudo tree file


RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-13-dev \
  libclang-13-dev \
  libc++-13-dev \
  libc++abi-13-dev \
  clang-13 \
  lld-13

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-14-dev \
  libclang-14-dev \
  libc++-14-dev \
  libc++abi-14-dev \
  clang-14 \
  lld-14

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-15-dev \
  libclang-15-dev \
  libc++-15-dev \
  libc++abi-15-dev \
  clang-15 \
  lld-15


ARG USERNAME=mull
ARG USER_UID=1042
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
  && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
  && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
  && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
