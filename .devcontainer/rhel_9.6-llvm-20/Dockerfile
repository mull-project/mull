FROM redhat/ubi9:9.6

RUN yum install -y --nogpgcheck --allowerasing \
  vim wget curl sudo file libstdc++-static \
  git sqlite rpmdevtools ca-certificates


RUN yum install -y --nogpgcheck llvm-devel clang-devel clang lld


ARG USERNAME=mull
ARG USER_UID=1042
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
  && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
  && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
  && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
