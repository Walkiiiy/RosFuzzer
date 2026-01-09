import json
import re
from typing import Dict, List
from argparse import Namespace
from llama_index.core.prompts import PromptTemplate
from llama_index.core.memory import (
    VectorMemory,
    SimpleComposableMemory,
    ChatMemoryBuffer,
)
from llama_index.core.llms import ChatMessage
from regex import P
from tqdm import tqdm
import os
from loguru import logger
import yaml

from fuzzing import check_compile_fuzz_driver

class FuzzingGenerationAgent:
    file_suffix = {"c": "c", "c++": "cc"}
    
    def __init__(self, llm_coder, llm_embedding, llm_analyzer, query_tools: Dict, database_dir: str, headers: list, language: str, use_memory=False):
        self.llm_coder = llm_coder
        self.llm_analyzer = llm_analyzer
        self.llm_embedding = llm_embedding
        self.database_dir = database_dir
        self.headers = headers
        self.language = language
        self.query_tools = query_tools                
        self.chat_memory_buffer = ChatMemoryBuffer.from_defaults(llm=llm_analyzer)
        self.vector_memory = VectorMemory.from_defaults(
            vector_store=None,
            embed_model=llm_embedding,
            retriever_kwargs={"similarity_top_k": 1},
        )
        self.composable_memory = SimpleComposableMemory.from_defaults(
            primary_memory=self.chat_memory_buffer,
            secondary_memory_sources=[self.vector_memory],
        )
        self.use_memory = use_memory
        self.project_config = self._load_project_config()
        
        self.fuzz_driver_generation_prompt = PromptTemplate(
            "You are a fuzz driver expert, capable of writing a high-quality, compilable fuzz driver to test a library with extensive code coverage and robust error handling."
            "Please generate an executable {lang} fuzz driver according to the following instructions:\n"
            "1. Create a function named `LLVMFuzzerTestOneInput` that achieves a task using the provided API combination. Each API should be called at least once. The function signature must be `int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)`.\n"
            "2. Ensure the generated code correctly utilizes the fuzz driver inputs, `const uint8_t *data` and `size_t size`.\n"
            "3. API inputs must derive from the fuzz driver inputs, `const uint8_t *data` and `size_t size`.\n"
            "4. Include all the provided headers at the beginning of the file.\n"
            "5. Do NOT reimplement, mock, or redefine the target APIs, their structs, or allocators. Only call the real APIs via their headers; if a required header is missing, include it explicitly instead of writing your own stubs.\n"
            "6. The code should be complete and executable without requiring manual completion by the developer.\n"
            "7. Implement robust error handling for all API calls. Check return values and handle potential errors appropriately.\n"
            "8. Avoid using incomplete types. If a type's size is unknown, use opaque pointers and the library's provided functions for allocation and deallocation.\n"
            "9. Prevent buffer overflows by carefully managing buffer sizes and using safe string functions.\n"
            "10. Ensure proper memory management: allocate memory as needed and free all allocated resources before the function returns.\n"
            "11. Implement proper initialization of variables and structures to avoid undefined behavior.\n"
            "12. Add appropriate bounds checking before accessing arrays or performing pointer arithmetic.\n"
            "13. If the API requires supporting types (e.g., rcutils allocator), include the official headers (e.g., <rcutils/allocator.h>) and use the library-provided helpers instead of custom replacements.\n"
            "I will provide the API combination, headers, API source code, and API summary below.\n"
            "API Combination:\n"
            "{api_list}\n\n"
            "Below are the system headers, API source code, and API summary required for the fuzz driver. Use all provided content to ensure the correctness of the generated fuzz driver:\n"
            "Provided headers (Include all header files to ensure the executability of the fuzz driver):\n"
            "{headers}\n\n"
            "API Source Code:\n"
            "```{lang}"
            "{api_info}\n"
            "```\n\n"
            "API Summary:\n"
            "{api_sum}\n\n"
            "If file operations are required, you firstly need to convert the fuzz input into a string and create the corresponding object (e.g., TIFFStreamOpen()) directly in memory with the string rather than reading input files from disk. If an output file is needed, name it uniformly as 'output_file.'\n"
            "Add any non-code content as comments. Generate an executable {lang} fuzz driver according to the above descriptions, focusing on safety, proper resource management, and error handling."
        )

        self.fix_compilation_prompt = PromptTemplate(
            "You are an expert C/C++ programmer specializing in fixing compilation errors.\n"
            "Fix the following fuzz driver file:\n"
            "```\n"
            "{fuzz_driver}\n"
            "```\n"
            "Compilation errors:\n"
            "{error}\n"
            "Instructions:\n"
            "1. Make minimal changes to fix the errors.\n"
            "2. Do NOT modify or remove any #include statements.\n"
            "3. Return the complete fixed code wrapped in triple backticks.\n"
        )

    def _load_project_config(self):
        config_path = os.path.join(self.database_dir, "config.yaml")
        if not os.path.isfile(config_path):
            logger.warning(f"Missing config.yaml at {config_path}, fallback to defaults.")
            return {}
        with open(config_path, "r", encoding="utf-8") as f:
            config = yaml.safe_load(f) or {}
        return config.get("config", {})

    def _build_compile_args(self, project, fuzz_driver_path):
        project_base = project.split("/")[-1]
        work_dir = self.project_config.get("work_dir", os.getcwd())
        shared_dir = self.project_config.get("shared_dir", os.path.join(work_dir, "docker_shared"))
        if "/" in project:
            container_path = f"/ws/src/{project}"
            workdir = "/ws"
        else:
            container_path = f"/src/{project}"
            workdir = "/src"
        cmakelists_path = os.path.join(
            work_dir, "fuzzing_llm_engine", "projects", project_base, "CMakeLists.txt"
        )
        if not os.path.isfile(cmakelists_path):
            logger.warning(f"Missing CMakeLists.txt at {cmakelists_path}, skip compilation check.")
            return None
        fuzz_script_path = os.path.join(
            work_dir, "fuzzing_llm_engine", "projects", project_base, "fuzzing.sh"
        )
        if not os.path.isfile(fuzz_script_path):
            logger.warning(f"Missing fuzzing.sh at {fuzz_script_path}, skip compilation check.")
            return None
        fuzz_script = fuzz_script_path
        output_path = os.path.join(shared_dir, f"{project_base}_fuzz_output.txt")
        image = f"{project_base}_base_image:latest"
        return Namespace(
            image=image,
            container_path=container_path,
            fuzz_driver=fuzz_driver_path,
            cmakelists=cmakelists_path,
            fuzz_script=fuzz_script,
            driver_dest_name="fuzzer.c",
            target_binary="fuzzer",
            timeout_seconds=10,
            coverage_output=None,
            coverage_container_path="/ws/fuzz_corpus/coverage.json",
            coverage_text_output=None,
            coverage_text_container_path="/ws/fuzz_corpus/coverage.txt",
            output=output_path,
            container_name=None,
            workdir=workdir,
        )

    def _check_compile(self, args):
        try:
            return check_compile_fuzz_driver(args)
        except SystemExit:
            return 0

    def _read_compile_output(self, output_path):
        if not output_path or not os.path.isfile(output_path):
            return ""
        with open(output_path, "r", encoding="utf-8", errors="ignore") as f:
            return f.read()

    def fix_compilation(self, fuzz_driver_path, error_message):
        if not error_message:
            logger.warning("Empty compilation error output, skip fix.")
            return False
        if not os.path.isfile(fuzz_driver_path):
            logger.warning(f"Missing fuzz driver file: {fuzz_driver_path}")
            return False
        with open(fuzz_driver_path, "r", encoding="utf-8", errors="ignore") as f:
            code = f.read()
        question = self.fix_compilation_prompt.format(fuzz_driver=code, error=error_message)
        fix_code_raw = self.llm_coder.complete(question).text
        fix_code = self.extract_code(str(fix_code_raw))
        if fix_code == "No code found":
            logger.warning("Failed to extract fixed code from LLM response.")
            return False
        with open(fuzz_driver_path, "w", encoding="utf-8") as f:
            f.write(fix_code)
        return True
  
        self.fuzz_driver_generation_prompt_with_memory = PromptTemplate(
            "You are a fuzz driver expert, capable of writing a high-quality, compilable fuzz driver to test a library with extensive code coverage and robust error handling."
            "Please generate an executable {lang} fuzz driver according to the following instructions:\n"
            "1. Create a function named `LLVMFuzzerTestOneInput` that achieves a task using the provided API combination. Each API should be called at least once. The function signature must be `int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)`.\n"
            "2. Ensure the generated code correctly utilizes the fuzz driver inputs, `const uint8_t *data` and `size_t size`.\n"
            "3. API inputs must derive from the fuzz driver inputs, `const uint8_t *data` and `size_t size`.\n"
            "4. Include all the provided headers at the beginning of the file.\n"
            "5. Do NOT reimplement, mock, or redefine the target APIs, their structs, or allocators. Only call the real APIs via their headers; if a required header is missing, include it explicitly instead of writing your own stubs.\n"
            "6. The code should be complete and executable without requiring manual completion by the developer.\n"
            "7. Implement robust error handling for all API calls. Check return values and handle potential errors appropriately.\n"
            "8. Avoid using incomplete types. If a type's size is unknown, use opaque pointers and the library's provided functions for allocation and deallocation.\n"
            "9. Prevent buffer overflows by carefully managing buffer sizes and using safe string functions.\n"
            "10. Ensure proper memory management: allocate memory as needed and free all allocated resources before the function returns.\n"
            "11. Implement proper initialization of variables and structures to avoid undefined behavior.\n"
            "12. Add appropriate bounds checking before accessing arrays or performing pointer arithmetic.\n"
            "13. If the API requires supporting types (e.g., rcutils allocator), include the official headers (e.g., <rcutils/allocator.h>) and use the library-provided helpers instead of custom replacements.\n"
            "I will provide the API combination, headers, API source code, and API summary below.\n"
            "API Combination:\n"
            "{api_list}\n\n"
            "Below are the system headers, API source code, and API summary required for the fuzz driver. Use all provided content to ensure the correctness of the generated fuzz driver:\n"
            "Provided headers (Include all header files to ensure the executability of the fuzz driver):\n"
            "{headers}\n\n"
            "API Source Code:\n"
            "```{lang}"
            "{api_info}\n"
            "```\n\n"
            "API Summary:\n"
            "{api_sum}\n\n"
            "Below is the historical context:\n"
            "Start\n"
            "{memory_context}\n"
            "End\n\n"
            "If file operations are required, you firstly need to convert the fuzz input into a string and create the corresponding object (e.g., TIFFStreamOpen()) directly in memory with the string rather than reading input files from disk. If an output file is needed, name it uniformly as 'output_file.'\n"
            "Add any non-code content as comments. Generate an executable {lang} fuzz driver according to the above descriptions, focusing on safety, proper resource management, and error handling."
        )

    def fuzz_driver_generation(self, api_list, api_info, headers, api_sum):
        '''
        根据提供的 API 信息（源码、摘要）,项目名称 自动生成 fuzz driver。
        参数：
            api_list:      组合中使用的 API 名称列表
            api_info:      这些 API 的源码拼# Optional libFuzzer target for string APIs
option(ROSIDL_RUNTIME_C_ENABLE_FUZZ "Build libFuzzer targets for rosidl_runtime_c" OFF)
if(ROSIDL_RUNTIME_C_ENABLE_FUZZ AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_executable(fuzz_string fuzz/string_fuzzer.c)
  target_link_libraries(fuzz_string PRIVATE ${PROJECT_NAME} -fsanitize=fuzzer,address,undefined)
  target_compile_options(fuzz_string PRIVATE -fsanitize=fuzzer,address,undefined)
  # Ensure the installed binary can locate shared libs from its own directory tree.
  set_target_properties(fuzz_string PROPERTIES
    BUILD_RPATH "$ORIGIN;$ORIGIN/.."
    # Covers local lib dir, parent, and common dependency install locations.
    INSTALL_RPATH
      "$ORIGIN;$ORIGIN/..;$ORIGIN/../../../rcutils/lib;$ORIGIN/../../../rosidl_typesupport_interface/lib"
  )
  install(TARGETS fuzz_string RUNTIME DESTINATION lib/${PROJECT_NAME})
endif()接文本
            headers:       请求中使用的头部信息(config中的headers)
            api_sum:       这些 API 的摘要文本
        返回：
            fuzz_driver_generation_response: 生成的 fuzz driver 代码（字符串形式）
        '''
        question = self.fuzz_driver_generation_prompt.format(lang=self.language, api_list=api_list, headers=headers, api_info=api_info, api_sum=api_sum)
        if self.use_memory:
            memory_chamessage = self.composable_memory.get(question)
            if len(memory_chamessage):
                memory_chamessage = "\n".join([str(m) for m in memory_chamessage])
                question = self.fuzz_driver_generation_prompt_with_memory.format(lang=self.language, api_list=api_list, memory_context=memory_chamessage, headers=headers, api_info=api_info, api_sum=api_sum)
        logger.info("Question:")
        logger.info(question)
        # 调用 LLM coder版本 来生成测试程序
        fuzz_driver_generation_response = self.llm_coder.complete(question).text
        logger.info("Generated Fuzz Driver:")
        logger.info(fuzz_driver_generation_response)
        query_answer = [
            ChatMessage.from_str(question, "user"),
            ChatMessage.from_str(fuzz_driver_generation_response, "assistant"),
        ]
        self.vector_memory.put_messages(query_answer)
        return fuzz_driver_generation_response

    def extract_code(self, s):
        '''
        从包含代码块的字符串中提取 C/C++ 源代码。
        参数：
            s: 包含 LLM 返回内容的字符串（通常是带 Markdown 格式的代码块）
        返回：
            匹配到的代码字符串
            若未匹配到，返回 "No code found"
        '''
        # 定义正则模式，匹配 C/C++ Markdown 代码块
        # 解释：
        # - ```         匹配 Markdown 代码块起始标志
        # - (?:c|cpp|c\+\+) 非捕获组，匹配三种语言标记：c、cpp、c++
        # - \s           空格，表示语言声明后有空格
        # - (.*?)        非贪婪匹配代码本体（捕获组1）
        # - ```         匹配代码块结束标志
        pattern = r'```(?:c|cpp|c\+\+)\s(.*?)```'
        match = re.search(pattern, s, re.DOTALL)
        if match:
            return match.group(1)
        else:
            return "No code found"
    # 根据 API 组合结果,生成对应的 fuzz driver,并保存为文件
    def driver_gen(self, api_combination, api_code, api_summary, fuzz_gen_code_output_dir,project):
        i = 1
        # 遍历 API 组合列表
            # 1. 获取 API 源代码
            # 2. 获取 API 描述
            # 3. 生成 fuzz driver
            # 4. 保存 fuzz driver 为文件
        for api_list in api_combination:
            api_list = list(set(api_list))  # 去重，防止同一个 API 被重复使用（因为在生成API联合体时，会把API再次添加）
            api_list_proc = []              # 实际处理成功的 API 列表
            api_info = ""                   # 存储 API 的源码片段
            api_sum = ""                    # 存储 API 的功能摘要
            for api in api_list:
                for file_key in api_summary.keys():
                    summary = api_summary[file_key].get(api, None)
                    if summary:
                        single_api_sum = f"{api}:\n{summary}"
                        single_api_info = f"{api}:\n{api_code[api]}"
                        api_sum = "\n".join([api_sum, single_api_sum])
                        api_info = "\n".join([api_info, single_api_info])
                        api_list_proc.append(api)
                        break
                 
            if api_list_proc:
                # self.headers -> config 配置文件中的头文件列表
                fuzz_driver_generation_response = self.fuzz_driver_generation(api_list_proc, api_info, self.headers, api_sum)
                # 从 LLM 返回内容(一般都为 Markdown格式)中提取出 C/C++ 代码
                fuzz_driver_generation_response = self.extract_code(str(fuzz_driver_generation_response))
                logger.info(fuzz_driver_generation_response)
                model_name = self.llm_coder.model.replace(":", "_")
                safe_project = project.replace("/", "_")
                fuzzer_name = f"{safe_project}_fuzz_driver_{self.use_memory}_{model_name}_{i}.{self.file_suffix[self.language.lower()]}"
                fuzzer_path = os.path.join(fuzz_gen_code_output_dir, fuzzer_name)
                with open(fuzzer_path, "w") as f:
                    f.write(fuzz_driver_generation_response)
                compile_args = self._build_compile_args(project, fuzzer_path)
                if compile_args:
                    compile_ok = self._check_compile(compile_args)
                    fix_attempts = 0
                    while compile_ok == 0 and fix_attempts < 5:
                        error_message = self._read_compile_output(compile_args.output)
                        if not self.fix_compilation(fuzzer_path, error_message):
                            break
                        compile_ok = self._check_compile(compile_args)
                        fix_attempts += 1
                    if compile_ok == 0:
                        logger.warning(f"Compilation failed after {fix_attempts} fixes, delete {fuzzer_path}")
                        os.remove(fuzzer_path)
                i += 1
            else:
                return False

    def generate_single_fuzz_driver(self, api_list, fuzzer_name, api_code, api_summary, fuzz_gen_code_output_dir):
        api_list = list(set(api_list))
        api_info = ""
        api_sum = ""
        api_list_proc = []
        
        for api in api_list:
            for file_key in api_summary.keys():
                summary = api_summary[file_key].get(api, None)
                if summary:
                    single_api_sum = f"{api}:\n{summary}"
                    single_api_info = f"{api}:\n{api_code[api]}"
                    api_sum = "\n".join([api_sum, single_api_sum])
                    api_info = "\n".join([api_info, single_api_info])
                    api_list_proc.append(api)
                    break

        if api_list_proc:
            fuzz_driver_generation_response = self.fuzz_driver_generation(api_list_proc, api_info, self.headers, api_sum)
            fuzz_driver_generation_response = self.extract_code(str(fuzz_driver_generation_response))
            logger.info(fuzz_driver_generation_response)
            fuzzer_path = os.path.join(fuzz_gen_code_output_dir, fuzzer_name)
            with open(fuzzer_path, "w") as f:
                f.write(fuzz_driver_generation_response)
            project_name = self.project_config.get("project_name")
            if not project_name:
                logger.warning("Missing project_name in config, skip compilation check.")
            else:
                compile_args = self._build_compile_args(project_name, fuzzer_path)
                if compile_args:
                    compile_ok = self._check_compile(compile_args)
                    fix_attempts = 0
                    while compile_ok == 0 and fix_attempts < 5:
                        error_message = self._read_compile_output(compile_args.output)
                        if not self.fix_compilation(fuzzer_path, error_message):
                            break
                        compile_ok = self._check_compile(compile_args)
                        fix_attempts += 1
                    if compile_ok == 0:
                        logger.warning(f"Compilation failed after {fix_attempts} fixes, delete {fuzzer_path}")
                        os.remove(fuzzer_path)
        else:
            return False
