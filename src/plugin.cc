#include "hypaper.h"

#include <hyprland/src/plugins/PluginAPI.hpp>

using namespace hypaper;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    hypaper::initialize(handle);

    return {
        .name        = "HyPaper",
        .description = "A scrollable tilling layout.",
        .author      = "HardGraphite",
        .version     = "0.1",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    hypaper::finalize();
}
