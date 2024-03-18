#!/bin/sh

make -C "$HYPAPER_PROJECT_DIR" debug
make -C "$HYPAPER_PROJECT_DIR" plugin-load
make -C "$HYPAPER_PROJECT_DIR" plugin-layout
