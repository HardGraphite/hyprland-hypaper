#include "hypaper.h"

#include <hyprland/src/plugins/PluginAPI.hpp>

#include "column.h"
#include "config.h"
#include "layout.h"
#include "workbench.h"


using namespace hypaper;

HANDLE hypaper::hyprland_handle = nullptr;

static Layout *layout = nullptr;

void hypaper::initialize(HANDLE h) {
    hyprland_handle = h;
    layout = new Layout;
    HyprlandAPI::addLayout(h, Layout::name, layout);
    conf::_register();
}

void hypaper::finalize() {
    HyprlandAPI::removeLayout(hyprland_handle, layout);
    delete layout;
    layout = nullptr;
}
