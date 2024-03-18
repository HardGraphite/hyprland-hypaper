#include "config.h"

#include <hyprland/src/config/ConfigDataValues.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#include "hypaper.h"

using namespace hypaper;

static SConfigValue *column_width;

static SConfigValue *add_conf_var(const std::string &name, const SConfigValue &val) {
    HyprlandAPI::addConfigValue(hyprland_handle, name, val);
    return HyprlandAPI::getConfigValue(hyprland_handle, name);
}

void conf::_register() {
    using namespace std::string_literals;
    ::column_width = add_conf_var("plugin:hypaper:column_width"s, {.floatValue = 1.0});
}

double conf::column_width() noexcept {
    const auto x = ::column_width->floatValue;
    return x > 0.0 ? x : 1.0;
}
