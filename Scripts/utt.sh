#!/bin/sh

print_usage() {
    echo "Usage: $0 <command> [args...]"
    echo
    echo "Available commands:"
    echo "  generate [preset]   Generate the project using CMake preset"
    echo "  clean               Remove build directories"
    echo "  compile_commands    Generate compile_commands.json symlink"
    exit 1
}

setup_path() {
    SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
    ROOT_DIR="$(dirname "$SCRIPT_DIR")"
    cd "$ROOT_DIR" || exit 1
}

DEFAULT_PRESET="test"

generate_project() {
    PRESET="$1"
    if [ -z "$PRESET" ]; then
        echo "[Build] Using default preset."
        cmake -S ./Worse --preset="$DEFAULT_PRESET"
    else
        echo "[Build] Using preset: $PRESET"
        cmake -S ./Worse --preset="$PRESET"
    fi

}

clean_project() {
    BUILD_DIR="./Worse/Build"

    if [ -t 0 ]; then
        printf "Sure to remove build directory '%s'? [y/N] " "$BUILD_DIR"
        read -r CONFIRM
    else
        CONFIRM="n"
    fi

    case "$CONFIRM" in
        [yY]|[yY][eE][sS])
            echo "[Clean] Removing $BUILD_DIR ..."
            rm -rf "$BUILD_DIR"
            ;;
        *)
            echo "[Clean] Canceled."
            ;;
    esac
}

generate_compile_commands() {
    BUILD_DIR="./Worse/Build"

    if [ ! -d "$BUILD_DIR" ]; then
        echo "Error: Build directory '$BUILD_DIR' does not exist."
        exit 1
    fi

    ln -sf "$BUILD_DIR/compile_commands.json" "./compile_commands.json"
    echo "[Generate] Symlinked compile_commands.json to project root."
}

if [ $# -lt 1 ]; then
    print_usage
fi

COMMAND="$1"
shift

setup_path

case "$COMMAND" in
    generate)
        generate_project "$@"
        ;;
    clean)
        clean_project
        ;;
    compile_commands)
        generate_compile_commands
        ;;
    *)
        echo "Unknown command: $COMMAND"
        print_usage
        ;;
esac