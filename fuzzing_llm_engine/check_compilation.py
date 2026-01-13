from pathlib import Path
import os
import time
import yaml
from fuzzing import fuzz_runner
from models.get_model import get_model
from roles.compilation_fix_agent import extract_code

def check_compilation(fuzz_driver_dir):
    report = []
    for driver in fuzz_driver_dir.iterdir():
        start_time = time.time()
        iteration=0
        result=False
        while not result and iteration< 2:
            result, error = fuzz_runner(
                image="rosidl_runtime_c_base_image:latest",
                container_path="/ws/src/ros2/rosidl/rosidl_runtime_c",
                fuzz_driver=str(driver),
                cmakelists="/home/walkiiiy/RosFuzzer/fuzzing_llm_engine/projects/rosidl_runtime_c/CMakeLists.txt",
                fuzz_script=str(Path(__file__).parent / "projects/rosidl_runtime_c/fuzzing.sh"),
                driver_dest_name="fuzzer.c",
                target_binary="fuzzer",
                timeout_seconds=10,
                coverage_text_output='docker_shared/rosidl_coverage_'+driver.name+'.txt',
            )
            if result:
                print(f"Compilation succeeded for driver: {driver.name}")
                break
            else:
                print(f"Compilation failed for driver: {driver.name}")
                print(f"Error details: {error}")
                fix_compilation_with_llm(driver,error)
                iteration+=1         
        duration = time.time() - start_time
        report.append(
            {
                "driver": driver.name,
                "success": bool(result),
                "iterations": iteration,
                "duration_seconds": duration,
            }
        )
        # break  # For testing, only check the first driver
    print("\n=== Compilation Report ===")
    for item in report:
        status = "success" if item["success"] else "failed"
        print(
            f"{item['driver']}: {status}, iterations={item['iterations']}, "
            f"duration_seconds={item['duration_seconds']:.2f}"
        )

def fix_compilation_with_llm(driver, error):
    config_path = Path(__file__).parent / "external_database/rosidl_runtime_c/config.yaml"
    if not config_path.is_file():
        raise FileNotFoundError(f"Missing config file: {config_path}")
    with config_path.open("r", encoding="utf-8") as f:
        config = yaml.safe_load(f)
    llm_coder = get_model(config["llm_coder"])

    with open(driver, "r", encoding="utf-8") as f:
        code = f.read()

    if isinstance(error, list):
        error_text = "".join(error)
    else:
        error_text = str(error)

    prompt = (
        "You are an expert C/C++ programmer specializing in fixing compilation errors. "
        "Fix the following fuzz driver using the compilation error messages.\n"
        "Fuzz driver:\n"
        "```\n"
        f"{code}\n"
        "```\n"
        "Compilation errors:\n"
        f"{error_text}\n"
        "Instructions:\n"
        "1. Make minimal necessary changes to fix the compilation issues.\n"
        "2. Do not modify or remove any #include statements.\n"
        "3. Ensure the fixes address all reported errors.\n"
        "4. Return only the complete, fixed code wrapped in triple backticks (```).\n"
        "Fix the code now:"
    )
    response = llm_coder.complete(prompt).text
    fixed_code = extract_code(response)
    if fixed_code == "No code found":
        print("LLM did not return code; keeping original driver.")
        return
    with open(driver, "w", encoding="utf-8") as f:
        f.write(fixed_code)

if __name__=='__main__':

    fuzz_driver_dir = Path("/home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c/fuzz_driver")
    if not fuzz_driver_dir.exists():
        print('Fuzz driver directory does not exist.')

    check_compilation(fuzz_driver_dir)
