#include "dispatchers.h"

#include <cctype>
#include <charconv>
#include <system_error>

#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

#include "column.h"
#include "hypaper.h"
#include "layout.h"
#include "logging.h"
#include "workbench.h"

using namespace hypaper;

static void cmd_column_width(std::string arg) {
    hypaper_log("{}({})", __func__, arg);

    double arg_width;
    if (std::from_chars(arg.c_str(), arg.c_str() + arg.size(), arg_width).ec != std::errc{})
        return;
    layout->cmd_column_width(arg_width);
}

static void cmd_absorb_window([[maybe_unused]] std::string arg) {
    hypaper_log("{}({})", __func__, arg);

    layout->cmd_absorb_window();
}

static void cmd_expel_window([[maybe_unused]] std::string arg) {
    hypaper_log("{}({})", __func__, arg);

    layout->cmd_expel_window();
}

static void cmd_scroll(std::string arg) {
    hypaper_log("{}({})", __func__, arg);

    Layout::ScrollArg scroll_arg;
    if (arg.empty()) {
        scroll_arg.type = Layout::ScrollArg::AUTO;
    } else if (arg.size() == 1 && std::ispunct(arg[0])) {
        switch (arg[0]) {
        case '=': scroll_arg.type = Layout::ScrollArg::AUTO; break;
        case '^': scroll_arg.type = Layout::ScrollArg::CENTER; break;
        case '<': scroll_arg.type = Layout::ScrollArg::ALIGN_L; break;
        case '>': scroll_arg.type = Layout::ScrollArg::ALIGN_R; break;
        default: return;
        }
    } else {
        scroll_arg.type = Layout::ScrollArg::OFFSET;
        if (std::from_chars(arg.c_str(), arg.c_str() + arg.size(), scroll_arg.value).ec != std::errc{})
            return;
    }
    layout->cmd_scroll(scroll_arg);
}

#ifndef NDEBUG

static void cmd_dump_layout([[maybe_unused]] std::string arg) {
    hypaper_log("{}({})", __func__, arg);

    hypaper_log("<Paper addr=\"{}\">", static_cast<void *>(layout));
    layout->foreach_workspace([](Workbench &wb) {
        hypaper_log(
            "  <Workbench addr=\"{}\" workspace=\"{}\" focus=\"{}\">",
            static_cast<void *>(&wb), wb.get_workspace_id(), wb.get_focused_column_index()
        );
        for (std::size_t col_i = 0, col_n = wb.count_columns(); col_i < col_n; col_i++) {
            auto &col = wb.get_column(col_i);
            hypaper_log(
                "    <Column addr=\"{}\" index=\"{}\" x=\"{}\" w=\"{}\" focus=\"{}\">",
                static_cast<void *>(&col), col_i, col.get_hposition(), col.get_width(),
                col.get_focused_window_index()
            );
            for (std::size_t win_i = 0, win_n = col.count_windows(); win_i < win_n; win_i++) {
                auto win = col.get_window(win_i);
                const auto x = win->m_vPosition.x, y = win->m_vPosition.y;
                const auto w = win->m_vSize.x, h = win->m_vSize.y;
                hypaper_log(
                    "      <Window addr=\"{}\" index=\"{}\" xywh=\"{},{},{},{}\"/>",
                    static_cast<void *>(win), win_i, x, y, w, h
                );
            }
            hypaper_log("    </Column>");
        }
        hypaper_log("  </Workbench>");
        return true;
    });
    hypaper_log("</Paper>");
}

#endif // NDEBUG

static void add_command(const std::string &name, void (func)(std::string)) {
    HyprlandAPI::addDispatcher(hyprland_handle, name, func);
}

void hypaper::register_dispatchers() {
    using namespace std::string_literals;
    add_command("hypaper:column_width"s, cmd_column_width);
    add_command("hypaper:absorb_window"s, cmd_absorb_window);
    add_command("hypaper:expel_window"s, cmd_expel_window);
    add_command("hypaper:scroll"s, cmd_scroll);
#ifndef NDEBUG
    add_command("hypaper:dump_layout"s, cmd_dump_layout);
#endif // NDEBUG
}
