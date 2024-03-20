#include "layout.h"

#include <cassert>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/Window.hpp>

#include "column.h"
#include "logging.h"
#include "workbench.h"

using namespace hypaper;
using namespace std::string_literals;

const std::string Layout::name = "paper"s;

Layout::~Layout() = default;

void Layout::onEnable() {
    hypaper_log("Layout::{}()", __func__);

    for (auto &wp : g_pCompositor->m_vWindows)
        this->onWindowCreatedTiling(wp.get(), eDirection::DIRECTION_DEFAULT);
}

void Layout::onDisable() {
    hypaper_log("Layout::{}()", __func__);

    this->foreach_workspace([](Workbench &wb) -> bool {
        wb.clear();
        return true;
    });
}

bool Layout::isWindowTiled(CWindow *win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    if (auto wb = this->get_workbench(win->m_iWorkspaceID); wb)
        return wb ? wb->find_window(win) : false;
    return false;
}

void Layout::onWindowCreatedTiling(CWindow *win, eDirection) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    if (const auto wid = win->m_iWorkspaceID; wid >= 0)
        this->get_or_new_workbench(wid).add_window(win);
}

void Layout::onWindowRemovedTiling(CWindow *win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    auto wb = this->get_workbench(win->m_iWorkspaceID);
    if (!wb)
        return;
    if (auto fwr = wb->find_window(win); fwr) {
        wb->del_window(fwr.column, fwr.window);
        return;
    }
    this->foreach_workspace([win](Workbench &wb) -> bool {
        auto fwr = wb.find_window(win);
        if (!fwr)
            return true;
        wb.del_window(fwr.column, fwr.window);
        return false;
    });
}

void Layout::onWindowFocusChange(CWindow *win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    if (!win)
        return;

    if (auto wb = this->get_workbench(win->m_iWorkspaceID); wb) {
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
    const auto workspace = g_pCompositor->getWorkspaceByID(monitor->activeWorkspace);

    g_pHyprRenderer->damageMonitor(monitor);

#if 0
    if (monitor->specialWorkspaceID) {
        const auto special_ws = g_pCompositor->getWorkspaceByID(monitor->specialWorkspaceID);

        if (special_ws->m_bHasFullscreenWindow) {
            const auto fullscreen_window =
                g_pCompositor->getFullscreenWindowOnWorkspace(special_ws->m_iID);

            if (special_ws->m_efFullscreenMode == FULLSCREEN_FULL) {
                fullscreen_window->m_vRealPosition = monitor->vecPosition;
                fullscreen_window->m_vRealSize     = monitor->vecSize;
            } else if (special_ws->m_efFullscreenMode == FULLSCREEN_MAXIMIZED) {
                SDwindleNodeData fakeNode;
                fakeNode.pWindow     = fullscreen_window;
                fakeNode.box         = {
                    monitor->vecPosition + monitor->vecReservedTopLeft,
                    monitor->vecSize - monitor->vecReservedTopLeft - monitor->vecReservedBottomRight,
                };
                fakeNode.workspaceID = special_ws->m_iID;
                fullscreen_window->m_vPosition        = fakeNode.box.pos();
                fullscreen_window->m_vSize            = fakeNode.box.size();
                fakeNode.ignoreFullscreenChecks = true;

                applyNodeDataToWindow(&fakeNode);
            }
        }

        const auto top_node = getMasterNodeOnWorkspace(monitor->specialWorkspaceID);

        if (top_node && monitor) {
            top_node->box = {
                monitor->vecPosition + monitor->vecReservedTopLeft,
                monitor->vecSize - monitor->vecReservedTopLeft - monitor->vecReservedBottomRight,
            };
            top_node->recalcSizePosRecursive();
        }
    }

    if (workspace->m_bHasFullscreenWindow) {
        // massive hack from the fullscreen func
        const auto fullscreen_window = g_pCompositor->getFullscreenWindowOnWorkspace(workspace->m_iID);

        if (workspace->m_efFullscreenMode == FULLSCREEN_FULL) {
            fullscreen_window->m_vRealPosition = monitor->vecPosition;
            fullscreen_window->m_vRealSize     = monitor->vecSize;
        } else if (workspace->m_efFullscreenMode == FULLSCREEN_MAXIMIZED) {
            SDwindleNodeData fakeNode;
            fakeNode.pWindow         = fullscreen_window;
            fakeNode.box             = {
                monitor->vecPosition + monitor->vecReservedTopLeft,
                monitor->vecSize - monitor->vecReservedTopLeft - monitor->vecReservedBottomRight,
            };
            fakeNode.workspaceID     = workspace->m_iID;
            fullscreen_window->m_vPosition = fakeNode.box.pos();
            fullscreen_window->m_vSize     = fakeNode.box.size();
            fakeNode.ignoreFullscreenChecks = true;

            applyNodeDataToWindow(&fakeNode);
        }

        return;
    }

    const auto top_node = getMasterNodeOnWorkspace(monitor->activeWorkspace);

    if (top_node && monitor) {
        top_node->box = {
            monitor->vecPosition + monitor->vecReservedTopLeft,
            monitor->vecSize - monitor->vecReservedTopLeft - monitor->vecReservedBottomRight,
        };
        top_node->recalcSizePosRecursive();
    }

#endif

}

void Layout::recalculateWindow(CWindow *win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));
}

