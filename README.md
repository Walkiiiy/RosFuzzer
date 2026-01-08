this project will be witten based on CKGFUZZER

- 需要ubuntu下的conda和docker 
- bash env_setup.sh
- conda activate ros-fuzzer
- wget https://github.com/github/codeql-action/releases/download/codeql-bundle-20211208/codeql-bundle-linux64.tar.gz
- 解压后mv codeql到docker_shared
- pip  install docker
- ROSFuzzerPath=$(pwd)
- cd $ROSFuzzerPath/fuzzing_llm_engine/codetext/parser/tree-sitter
- git clone https://github.com/tree-sitter/tree-sitter-cpp.git tree-sitter-cpp
- cd $ROSFuzzerPath/fuzzing_llm_engine/repo/
- 构建c-ares知识图谱 python repo.py --project_name c-ares --shared_llm_dir $ROSFuzzerPath/docker_shared --saved_dir $ROSFuzze
rPath/fuzzing_llm_engine/external_database/c-ares/codebase --src_api --call_graph

- 构建call_graph 以rosidl_runtime_c为例:
# 预处理
```
(ros-fuzzer) walkiiiy@DESKTOP-UI66N0K:~/RosFuzzer/fuzzing_llm_engine/repo$ python3 repo.py --project_name rosidl_runtime_c --language cpp --shared_llm_dir /home/walkiiiy/RosFuzzer/docker_shared --saved_dir /home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c/codebase --target_src_path /ws/src/ros2/rosidl/rosidl_runtime_c --src_api --call_graph
```
- 提取api对应的代码调用图
```
python preproc.py --project_name rosidl_runtime_c --src_api_file_path /home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c
```

# 正式操作
- 生成driver
```
python driver_gen.py --yaml /fuzzing_llm_engine/external_database/rosidl_runtime_c/config.yaml --gen_driver --summary_api --check_compilation --gen_input
```
- 执行fuzz
```
python fuzzing_llm_engine/fuzzing.py   --container-path /ws/src/ros2/rosidl/rosidl_runtime_c   --fuzz-driver /home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c/fuzz_driver/ros2_rosidl_runtime_c_fuzz_driver_False_deepseek-coder_1.c --cmakelists /home/walkiiiy/RosFuzzer/fuzzing_llm_engine/projects/rosidl_runtime_c/CMakeLists.txt   --workdir /ws   --output docker_shared/rosidl_fuzz_output.txt
```

## FUZZ GEN流程设计

下一步：
- 生成的fuzzer编译通过率问题
- 合并成一个fuzzer
- 生成对应的input
- 头文件自动获取


## Fuzz 流程设计

build.sh 负责初始构建没有fuzz的项目，供codeql生成调用图

将projects里面的cmakelists和fuzzing.sh复制到docker里面，将external_database里上一步llm生成的driver也复制到里面
fuzzing.sh负责用新的cmakelist链接libfuzzer模块重新编译指定模块并执行fuzzer输出覆盖率报告

下一步：
- 自动生成能连接libfuzzer的cmakelists,（fuzzing.sh能在各个模块间通用但cmakelists不行）
- 使用生成的input

# CKGFuzzer legacy


CKGFuzzer is an automated fuzzing framework that leverages large language models (LLMs) and a code knowledge graph to generate fuzz drivers. By automating the fuzz driver creation process and analyzing API usage, CKGFuzzer improves fuzzing efficiency, code coverage, and bug detection in complex software systems.

---

## Workflow

![CKGFuzzer Workflow](workflow.png)




---

### 1. Preparation

Before running CKGFuzzer, ensure the following steps are completed:

1. **Ensure CodeQL is Installed**  
   Make sure that CodeQL is properly installed under the `docker_shared/` directory.

2. **API List**  
   Ensure that the API list (`api_list.json`) for the library to be tested is stored in the following path:
   ```
   /fuzzing_llm_engine/external_database/{project}/api_list.json
   ```

3. **Fuzzing Configuration**  
   Ensure that the fuzzing configuration (including your `API key` for LLM service) file (`config.yaml`) for the library is stored in the following path:  
   ```
   /fuzzing_llm_engine/external_database/{project}/config.yaml
   ```

4. **Library Environment**  
Ensure that the environment files (dockerfile etc.) for the library are stored in the following path:
    ```
    /fuzzing_llm_engine/projects/{project}/
    ```

5. **Library Usage**  
Prepare proper usage cases for the library to fix generated fuzz drivers and store them in the following folder:
    ```
    /fuzzing_llm_engine/external_database/{project}/test/
    ```

### 2. Extract Information from the Target Library

To extract the necessary information from the target library, such as source APIs and the call graph.

Navigate to the `fuzzing_llm_engine/repo` directory and run the `repo.py` script with the appropriate parameter:
```bash
cd fuzzing_llm_engine/repo
python repo.py --project_name {project} --shared_llm_dir /docker_shared --saved_dir /fuzzing_llm_engine/external_database/{project}/codebase --src_api --call_graph
```
Replace `{project}` with the name of the project you are testing.

### 3. Build External Knowledge Base 

After extracting the necessary information, build the external knowledge base for the target library by running the following command:
```bash
python preproc.py --project_name {project} --src_api_file_path /fuzzing_llm_engine/external_database/{project}
```
Replace `{project}` with the name of the project you are testing.



### 4. Run the Fuzzing Process

To run the fuzzing process, use the `fuzzing.py` script. You can skip certain steps by adding the `skip` prefix to the relevant parameters.

Run the fuzzing script with the following command:
```bash
cd ..
python fuzzing.py --yaml /fuzzing_llm_engine/external_database/{project}/config.yaml --gen_driver --summary_api --check_compilation --gen_input
```
**Skipping Steps**
If you want to skip certain steps, use the `--skip_*` options. For example:
```bash
python fuzzing.py --yaml /fuzzing_llm_engine/external_database/{project}/config.yaml --skip_gen_driver --skip_summary_api --skip_check_compilation --skip_gen_input
```
This command skips the fuzz driver generation and proceeds with the remaining steps.