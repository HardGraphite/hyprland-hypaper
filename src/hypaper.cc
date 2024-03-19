#include "hypaper.h"

#include <hyprland/src/plugins/PluginAPI.hpp>

#include "column.h"
#include "config.h"
#include "dispatchers.h"
#include "layout.h"
#include "workbench.h"

using namespace hypaper;

HANDLE hypaper::hyprland_handle = nullptr;
Layout *hypaper::layout = nullptr;

void hypaper::initialize(HANDLE h) {
    hyprland_handle = h;
    hypaper::layout = new Layout;
    HyprlandAPI::addLayout(h, Layout::name, hypaper::layout);
    conf::_register();
    register_dispatchers();
}

void hypaper::finalize() {
    HyprlandAPI::removeLayout(hyprland_handle, hypaper::layout);
    delete hypaper::layout;
    hypaper::layout = nullptr;
}
