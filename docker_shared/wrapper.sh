#!/bin/bash
set -euo pipefail

# The wrapper is invoked by CodeQL to record build commands. It needs to work
# for both OSS-Fuzz style projects and ROS2 workspaces where the source may
# live under a nested /ws/src/... tree. The caller can optionally set
# TARGET_SRC_PATH to point to the package being analyzed.

PROJECT_NAME="${1:-}"
TARGET_SRC_PATH="${TARGET_SRC_PATH:-}"

if [[ -n "$TARGET_SRC_PATH" ]]; then
    # Normalize to an absolute path if possible.
    TARGET_SRC_PATH="$(realpath "$TARGET_SRC_PATH" 2>/dev/null || echo "$TARGET_SRC_PATH")"
fi

# Detect a workspace root from TARGET_SRC_PATH if it contains /src/
WORKSPACE_ROOT=""
if [[ -n "$TARGET_SRC_PATH" && "$TARGET_SRC_PATH" == *"/src/"* ]]; then
    WORKSPACE_ROOT="${TARGET_SRC_PATH%%/src/*}"
fi

# Prefer to run from a workspace root so colcon can resolve dependencies.
if [[ -n "$WORKSPACE_ROOT" && -d "$WORKSPACE_ROOT" ]]; then
    cd "$WORKSPACE_ROOT"
elif [[ -d "/ws" ]]; then
    cd /ws
elif [[ -n "$TARGET_SRC_PATH" && -d "$TARGET_SRC_PATH" ]]; then
    cd "$TARGET_SRC_PATH"
elif [[ -n "$PROJECT_NAME" && -d "/src/$PROJECT_NAME" ]]; then
    cd "/src/$PROJECT_NAME"
else
    echo "Error: could not locate a workspace to build ($TARGET_SRC_PATH)."
    exit 1
fi

# Source ROS environment if available to satisfy ament_* dependencies.
# Disable nounset temporarily because some setup scripts rely on unset vars.
set +u
if [[ -f "/opt/ros/humble/setup.bash" ]]; then
    source /opt/ros/humble/setup.bash
elif [[ -f "/opt/ros/foxy/setup.bash" ]]; then
    source /opt/ros/foxy/setup.bash
fi
set -u

# Choose the build entry point. Prefer a workspace-level build.sh, otherwise
# fall back to a package-local build or a colcon build command.
if [[ -f "./build.sh" ]]; then
    script_path="./build.sh"
elif [[ -f "/src/build.sh" ]]; then
    script_path="/src/build.sh"
elif [[ -n "$TARGET_SRC_PATH" && -f "$TARGET_SRC_PATH/build.sh" ]]; then
    script_path="$TARGET_SRC_PATH/build.sh"
else
    script_path=""
fi

if [[ -n "$script_path" ]]; then
    if grep -q "bazel_build_fuzz_tests" "$script_path"; then
        echo "The command 'bazel_build_fuzz_tests' is found in '$script_path'."
        cp /src/fuzzing_os/bazel_build /usr/local/bin/
        sed -i 's/exec bazel_build_fuzz_tests/exec bazel_build/g' "$script_path"
    else
        echo "The command 'bazel_build_fuzz_tests' is not found in '$script_path'."
    fi
    # Keep running even if the project build fails so CodeQL can still finalize.
    set +e
    bash "$script_path"
    set -e
else
    # Fallback for ROS workspaces: try building the target package directly.
    if command -v colcon >/dev/null 2>&1 && [[ -n "$PROJECT_NAME" ]]; then
        echo "No build.sh found; running colcon build for $PROJECT_NAME"
        set +e
        colcon build --merge-install --packages-select "$PROJECT_NAME" --cmake-args -DBUILD_TESTING=OFF
        set -e
    else
        echo "Error: no build script found and colcon unavailable."
        exit 1
    fi
fi

# Do not propagate build failures to CodeQL so database creation can continue.
exit 0
