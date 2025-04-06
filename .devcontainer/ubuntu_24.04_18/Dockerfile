FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client sudo
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-18-dev \
  libclang-18-dev \
  libc++-18-dev \
  libc++abi-18-dev \
  clang-18

ARG USERNAME=mull
ARG USER_UID=1042
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
  && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
  && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
  && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
