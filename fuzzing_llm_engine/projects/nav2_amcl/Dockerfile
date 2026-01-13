# 使用预编译的 ROS 2 镜像
FROM ros:humble-ros-base

# ========= 基本环境 =========
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# ========= 系统内存和核心转储配置 =========
RUN echo "kernel.core_pattern=/tmp/core.%e.%p.%t" >> /etc/sysctl.conf && \
    echo "kernel.core_uses_pid=1" >> /etc/sysctl.conf && \
    echo "fs.suid_dumpable=2" >> /etc/sysctl.conf && \
    echo "vm.overcommit_memory=1" >> /etc/sysctl.conf && \
    echo "vm.mmap_min_addr=0" >> /etc/sysctl.conf

# 增加用户进程限制
RUN echo "* soft core unlimited" >> /etc/security/limits.conf && \
    echo "* hard core unlimited" >> /etc/security/limits.conf && \
    echo "* soft memlock unlimited" >> /etc/security/limits.conf && \
    echo "* hard memlock unlimited" >> /etc/security/limits.conf && \
    echo "* soft stack unlimited" >> /etc/security/limits.conf && \
    echo "* hard stack unlimited" >> /etc/security/limits.conf && \
    echo "* soft nofile 65536" >> /etc/security/limits.conf && \
    echo "* hard nofile 65536" >> /etc/security/limits.conf

# ========= 替换 Ubuntu/ROS2 软件源为镜像 =========
ARG UBUNTU_MIRROR=https://mirrors.tuna.tsinghua.edu.cn/ubuntu
ARG ROS2_MIRROR=https://mirrors.tuna.tsinghua.edu.cn/ros2
RUN set -eux; \
    if [ -f /etc/apt/sources.list ]; then \
      sed -i "s@http://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list; \
      sed -i "s@http://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list; \
      sed -i "s@https://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list; \
      sed -i "s@https://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list; \
      sed -i "s@http://packages.ros.org/ros2@${ROS2_MIRROR}@g" /etc/apt/sources.list; \
      sed -i "s@https://packages.ros.org/ros2@${ROS2_MIRROR}@g" /etc/apt/sources.list; \
    fi; \
    if [ -f /etc/apt/sources.list.d/ubuntu.sources ]; then \
      sed -i "s@http://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
      sed -i "s@http://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
      sed -i "s@https://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
      sed -i "s@https://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
      sed -i "s@http://packages.ros.org/ros2@${ROS2_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
      sed -i "s@https://packages.ros.org/ros2@${ROS2_MIRROR}@g" /etc/apt/sources.list.d/ubuntu.sources; \
    fi; \
    for f in /etc/apt/sources.list.d/*.sources /usr/share/ros-apt-source/*.sources; do \
      [ -f "$f" ] || continue; \
      sed -i "s@http://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@http://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@https://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@https://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@http://packages.ros.org/ros2@${ROS2_MIRROR}@g" "$f"; \
      sed -i "s@https://packages.ros.org/ros2@${ROS2_MIRROR}@g" "$f"; \
      if grep -q "ros2/ubuntu" "$f"; then \
        sed -i "s/^Types:.*$/Types: deb/" "$f"; \
      fi; \
    done; \
    for f in /etc/apt/sources.list.d/*.list; do \
      [ -f "$f" ] || continue; \
      sed -i "s@http://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@http://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@https://archive.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@https://security.ubuntu.com/ubuntu@${UBUNTU_MIRROR}@g" "$f"; \
      sed -i "s@http://packages.ros.org/ros2@${ROS2_MIRROR}@g" "$f"; \
      sed -i "s@https://packages.ros.org/ros2@${ROS2_MIRROR}@g" "$f"; \
    done

# ========= 安装基础工具 =========
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# ========= 可选：添加 Gazebo 仓库 =========
ARG ENABLE_GAZEBO_REPO=false
RUN if [ "${ENABLE_GAZEBO_REPO}" = "true" ]; then \
      mkdir -p /usr/share/keyrings && \
      curl -fsSL --retry 5 --retry-delay 2 https://packages.osrfoundation.org/gazebo.key | \
      gpg --dearmor -o /usr/share/keyrings/gazebo-archive-keyring.gpg && \
      echo "deb [arch=amd64 signed-by=/usr/share/keyrings/gazebo-archive-keyring.gpg] \
https://packages.osrfoundation.org/gazebo/ubuntu-stable jammy main" \
      > /etc/apt/sources.list.d/gazebo-stable.list; \
    fi

# ========= 安装编译工具 =========
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    python3-argcomplete \
    # Python ROS 工具链
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    python3-catkin-pkg \
    && rm -rf /var/lib/apt/lists/*

# ========= 初始化 rosdep =========
RUN rosdep init || true && rosdep update

# ========= 安装额外的 Python 包 =========
RUN pip3 install --no-cache-dir empy==3.3.4

# ========= 安装 Nav2 编译依赖 =========
RUN apt-get update && apt-get install -y --no-install-recommends \
    libasio-dev \
    libtinyxml2-dev \
    libcunit1-dev \
    # 预安装已知有问题的 ROS 包
    ros-humble-bondcpp \
    ros-humble-diagnostic-updater \
    ros-humble-angles \
    && rm -rf /var/lib/apt/lists/*

# ========= 创建工作空间 =========
WORKDIR /ws

# ========= 拉取 Nav2 源码 =========
RUN mkdir -p src && \
    cd src && \
    git clone https://github.com/ros-planning/navigation2.git -b humble --depth 1

# ========= 安装 Nav2 的 rosdep 依赖 =========
RUN apt-get update && \
    rosdep install \
    --from-paths src \
    --ignore-src \
    --rosdistro humble \
    -y \
    --skip-keys "fastcdr rti-connext-dds-6.0.1" \
    && rm -rf /var/lib/apt/lists/*

# ========= 安装 fuzzing 相关工具 =========
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang lld \
    llvm llvm-dev llvm-runtime \
    && rm -rf /var/lib/apt/lists/*

# ========= 设置环境变量 =========
RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc && \
    echo "cd /ws" >> /root/.bashrc

COPY build.sh /ws/

# ========= 默认进入 bash =========
CMD ["/bin/bash"]
