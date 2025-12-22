#!/bin/bash
# 以c-ares为例
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
# 构建c-ares知识图谱, 默认语言cpp
python repo.py --project_name c-ares \
    --shared_llm_dir $ROSFuzzerPath/docker_shared \
    --saved_dir $ROSFuzzerPath/fuzzing_llm_engine/external_database/c-ares/codebase \
    --src_api \
    --call_graph
