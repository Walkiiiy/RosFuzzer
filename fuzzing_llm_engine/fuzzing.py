import argparse
import subprocess
import sys
import uuid
from pathlib import Path


def _run(cmd, capture_output=False, allow_error=False):
    """Run a shell command and optionally capture stdout/stderr."""
    process = subprocess.run(
        cmd,
        stdout=subprocess.PIPE if capture_output else None,
        stderr=subprocess.STDOUT if capture_output else None,
        text=True,
    )
    output = process.stdout or ""
    if process.returncode != 0 and not allow_error:
        raise RuntimeError(f"Command failed ({process.returncode}): {' '.join(cmd)}\n{output}")
    return process.returncode, output


def _stream_docker_exec(container_name, command, container_workdir=None, tee_path="/tmp/fuzz_output.txt"):
    """Execute a command in the container, stream output locally, and tee into a file inside the container."""
    workdir_prefix = f"cd {container_workdir} && " if container_workdir else ""
    exec_cmd = [
        "docker",
        "exec",
        container_name,
        "bash",
        "-lc",
        f"{workdir_prefix}{command} | tee {tee_path}",
    ]
    process = subprocess.Popen(exec_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    captured = []
    for line in iter(process.stdout.readline, ""):
        sys.stdout.write(line)
        captured.append(line)
    process.wait()
    return process.returncode, "".join(captured), tee_path


def prepare_container(args, container_name):
    '''
    Start container and copy fuzz assets.
    copy fuzz driver, CMakeLists.txt, and optional fuzzing.sh into container.
    '''
    # Ensure image exists.
    _run(["docker", "image", "inspect", args.image])

    # Start a long-running container so we can copy files in/out.
    _run(["docker", "run", "-d", "--name", container_name, args.image, "sleep", "infinity"])

    target_fuzz_dir = f"{args.container_path.rstrip('/')}/fuzz"
    _run(["docker", "exec", container_name, "mkdir", "-p", target_fuzz_dir])

    # Copy fuzz driver (rename to configurable target source name).
    driver_dest_name = args.driver_dest_name
    driver_dest = f"{container_name}:{target_fuzz_dir}/{driver_dest_name}"
    _run(["docker", "cp", str(Path(args.fuzz_driver).resolve()), driver_dest])

    # Replace CMakeLists.txt.
    cmake_dest = f"{container_name}:{args.container_path.rstrip('/')}/CMakeLists.txt"
    _run(["docker", "cp", str(Path(args.cmakelists).resolve()), cmake_dest])

    # Copy fuzzing.sh if provided.
    if args.fuzz_script:
        workdir = args.workdir.rstrip("/") if args.workdir else "/"
        script_dest = f"{container_name}:{workdir}/fuzzing.sh"
        _run(["docker", "cp", str(Path(args.fuzz_script).resolve()), script_dest])

    return target_fuzz_dir


def run_fuzzing(args):
    container_name = args.container_name or f"rosidl_runtime_c_fuzz_{uuid.uuid4().hex[:8]}"
    host_output = Path(args.output).resolve()
    host_output.parent.mkdir(parents=True, exist_ok=True)

    container_output = "/tmp/fuzzing_result.txt"
    fuzzing_returncode = None
    fuzzing_output = ""

    try:
        prepare_container(args, container_name)
        # Ensure fuzzing.sh is executable and run through bash with envs to avoid permission issues.
        fuzz_cmd = (
            "chmod +x ./fuzzing.sh || true; "
            f"FUZZ_TIMEOUT={args.timeout_seconds} "
            f"FUZZ_TARGET_NAME={args.target_binary} "
            f"COVERAGE_OUT={args.coverage_container_path} "
            f"COVERAGE_TXT={args.coverage_text_container_path} "
            "bash ./fuzzing.sh"
        )
        fuzzing_returncode, fuzzing_output, container_output = _stream_docker_exec(
            container_name, fuzz_cmd, args.workdir, container_output
        )

        # Copy the tee'd output file back to host.
        _run(["docker", "cp", f"{container_name}:{container_output}", str(host_output)], allow_error=True)

        # Copy coverage output if present.
        if args.coverage_output:
            _run(
                [
                    "docker",
                    "cp",
                    f"{container_name}:{args.coverage_container_path}",
                    str(Path(args.coverage_output).resolve()),
                ],
                allow_error=True,
            )

        if args.coverage_text_output:
            _run(
                [
                    "docker",
                    "cp",
                    f"{container_name}:{args.coverage_text_container_path}",
                    str(Path(args.coverage_text_output).resolve()),
                ],
                allow_error=True,
            )
    finally:
        # Cleanup container regardless of success.
        _run(["docker", "rm", "-f", container_name], allow_error=True)

    # Always write captured output to host file (even if docker cp failed).
    with host_output.open("w", encoding="utf-8") as f:
        f.write(fuzzing_output)

    return fuzzing_returncode, host_output


def parse_args():
    parser = argparse.ArgumentParser(description="Run rosidl_runtime_c fuzzing inside docker.")
    parser.add_argument("--image", default="rosidl_runtime_c_base_image:latest", help="Docker image to run.")
    parser.add_argument(
        "--container-path",
        required=True,
        help="Target directory inside the container where fuzz/ and CMakeLists.txt should reside.",
    )
    parser.add_argument("--fuzz-driver", required=True, help="Host path to the fuzz driver source file.")
    parser.add_argument("--cmakelists", required=True, help="Host path to the replacement CMakeLists.txt.")
    parser.add_argument(
        "--fuzz-script",
        default=str(Path(__file__).parent / "projects/rosidl_runtime_c/fuzzing.sh"),
        help="Host path to fuzzing.sh to copy into container (defaults to project script).",
    )
    parser.add_argument(
        "--driver-dest-name",
        default="fuzzer.c",
        help="Destination filename inside container fuzz/ directory (matches target source name).",
    )
    parser.add_argument(
        "--target-binary",
        default="fuzzer",
        help="Fuzz target binary name inside install dir (used for coverage export).",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=120,
        help="Fuzzing run timeout inside container (seconds).",
    )
    parser.add_argument(
        "--coverage-output",
        default="docker_shared/rosidl_coverage.json",
        help="Host path to save coverage json (if produced).",
    )
    parser.add_argument(
        "--coverage-container-path",
        default="/ws/fuzz_corpus/coverage.json",
        help="Container path where fuzzing.sh writes coverage json.",
    )
    parser.add_argument(
        "--coverage-text-output",
        default="docker_shared/rosidl_coverage.txt",
        help="Host path to save human-readable coverage report (if produced).",
    )
    parser.add_argument(
        "--coverage-text-container-path",
        default="/ws/fuzz_corpus/coverage.txt",
        help="Container path where fuzzing.sh writes text coverage report.",
    )
    parser.add_argument(
        "--output",
        default="docker_shared/fuzzing_output.txt",
        help="Host path to save fuzzing.sh execution output.",
    )
    parser.add_argument(
        "--container-name",
        help="Optional container name to reuse; defaults to a temporary unique name.",
    )
    parser.add_argument(
        "--workdir",
        help="Optional working directory inside the container before running fuzzing.sh. "
        "If omitted, uses the image default.",
    )
    return parser.parse_args()


def _read_lines(path):
    if not path.is_file():
        return []
    with path.open("r", encoding="utf-8", errors="replace") as f:
        return f.readlines()


def _extract_error_context(lines, keywords, context_lines=20, max_line_length=300):
    for idx, line in enumerate(lines):
        if any(keyword in line for keyword in keywords):
            start = max(0, idx - context_lines)
            end = min(len(lines), idx + context_lines + 1)
            context = lines[start:end]
            truncated = []
            for item in context:
                if len(item) > max_line_length:
                    truncated.append(item[:max_line_length] + "...\n")
                else:
                    truncated.append(item)
            return truncated
    return []


def _compile_failed_from_output(output_path):
    lines = _read_lines(output_path)
    error_context = _extract_error_context(
        lines, keywords=("package failed:", "ERROR:"), context_lines=20, max_line_length=300
    )
    if error_context:
        return True, error_context
    return False, error_context


def fuzz_runner(
    image="rosidl_runtime_c_base_image:latest",
    container_path="/ws/src/ros2/rosidl/rosidl_runtime_c",
    fuzz_driver="/home/walkiiiy/RosFuzzer/fuzzing_llm_engine/external_database/rosidl_runtime_c/fuzz_driver/ros2_rosidl_runtime_c_fuzz_driver_False_deepseek-coder_1.c",
    cmakelists="/home/walkiiiy/RosFuzzer/fuzzing_llm_engine/projects/rosidl_runtime_c/CMakeLists.txt",
    fuzz_script=str(Path(__file__).parent / "projects/rosidl_runtime_c/fuzzing.sh"),
    driver_dest_name="fuzzer.c",
    target_binary="fuzzer",
    timeout_seconds=5,
    coverage_output="docker_shared/rosidl_coverage.json",
    coverage_container_path="/ws/fuzz_corpus/coverage.json",
    coverage_text_output="docker_shared/rosidl_coverage.txt",
    coverage_text_container_path="/ws/fuzz_corpus/coverage.txt",
    output="docker_shared/rosidl_fuzz_output.txt",
    container_name=None,
    workdir="/ws",
):
    args = argparse.Namespace(
        image=image,
        container_path=container_path,
        fuzz_driver=fuzz_driver,
        cmakelists=cmakelists,
        fuzz_script=fuzz_script,
        driver_dest_name=driver_dest_name,
        target_binary=target_binary,
        timeout_seconds=timeout_seconds,
        coverage_output=coverage_output,
        coverage_container_path=coverage_container_path,
        coverage_text_output=coverage_text_output,
        coverage_text_container_path=coverage_text_container_path,
        output=output,
        container_name=container_name,
        workdir=workdir,
    )

    # Validate input paths on host.
    for path_arg, label in [(args.fuzz_driver, "fuzz driver"), (args.cmakelists, "CMakeLists.txt")]:
        if not Path(path_arg).is_file():
            raise FileNotFoundError(f"Missing {label} file: {path_arg}")

    returncode, output_path = run_fuzzing(args)
    if returncode != 0:
        raise SystemExit(returncode)

    res,error=_compile_failed_from_output(output_path)
    if res:
        print("Fuzz driver compilation failed.")
        return 0, error
    print("Fuzz driver compilation succeeded.")
    return 1, error


if __name__ == "__main__":
    print(fuzz_runner())
