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
    """Start container and copy fuzz assets."""
    # Ensure image exists.
    _run(["docker", "image", "inspect", args.image])

    # Start a long-running container so we can copy files in/out.
    _run(["docker", "run", "-d", "--name", container_name, args.image, "sleep", "infinity"])

    target_fuzz_dir = f"{args.container_path.rstrip('/')}/fuzz"
    _run(["docker", "exec", container_name, "mkdir", "-p", target_fuzz_dir])

    # Copy fuzz driver.
    driver_dest = f"{container_name}:{target_fuzz_dir}/{Path(args.fuzz_driver).name}"
    _run(["docker", "cp", str(Path(args.fuzz_driver).resolve()), driver_dest])

    # Replace CMakeLists.txt.
    cmake_dest = f"{container_name}:{args.container_path.rstrip('/')}/CMakeLists.txt"
    _run(["docker", "cp", str(Path(args.cmakelists).resolve()), cmake_dest])

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
        fuzzing_returncode, fuzzing_output, container_output = _stream_docker_exec(
            container_name, "./fuzzing.sh", args.workdir, container_output
        )

        # Copy the tee'd output file back to host.
        _run(["docker", "cp", f"{container_name}:{container_output}", str(host_output)], allow_error=True)
    finally:
        # Cleanup container regardless of success.
        _run(["docker", "rm", "-f", container_name], allow_error=True)

    # Always write captured output to host file (even if docker cp failed).
    with host_output.open("w", encoding="utf-8") as f:
        f.write(fuzzing_output)

    return fuzzing_returncode


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


def main():
    args = parse_args()

    # Validate input paths on host.
    for path_arg, label in [(args.fuzz_driver, "fuzz driver"), (args.cmakelists, "CMakeLists.txt")]:
        if not Path(path_arg).is_file():
            raise FileNotFoundError(f"Missing {label} file: {path_arg}")

    returncode = run_fuzzing(args)
    if returncode != 0:
        raise SystemExit(returncode)


if __name__ == "__main__":
    main()
