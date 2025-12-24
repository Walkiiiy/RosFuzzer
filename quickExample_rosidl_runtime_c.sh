#!/bin/bash

bash env_setup.sh
conda activate ros-fuzzer
wget https://github.com/github/codeql-action/releases/download/codeql-bundle-20211208/codeql-bundle-linux64.tar.gz
tar -xvzf codeql-bundle-linux64.tar.gz
mv codeql docker_shared
pip  install docker
ROSFuzzerPath=$(pwd)
cd $ROSFuzzerPath/fuzzing_llm_engine/codetext/parser/tree-sitter
git clone https://github.com/tree-sitter/tree-sitter-cpp.git tree-sitter-cpp
cd $ROSFuzzerPath/fuzzing_llm_engine/repo/

# 构建知识图谱, 默认语言cpp

python3 repo.py --project_name rosidl_runtime_c --language cpp --shared_llm_dir /home/walkiiiy/RosFuzzer/docker_shared --saved_dir /home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c/codebase --target_src_path /ws/src/ros2/rosidl/rosidl_runtime_c --src_api --call_graph
