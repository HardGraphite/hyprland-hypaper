#pragma once

#include <hyprland/src/debug/Log.hpp>

#ifdef NDEBUG

#define hypaper_log(...) ((void)0)

#else // NDEBUG

#define hypaper_log(__format, ...) \
    (Debug::log(LOG, "[HyPaper] " __format __VA_OPT__(,) __VA_ARGS__))

#endif // NDEBUG
