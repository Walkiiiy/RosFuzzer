# !/bin/bash
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# 安装 clang/lld/llvm 和 ament 依赖（使用 -y 以非交互方式）
apt update
apt-get install -y --no-install-recommends \
    clang lld \
    llvm llvm-dev llvm-runtime \
    ros-humble-ament-cmake-ros \
    ros-humble-rcutils \
    ros-humble-rosidl-typesupport-interface \
    ros-humble-ament-lint-auto \
    llvm

# ROS 环境（确保 ament_cmake_ros 可用），在 set -u 下暂时关闭未定义变量错误
if [ -f /opt/ros/humble/setup.bash ]; then
    set +u
    # shellcheck disable=SC1091
    . /opt/ros/humble/setup.bash
    set -u
fi
# 明确补充路径，确保 CMake/ament 前缀可见
export CMAKE_PREFIX_PATH="/opt/ros/humble:${CMAKE_PREFIX_PATH:-}"
export AMENT_PREFIX_PATH="/opt/ros/humble:${AMENT_PREFIX_PATH:-}"

# 避免误把 /ws 根目录当作包构建（清理潜在残留）
rm -f /ws/CMakeLists.txt /ws/package.xml

# 确保 CMake 能找到 clang
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

# 覆盖率设置（仅在工具可用时生效）
export CFLAGS="${CFLAGS:-} -fprofile-instr-generate -fcoverage-mapping"
export CXXFLAGS="${CXXFLAGS:-} -fprofile-instr-generate -fcoverage-mapping"
export LLVM_PROFILE_FILE="${LLVM_PROFILE_FILE:-/ws/fuzz_corpus/fuzz-%p.profraw}"
PROFILE_LIB=$(clang -print-file-name=libclang_rt.profile-x86_64.a)
if [ -n "${PROFILE_LIB}" ] && [ "${PROFILE_LIB}" != "libclang_rt.profile-x86_64.a" ]; then
    export LDFLAGS="${LDFLAGS:-} -fprofile-instr-generate -fcoverage-mapping ${PROFILE_LIB}"
else
    export LDFLAGS="${LDFLAGS:-} -fprofile-instr-generate -fcoverage-mapping -lclang_rt.profile-x86_64"
fi

FUZZ_TIMEOUT=${FUZZ_TIMEOUT:-120}
FUZZ_TARGET_NAME=${FUZZ_TARGET_NAME:-fuzzer}
COVERAGE_OUT=${COVERAGE_OUT:-/ws/fuzz_corpus/coverage.json}
COVERAGE_TXT=${COVERAGE_TXT:-/ws/fuzz_corpus/coverage.txt}
FUZZ_ARGS=${FUZZ_ARGS:-"-max_total_time=${FUZZ_TIMEOUT} -print_final_stats=1 -ignore_crashes=1 -handle_segv=1 -handle_abort=1 -handle_bus=1 -handle_sigill=1"}
DEFAULT_COVERAGE_JSON="${COVERAGE_OUT}"
DEFAULT_COVERAGE_TXT="${COVERAGE_TXT:-/ws/fuzz_corpus/coverage.txt}"

# 避免生成 core 并容忍 fuzz 进程崩溃
ulimit -c 0

# fuzz rosidl_runtime_c，cmakelist.txt 和 fuzz/ 将在外部替换
rm -rf log build/ install/
mkdir -p fuzz_corpus
colcon build --packages-up-to rosidl_runtime_c \
    --cmake-args -DROSIDL_RUNTIME_C_ENABLE_FUZZ=ON \
                 -DBUILD_TESTING=OFF \
                 -DCMAKE_C_COMPILER=${CC} \
                 -DCMAKE_CXX_COMPILER=${CXX} \
                 -DCMAKE_BUILD_TYPE=Debug \
                 -DCMAKE_C_FLAGS="${CFLAGS}" \
                 -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
                 -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
                 -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}"
# 运行 fuzz，使用 libFuzzer 的 max_total_time
set +e
LLVM_PROFILE_FILE="/ws/fuzz_corpus/coverage.profraw" \
ASAN_OPTIONS="${ASAN_OPTIONS:-abort_on_error=0:disable_coredump=1}" \
UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=0}" \
./install/rosidl_runtime_c/lib/rosidl_runtime_c/${FUZZ_TARGET_NAME} fuzz_corpus ${FUZZ_ARGS} || true
set -e

# 导出覆盖率报告（如果 llvm 工具存在且生成了 profraw）
if command -v llvm-profdata >/dev/null 2>&1 && command -v llvm-cov >/dev/null 2>&1; then
    if ls /ws/fuzz_corpus/*.profraw >/dev/null 2>&1; then
        llvm-profdata merge -sparse /ws/fuzz_corpus/*.profraw -o /ws/fuzz_corpus/coverage.profdata || true
        OBJECT_ARGS=""
        for obj in $(find /ws/install/rosidl_runtime_c/lib -type f -name "*.so"); do
            OBJECT_ARGS="${OBJECT_ARGS} -object ${obj}"
        done
        llvm-cov export ./install/rosidl_runtime_c/lib/rosidl_runtime_c/${FUZZ_TARGET_NAME} \
            -instr-profile=/ws/fuzz_corpus/coverage.profdata \
            ${OBJECT_ARGS} \
            >/tmp/coverage.json || true
        llvm-cov report ./install/rosidl_runtime_c/lib/rosidl_runtime_c/${FUZZ_TARGET_NAME} \
            -instr-profile=/ws/fuzz_corpus/coverage.profdata \
            ${OBJECT_ARGS} \
            >/tmp/coverage.txt || true
        mv /tmp/coverage.json "${COVERAGE_OUT}" 2>/dev/null || true
        mv /tmp/coverage.txt "${DEFAULT_COVERAGE_TXT}" 2>/dev/null || true
    else
        # 创建空文件以避免调用端找不到
        : > "${DEFAULT_COVERAGE_JSON}"
        : > "${DEFAULT_COVERAGE_TXT}"
    fi
fi

# 始终生成占位文件，避免调用端 docker cp 报错
mkdir -p "$(dirname "${DEFAULT_COVERAGE_JSON}")" "$(dirname "${DEFAULT_COVERAGE_TXT}")"
[ -f "${DEFAULT_COVERAGE_JSON}" ] || : > "${DEFAULT_COVERAGE_JSON}"
[ -f "${DEFAULT_COVERAGE_TXT}" ] || : > "${DEFAULT_COVERAGE_TXT}"
echo "fuzz_target_name:${FUZZ_TARGET_NAME}"
