#include "layout.h"

#include <cassert>
#include <cctype>
#include <charconv>
#include <memory>
#include <string_view>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/desktop/Workspace.hpp>
#include <hyprland/src/managers/HookSystemManager.hpp>

#include "column.h"
#include "config.h"
#include "hypaper.h"
#include "indicator.h"
#include "logging.h"
#include "workbench.h"

using namespace hypaper;
using namespace std::string_literals;

const std::string Layout::name = "paper"s;

Layout::~Layout() = default;

void Layout::onEnable() {
    hypaper_log("Layout::{}()", __func__);

    for (auto &wp : g_pCompositor->m_vWindows)
        this->onWindowCreatedTiling(wp, eDirection::DIRECTION_DEFAULT);

    this->hook_event_workspace = g_pHookSystem->hookDynamic(
        "workspace"s, [](void *, SCallbackInfo &, std::any data) {
            auto ws = std::any_cast<std::shared_ptr<CWorkspace>>(data);
            auto wb = hypaper::layout->get_workbench(ws->m_iID);
            (wb ? (*indicator << *wb) : (*indicator << Indicator::ColumnStatus{0, 0})) << flush;
        }, hypaper::hyprland_handle
    );
}

void Layout::onDisable() {
    hypaper_log("Layout::{}()", __func__);

    g_pHookSystem->unhook(std::move(this->hook_event_workspace));

    this->foreach_workspace([](Workbench &wb) -> bool {
        wb.clear();
        return true;
    });
}

bool Layout::isWindowTiled(PHLWINDOW win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win.get()));

    if (auto wb = this->get_workbench(win->m_pWorkspace->m_iID); wb)
        return wb ? wb->find_window(win) : false;
    return false;
}

void Layout::onWindowCreatedTiling(PHLWINDOW win, eDirection) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win.get()));

    if (const auto wid = win->m_pWorkspace->m_iID; wid >= 0) {
        const auto width = this->column_width_rules(win->m_szInitialClass);
        this->get_or_new_workbench(wid).add_window(std::move(win), width);
    }
}

void Layout::onWindowRemovedTiling(PHLWINDOW win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win.get()));

    if (!win->m_pWorkspace) {
        hypaper_log("Layout::{}({}) : win->m_pWorkspace == nullptr", __func__, static_cast<void *>(win.get()));
        assert(this->foreach_workspace([&win](Workbench &wb) { return !wb.find_window(win); }));
        return;
    }

    auto wb = this->get_workbench(win->m_pWorkspace->m_iID);
    if (!wb)
        return;
    if (auto fwr = wb->find_window(win); fwr) {
        wb->del_window(fwr.column, fwr.window);
        return;
    }
    this->foreach_workspace([&win](Workbench &wb) -> bool {
        auto fwr = wb.find_window(win);
        if (!fwr)
            return true;
        wb.del_window(fwr.column, fwr.window);
        return false;
    });
}

void Layout::onWindowFocusChange(PHLWINDOW win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win.get()));

    if (!win)
        return;

    if (auto wb = this->get_workbench(win->m_pWorkspace->m_iID); wb) {
        auto fwr = wb->find_window(win);
        if (fwr) {
            wb->focus_column(fwr.column);
            wb->get_column(fwr.column).focus_window(fwr.window);
        }
    }
}

void Layout::recalculateMonitor(const int &monitor_id) {
    hypaper_log("Layout::{}({})", __func__, monitor_id);

    const auto monitor = g_pCompositor->getMonitorFromID(monitor_id);
    g_pHyprRenderer->damageMonitor(monitor); // ??
}

void Layout::recalculateWindow([[maybe_unused]] PHLWINDOW win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win.get()));
}

void Layout::resizeActiveWindow(const Vector2D &, eRectCorner, PHLWINDOW) {
    hypaper_log("Layout::{}()", __func__);
}

