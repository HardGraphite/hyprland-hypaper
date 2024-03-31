#include "hypaper.h"

#include <hyprland/src/plugins/PluginAPI.hpp>

#include "column.h"
#include "config.h"
#include "dispatchers.h"
#include "indicator.h"
#include "layout.h"
#include "workbench.h"

using namespace hypaper;

HANDLE hypaper::hyprland_handle = nullptr;
Layout *hypaper::layout = nullptr;
Indicator *hypaper::indicator = nullptr;

void hypaper::initialize(HANDLE h) {
    hyprland_handle = h;
    conf::_register();
    hypaper::layout = new Layout;
    hypaper::indicator = new Indicator;
    HyprlandAPI::addLayout(h, Layout::name, hypaper::layout);
    register_dispatchers();
}

void hypaper::finalize() {
    HyprlandAPI::removeLayout(hyprland_handle, hypaper::layout);
    delete hypaper::layout;
    delete hypaper::indicator;
    hypaper::layout = nullptr;
    hypaper::indicator = nullptr;
}
