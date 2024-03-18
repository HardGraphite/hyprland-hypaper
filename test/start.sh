#!/bin/sh

HYPR_CONF_FILE="$(dirname $(realpath "$0"))/hyprland.conf"
export HYPAPER_PROJECT_DIR="$(dirname $(dirname $(realpath "$0")))"
export HYPAPER_TEST_INIT_SCRIPT="$HYPAPER_PROJECT_DIR/test/init.sh"

echo 'Start a nested Hyprland instance in 3 seconds...'
echo 'Press Alt+Q to exit.'
sleep 3s

Hyprland --config "$HYPR_CONF_FILE"
