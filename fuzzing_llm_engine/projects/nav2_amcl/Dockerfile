# 基于预编译的 ROS 2 镜像
FROM ros:humble-desktop

# ========= 基本环境 =========
ENV DEBIAN_FRONTEND=noninteractive

# ========= 换源加速（可选） =========
RUN sed -i 's@http://archive.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g' /etc/apt/sources.list && \
    sed -i 's@http://security.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g' /etc/apt/sources.list

# ========= 安装编译工具和依赖 =========
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool \
    && rm -rf /var/lib/apt/lists/*

# ========= 初始化 rosdep =========
RUN rosdep init || true
RUN rosdep update

# ========= 创建工作空间 =========
WORKDIR /ws/src

# ========= 拉取 Nav2 源码 =========
RUN git clone https://github.com/ros-planning/navigation2.git -b humble

# ========= 安装 Nav2 依赖 =========
WORKDIR /ws
RUN apt-get update && \
    rosdep install \
    --from-paths src \
    --ignore-src \
    --rosdistro humble \
    -y \
    && rm -rf /var/lib/apt/lists/*

# ========= 设置环境 =========
RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc
RUN echo "source /ws/install/setup.bash" >> /root/.bashrc

# ========= 默认进入 bash =========
CMD ["/bin/bash"]