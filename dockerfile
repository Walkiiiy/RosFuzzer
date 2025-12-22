    # 构建能从源码构建ros2humble的镜像
    # 保存后运行docker build -t ros2-humble-src .
    FROM ubuntu:22.04

    # ========= 基本环境 =========
    ENV DEBIAN_FRONTEND=noninteractive
    ENV LANG=en_US.UTF-8
    ENV LC_ALL=en_US.UTF-8

    # ========= 换源（HTTP bootstrap，规避 TLS/CA 问题） =========
    RUN sed -i 's@http://archive.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g' /etc/apt/sources.list && \
        sed -i 's@http://security.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g' /etc/apt/sources.list && \
        sed -i '/jammy-updates/d' /etc/apt/sources.list

    # ========= 基础系统依赖 =========
    RUN apt-get update && apt-get install -y \
        ca-certificates \
        locales \
        curl \
        wget \
        git \
        build-essential \
        cmake \
        ninja-build \
        python3 \
        python3-pip \
        python3-setuptools \
        python3-wheel \
        python3-argcomplete \
        lsb-release \
        gnupg2 \
        && locale-gen en_US.UTF-8 \
        && rm -rf /var/lib/apt/lists/*

    # ========= 切回 HTTPS =========
    RUN sed -i 's@http://mirrors.tuna.tsinghua.edu.cn@https://mirrors.tuna.tsinghua.edu.cn@g' /etc/apt/sources.list

    # ========= 添加 OSRF / Gazebo 仓库（无 software-properties-common） =========
    RUN mkdir -p /usr/share/keyrings && \
        curl -fsSL https://packages.osrfoundation.org/gazebo.key | \
        gpg --dearmor -o /usr/share/keyrings/gazebo-archive-keyring.gpg && \
        echo "deb [arch=amd64 signed-by=/usr/share/keyrings/gazebo-archive-keyring.gpg] \
    https://packages.osrfoundation.org/gazebo/ubuntu-stable jammy main" \
        > /etc/apt/sources.list.d/gazebo-stable.list

    # ========= ROS Python 工具链（统一 pip） =========
    RUN python3 -m pip install --upgrade pip && \
        pip3 install \
        empy==3.3.4 \
        colcon-common-extensions \
        rosdep \
        vcstool \
        catkin_pkg \
        rosdistro

    # ========= 初始化 rosdep =========
    RUN rosdep init || true
    RUN rosdep update

    # ========= ROS2 Workspace =========
    WORKDIR /ros2_ws

    # ========= 拉取 ROS 2 Humble 源码 =========
    RUN mkdir -p src && \
        curl -fsSL https://raw.githubusercontent.com/ros2/ros2/humble/ros2.repos -o ros2.repos && \
        vcs import src < ros2.repos && \
        rm ros2.repos

    # ========= rosdep 安装系统依赖（最终稳定 skip 集） =========
    RUN apt-get update && \
        rosdep install \
        --from-paths src \
        --ignore-src \
        --rosdistro humble \
        -y \
        --skip-keys "fastcdr rti-connext-dds-6.0.1 urdfdom_headers \
                    python3-vcstool \
                    python3-colcon-common-extensions \
                    python3-rosdep \
                    python3-catkin-pkg-modules \
                    python3-rosdistro-modules" && \
        rm -rf /var/lib/apt/lists/*

    # ========= 默认进入 bash =========
    CMD ["/bin/bash"]


    # 构建后进入ros目录运行：
    # colcon build   --merge-install   --parallel-workers 2   --cmake-args     -DBUILD_TESTING=OFF
