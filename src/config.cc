#include "config.h"

#include <climits>

#include <hyprland/src/config/ConfigDataValues.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprlang.hpp>

#include "hypaper.h"

using namespace hypaper;

static Hyprlang::CConfigValue *column_width;
static Hyprlang::CConfigValue *general_gaps_in;
static Hyprlang::CConfigValue *general_gaps_out;

static Hyprlang::CConfigValue *add_conf_var(const std::string &name, const Hyprlang::CConfigValue &val) {
    HyprlandAPI::addConfigValue(hyprland_handle, name, val);
    return HyprlandAPI::getConfigValue(hyprland_handle, name);
}

static Hyprlang::CConfigValue *get_conf_var(const std::string &name) {
    return HyprlandAPI::getConfigValue(hyprland_handle, name);
}

void conf::_register() {
    using namespace std::string_literals;
    ::column_width = add_conf_var("plugin:hypaper:column_width"s, 1.0f);
    ::general_gaps_in = get_conf_var("general:gaps_in"s);
    ::general_gaps_out = get_conf_var("general:gaps_out"s);
}

double conf::column_width() noexcept {
    const auto x = *static_cast<Hyprlang::FLOAT *>(::column_width->dataPtr());
    return x > 0.0 ? x : 1.0;
}

unsigned int conf::gaps_in() noexcept {
    const auto x = static_cast<CCssGapData *>(
        static_cast<Hyprlang::CUSTOMTYPE *>(general_gaps_in->dataPtr())->getData()
    )->top;
    if (0 <= x && x <= INT_MAX)
        return static_cast<unsigned int>(x);
    return 0;
}

unsigned int conf::gaps_out() noexcept {
    const auto x = static_cast<CCssGapData *>(
        static_cast<Hyprlang::CUSTOMTYPE *>(general_gaps_out->dataPtr())->getData()
    )->top;
    if (0 <= x && x <= INT_MAX)
        return static_cast<unsigned int>(x);
    return 0;
}