void Layout::resizeActiveWindow(const Vector2D &delta, eRectCorner corner, CWindow *win) {
    hypaper_log("Layout::{}()", __func__);
}

void Layout::fullscreenRequestForWindow(CWindow *win, eFullscreenMode mode, bool on) {
    hypaper_log("Layout::{}({}, {}, {})", __func__, static_cast<void *>(win), int(mode), on);

    const auto monitor   = g_pCompositor->getMonitorFromID(win->m_iMonitorID);
    const auto workspace = g_pCompositor->getWorkspaceByID(win->m_iWorkspaceID);

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
            auto &wb = this->get_or_new_workbench(win->m_iWorkspaceID);
            if (auto fwr = wb.find_window(win); fwr)
                wb.get_column(fwr.column)._apply_window_data();
            else
                wb.add_window(win);
        }
    } else {
        if (win->m_bIsFloating) {
            win->m_vLastFloatingSize     = win->m_vRealSize.goalv();
            win->m_vLastFloatingPosition = win->m_vRealPosition.goalv();
            win->m_vPosition             = win->m_vRealPosition.goalv();
            win->m_vSize                 = win->m_vRealSize.goalv();
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
    g_pXWaylandManager->setWindowSize(win, win->m_vRealSize.goalv());
    g_pCompositor->changeWindowZOrder(win, true);
    this->recalculateMonitor(monitor->ID);
}

std::any Layout::layoutMessage(SLayoutMessageHeader, std::string) {
    hypaper_log("Layout::{}()", __func__);

    return ""s;
}

SWindowRenderLayoutHints Layout::requestRenderHints(CWindow *win) {
    hypaper_log("Layout::{}()", __func__);

    return { };
}

void Layout::switchWindows(CWindow *, CWindow *) {
    hypaper_log("Layout::{}()", __func__);
}

void Layout::moveWindowTo(CWindow *win, const std::string &direction) {
    hypaper_log("Layout::{}()", __func__);
}

void Layout::alterSplitRatio(CWindow *win, float ratio, bool exact) {
    hypaper_log("Layout::{}()", __func__);
}

std::string Layout::getLayoutName() {
    hypaper_log("Layout::{}()", __func__);

    return Layout::name;
}

CWindow *Layout::getNextWindowCandidate(CWindow *win) {
    hypaper_log("Layout::{}()", __func__);

    auto wbp = this->get_workbench(win->m_iWorkspaceID);
    if (!wbp || wbp->is_empty())
        return nullptr;
    auto &wb = *wbp;

    if (auto &fc = wb.get_focused_column(); !fc.is_empty() && fc.find_window(win) == Column::NPOS)
        return fc.get_focused_window();

    return nullptr;
}

void Layout::replaceWindowDataWith(CWindow *win_from, CWindow *win_to) {
    hypaper_log("Layout::{}()", __func__);
}

static int get_active_workspace() noexcept {
    return g_pCompositor->m_pLastMonitor->activeWorkspace;
}

void Layout::cmd_column_width(double w) {
    hypaper_log("Layout::{}({})", __func__, w);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->is_empty())
        return;
    auto &wb = *wbp;

    wb.get_focused_column().set_width(w);
    wb.on_column_width_changed(wb.get_focused_column_index());
    wb.scroll();
}

void Layout::cmd_absorb_window() {
    hypaper_log("Layout::{}()", __func__);

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->get_focused_column_index() + 1 >= wbp->count_columns())
        return;
    auto &wb = *wbp;

    CWindow *win;
    if (auto &col = wb.get_column(wb.get_focused_column_index() + 1); col.is_empty())
        return;
    else if (col.count_windows() == 1)
        win = wb.del_window(wb.get_focused_column_index() + 1, 0);
    else
        win = col.del_window(col.get_focused_window_index());
    assert(!wb.is_empty());
    if (auto &col = wb.get_focused_column(); col.is_empty()) {
        col.add_window(win);
        g_pCompositor->focusWindow(win);
    } else {
        const auto old_fw = col.get_focused_window_index();
        col.add_window(win);
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
    hypaper_log("Layout::{}({})", __func__, int(arg));

    auto wbp = this->get_workbench(get_active_workspace());
    if (!wbp || wbp->is_empty())
        return;
    auto &wb = *wbp;

    switch (arg) {
        using enum ScrollArg;
    case AUTO:
        wb.scroll();
        break;
    case CENTER:
        wb.scroll(true);
        break;
    default:
        break;
    }
}

Workbench *Layout::get_workbench(int workspace_id) const {
    if (std::size_t(workspace_id) <=this->workspaces.size()) {
        auto &w = this->workspaces[workspace_id];
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
    assert(workspace_id >= 0);

    if (std::size_t(workspace_id) <=this->workspaces.size()) {
        auto &w = this->workspaces[workspace_id];
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