void Layout::fullscreenRequestForWindow(PHLWINDOW win, eFullscreenMode mode, bool on) {
    hypaper_log("Layout::{}({}, {}, {})", __func__, static_cast<void *>(win.get()), int(mode), on);

    const auto monitor   = g_pCompositor->getMonitorFromID(win->m_iMonitorID);
    const auto workspace = g_pCompositor->getWorkspaceByID(win->m_pWorkspace->m_iID);

    if (workspace->m_bHasFullscreenWindow == on)
        return;

    win->m_bIsFullscreen              = on;
    workspace->m_bHasFullscreenWindow = on;

    win->updateDynamicRules();
    win->updateWindowDecos();

    using namespace std::string_literals;
    g_pEventManager->postEvent(SHyprIPCEvent{"fullscreen"s, on ? "1"s : "0"s});
    EMIT_HOOK_EVENT("fullscreen", win);

    if (!on) {
        if (win->m_bIsFloating) {
            win->m_vRealPosition = win->m_vLastFloatingPosition;
            win->m_vRealSize     = win->m_vLastFloatingSize;
            win->updateSpecialRenderData();
        } else {
            auto &wb = this->get_or_new_workbench(win->m_pWorkspace->m_iID);
            if (auto fwr = wb.find_window(win); fwr)
                wb.get_column(fwr.column)._apply_window_data();
            else
                wb.add_window(win);
        }
    } else {
        if (win->m_bIsFloating) {
            win->m_vLastFloatingSize     = win->m_vRealSize.goal();
            win->m_vLastFloatingPosition = win->m_vRealPosition.goal();
            win->m_vPosition             = win->m_vRealPosition.goal();
            win->m_vSize                 = win->m_vRealSize.goal();
        }

        workspace->m_efFullscreenMode = mode;

        switch (mode) {
        case FULLSCREEN_FULL:
            win->m_vRealPosition = monitor->vecPosition;
            win->m_vRealSize     = monitor->vecSize;
            break;

        case FULLSCREEN_MAXIMIZED:
            win->m_vRealPosition = monitor->vecPosition + monitor->vecReservedTopLeft;
            win->m_vRealSize     = monitor->vecSize - (monitor->vecReservedTopLeft + monitor->vecReservedBottomRight);
            break;

        default:
            break;
        }
    }

    g_pCompositor->updateWindowAnimatedDecorationValues(win);
    g_pXWaylandManager->setWindowSize(win, win->m_vRealSize.goal());
    g_pCompositor->changeWindowZOrder(win, true);
    this->recalculateMonitor(monitor->ID);
}

std::any Layout::layoutMessage(SLayoutMessageHeader, std::string) {
    hypaper_log("Layout::{}()", __func__);

    return ""s;
}

SWindowRenderLayoutHints Layout::requestRenderHints(PHLWINDOW) {
    hypaper_log("Layout::{}()", __func__);

    return { };
}

void Layout::switchWindows([[maybe_unused]] PHLWINDOW win1, [[maybe_unused]] PHLWINDOW win2) {
    hypaper_log("Layout::{}({}, {})", __func__, static_cast<void *>(win1.get()), static_cast<void *>(win2.get()));
}

void Layout::moveWindowTo(PHLWINDOW win, const std::string &dir, [[maybe_unused]] bool silent) {
    hypaper_log("Layout::{}({}, {}, {})", __func__, static_cast<void *>(win.get()), dir, silent);

    if (dir.size() != 1)
        return;
    const char dir_x = std::tolower(dir[0]);

    auto wb = this->get_workbench(win->m_pWorkspace->m_iID);
    if (!wb)
        return;
    auto fwr = wb->find_window(win);
    if (!fwr)
        return;

    switch (dir_x) {
    case 'l':
        if (fwr.column > 0) {
            wb->swap_columns(fwr.column - 1, fwr.column);
            wb->scroll_to_fit_focus();
        }
        break;
    case 'r':
        if (fwr.column + 1 < wb->count_columns()) {
            wb->swap_columns(fwr.column, fwr.column + 1);
            wb->scroll_to_fit_focus();
        }
        break;
    case 'u':
        if (Column &col = wb->get_column(fwr.column); fwr.window > 0)
            col.swap_windows(fwr.window - 1, fwr.window);
        break;
    case 'd':
        if (Column &col = wb->get_column(fwr.column); fwr.window + 1 < col.count_windows())
            col.swap_windows(fwr.window, fwr.window + 1);
        break;
    }
}

void Layout::alterSplitRatio(PHLWINDOW, float, bool) {
    hypaper_log("Layout::{}()", __func__);
}

std::string Layout::getLayoutName() {
    hypaper_log("Layout::{}()", __func__);

    return Layout::name;
}

PHLWINDOW Layout::getNextWindowCandidate(PHLWINDOW win) {
    hypaper_log("Layout::{}()", __func__);

    auto wbp = this->get_workbench(win->m_pWorkspace->m_iID);
    if (!wbp || wbp->is_empty())
        return nullptr;
    auto &wb = *wbp;

    if (auto &fc = wb.get_focused_column(); !fc.is_empty() && fc.find_window(win) == Column::NPOS)
        return fc.get_focused_window();

    return nullptr;
}

void Layout::replaceWindowDataWith(PHLWINDOW, PHLWINDOW) {
    hypaper_log("Layout::{}()", __func__);
}

Vector2D Layout::predictSizeForNewWindowTiled() {
    hypaper_log("Layout::{}()", __func__);

    return {0, 0};
}

static int get_active_workspace() noexcept {
    return g_pCompositor->m_pLastMonitor->activeWorkspace->m_iID;
}

void Layout::cmd_column_width(double w) {
    hypaper_log("Layout::{}({})", __func__, w);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->is_empty())
        return;
    auto &wb = *wbp;

    auto &col = wb.get_focused_column();
    if (w == 0.0 && !col.is_empty())
        w = this->column_width_rules(col.get_focused_window()->m_szInitialClass);
    col.set_width(w);
    wb.on_column_width_changed(wb.get_focused_column_index());
    wb.scroll_to_fit_focus();
}

