#!/bin/bash

HYPR_CONF_FILE="$(dirname $(realpath "$0"))/hyprland.conf"
export HYPAPER_PROJECT_DIR="$(dirname $(dirname $(realpath "$0")))"
export HYPAPER_TEST_INIT_SCRIPT="$HYPAPER_PROJECT_DIR/test/init.sh"

case "$1" in
gdb)
    gdb --eval-command 'catch signal SIGSEGV SIGABRT' \
        --args Hyprland --config "$HYPR_CONF_FILE"
    ;;
valgrind)
    valgrind --leak-check=full -- Hyprland --config "$HYPR_CONF_FILE"
    ;;
*)
    echo 'Start a nested Hyprland instance in 3 seconds...'
    echo 'Press Alt+Q to exit.'
    echo 'PID =' $$
    sleep 3s
    exec Hyprland --config "$HYPR_CONF_FILE"
    ;;
esac
