#include "layout.h"

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

    auto wb = this->get_workbench(win->m_iWorkspaceID);
    return wb ? wb->find_window(win) : false;
}

void Layout::onWindowCreatedTiling(CWindow *win, eDirection) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    this->get_or_new_workbench(win->m_iWorkspaceID).add_window(win);
}

void Layout::onWindowRemovedTiling(CWindow *win) {
    hypaper_log("Layout::{}({})", __func__, static_cast<void *>(win));

    auto wb = this->get_workbench(win->m_iWorkspaceID);
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
    hypaper_log("Layout::{}({}, {})", __func__, static_cast<void *>(win), on);

#if 0

    const auto monitor   = g_pCompositor->getMonitorFromID(win->m_iMonitorID);
    const auto workspace = g_pCompositor->getWorkspaceByID(win->m_iWorkspaceID);

    if (workspace->m_bHasFullscreenWindow && on)
        return;

    if (win->m_bIsFloating && on) {
        win->m_vLastFloatingSize     = win->m_vRealSize.goalv();
        win->m_vLastFloatingPosition = win->m_vRealPosition.goalv();
        win->m_vPosition             = win->m_vRealPosition.goalv();
        win->m_vSize                 = win->m_vRealSize.goalv();
    }

    win->m_bIsFullscreen              = on;
    workspace->m_bHasFullscreenWindow = !workspace->m_bHasFullscreenWindow;

    win->updateDynamicRules();
    win->updateWindowDecos();

    g_pEventManager->postEvent(SHyprIPCEvent{"fullscreen", on ? "1" : "0"});
    EMIT_HOOK_EVENT("fullscreen", win);

    if (!win->m_bIsFullscreen) {
        // if it got its fullscreen disabled, set back its node if it had one
        const auto PNODE = getNodeFromWindow(win);
        if (PNODE)
            applyNodeDataToWindow(PNODE);
        else {
            // get back its' dimensions from position and size
            win->m_vRealPosition = win->m_vLastFloatingPosition;
            win->m_vRealSize     = win->m_vLastFloatingSize;
            win->updateSpecialRenderData();
        }
    } else {
        // if it now got fullscreen, make it fullscreen

        workspace->m_efFullscreenMode = mode;

        // apply new pos and size being monitors' box
        if (mode == FULLSCREEN_FULL) {
            win->m_vRealPosition = monitor->vecPosition;
            win->m_vRealSize     = monitor->vecSize;
        } else {
            // This is a massive hack.
            // We make a fake "only" node and apply
            // To keep consistent with the settings without C+P code

            SDwindleNodeData fakeNode;
            fakeNode.win     = win;
            fakeNode.box         = {monitor->vecPosition + monitor->vecReservedTopLeft, monitor->vecSize - monitor->vecReservedTopLeft - monitor->vecReservedBottomRight};
            fakeNode.workspaceID = win->m_iWorkspaceID;
            win->m_vPosition = fakeNode.box.pos();
            win->m_vSize     = fakeNode.box.size();
            fakeNode.ignoreFullscreenChecks = true;

            applyNodeDataToWindow(&fakeNode);
        }
    }

    g_pCompositor->updateWindowAnimatedDecorationValues(win);
    g_pXWaylandManager->setWindowSize(win, win->m_vRealSize.goalv());
    g_pCompositor->changeWindowZOrder(win, true);
    this->recalculateMonitor(monitor->ID);

#endif
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