void Layout::cmd_absorb_window() {
    hypaper_log("Layout::{}()", __func__);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->get_focused_column_index() + 1 >= wbp->count_columns())
        return;
    auto &wb = *wbp;

    PHLWINDOW win;
    if (auto &col = wb.get_column(wb.get_focused_column_index() + 1); col.is_empty())
        return;
    else if (col.count_windows() == 1)
        win = wb.del_window(wb.get_focused_column_index() + 1, 0);
    else
        win = col.del_window(col.get_focused_window_index());
    assert(!wb.is_empty());
    if (auto &col = wb.get_focused_column(); col.is_empty()) {
        col.add_window(win);
        g_pCompositor->focusWindow(std::move(win));
    } else {
        const auto old_fw = col.get_focused_window_index();
        col.add_window(std::move(win));
        col.focus_window(old_fw);
    }
}

void Layout::cmd_expel_window() {
    hypaper_log("Layout::{}()", __func__);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->is_empty())
        return;
    auto &wb = *wbp;

    if (auto &col = wb.get_focused_column(); col.count_windows() >= 2)
        wb.add_window(col.del_window(col.get_focused_window_index()));
}

void Layout::cmd_scroll(ScrollArg arg) {
    hypaper_log("Layout::{}({{{}, {}}})", __func__, int(arg.type), arg.value);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->is_empty())
        return;
    auto &wb = *wbp;

    if (arg.type == ScrollArg::OFFSET) {
        wb.scroll(arg.value);
    } else {
        Workbench::ScrollAlignment sa;
        switch (arg.type) {
        case ScrollArg::AUTO   : sa = Workbench::ScrollAlignment::AUTO  ; break;
        case ScrollArg::CENTER : sa = Workbench::ScrollAlignment::CENTER; break;
        case ScrollArg::ALIGN_L: sa = Workbench::ScrollAlignment::LEFT  ; break;
        case ScrollArg::ALIGN_R: sa = Workbench::ScrollAlignment::RIGHT ; break;
        default: return;
        }
        wb.scroll_to_fit_focus(sa);
    }
}

Workbench *Layout::get_workbench(int workspace_id) const {
    if (workspace_id >= 1 && std::size_t(workspace_id) <= this->workspaces.size()) {
        auto &w = this->workspaces[workspace_id - 1];
        if (!w)
            return nullptr;
        return w.get();
    } else {
        for (auto &&wp : this->extra_workspaces) {
            if (wp->get_workspace_id() == workspace_id)
                return wp.get();
        }
        return nullptr;
    }
}

Workbench &Layout::get_or_new_workbench(int workspace_id) {
    if (workspace_id >= 1 && std::size_t(workspace_id) <= this->workspaces.size()) {
        auto &w = this->workspaces[workspace_id - 1];
        if (!w)
            w.reset(new Workbench(workspace_id));
        return *w.get();
    } else {
        for (auto &&wp : this->extra_workspaces) {
            if (wp->get_workspace_id() == workspace_id)
                return *wp.get();
        }
        return *this->extra_workspaces.emplace_back(new Workbench(workspace_id)).get();
    }
}

double Layout::ColumnWidthRules::operator()(const std::string &client_class) {
    if (const char *conf_str = conf::column_width_rules(); conf_str != this->last_conf_str)
        this->reload_rules(conf_str);
    if (const auto iter = this->rules.find(client_class); iter != this->rules.end())
        return iter->second;
    return conf::column_width();
}

void Layout::ColumnWidthRules::reload_rules(const char *conf) {
    hypaper_log("Layout::ColumnWidthRules::{}({})", __func__, conf);

    this->last_conf_str = reinterpret_cast<const void *>(conf);
    this->rules.clear();
    for (auto conf_sv = std::string_view(conf); !conf_sv.empty(); ) {
        const auto comma_pos = conf_sv.find(',');
        auto rule_sv = conf_sv.substr(0, comma_pos);
        if (comma_pos == std::string_view::npos)
            conf_sv = {};
        else
            conf_sv.remove_prefix(comma_pos + 1);
        const auto eq_pos = rule_sv.find('=');
        double value = 0.0;
        if (
            eq_pos == std::string_view::npos ||
            std::from_chars(rule_sv.data() + eq_pos + 1, rule_sv.data() + rule_sv.size(), value).ec != std::errc{}
        ) {
            hypaper_log("Layout::ColumnWidthRules : bad-rule: {}", rule_sv);
            continue;
        }
        hypaper_log("Layout::ColumnWidthRules : {} = {}", rule_sv.substr(0, eq_pos), value);
        this->rules.emplace(rule_sv.substr(0, eq_pos), value);
    }
}
