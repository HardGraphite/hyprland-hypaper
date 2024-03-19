#include "config.h"

#include <climits>

#include <hyprland/src/config/ConfigDataValues.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#include "hypaper.h"

using namespace hypaper;

static SConfigValue *column_width;
static SConfigValue *general_gaps_in;
static SConfigValue *general_gaps_out;

static SConfigValue *add_conf_var(const std::string &name, const SConfigValue &val) {
    HyprlandAPI::addConfigValue(hyprland_handle, name, val);
    return HyprlandAPI::getConfigValue(hyprland_handle, name);
}

static SConfigValue *get_conf_var(const std::string &name) {
    return HyprlandAPI::getConfigValue(hyprland_handle, name);
}

void conf::_register() {
    using namespace std::string_literals;
    ::column_width = add_conf_var("plugin:hypaper:column_width"s, {.floatValue = 1.0});
    ::general_gaps_in = get_conf_var("general:gaps_in"s);
    ::general_gaps_out = get_conf_var("general:gaps_out"s);
}

double conf::column_width() noexcept {
    const auto x = ::column_width->floatValue;
    return x > 0.0 ? x : 1.0;
}

unsigned int conf::gaps_in() noexcept {
    const auto x = ::general_gaps_in->intValue;
    if (0 <= x && x <= INT_MAX)
        return static_cast<unsigned int>(x);
    return 0;
}

unsigned int conf::gaps_out() noexcept {
    const auto x = ::general_gaps_out->intValue;
    if (0 <= x && x <= INT_MAX)
        return static_cast<unsigned int>(x);
    return 0;
}
