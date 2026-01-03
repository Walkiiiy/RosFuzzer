# !/bin/bash
# 对rosidl_runtime_c进行fuzz
    # clang编译器支持
apt update
apt-get install clang lld

# fuzz rosidl_runtime_c, cmakelist.txt和fuzz/需要替换  可以在fuzzing里面实现




rm -rf log build/ install/
mkdir -p fuzz_corpus
colcon build --packages-up-to rosidl_runtime_c   --cmake-args -DROSIDL_RUNTIME_C_ENABLE_FUZZ=ON                -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++                -DCMAKE_BUILD_TYPE=RelWithDebInfo
./install/rosidl_runtime_c/lib/rosidl_runtime_c/fuzz_string fuzz_corpus