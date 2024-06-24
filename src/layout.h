#pragma once

#include <hyprland/src/layout/IHyprLayout.hpp>
#include <hyprland/src/managers/HookSystemManager.hpp>

#include <array>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace hypaper {

class Workbench;

/// The paper layout.
class Layout final : public IHyprLayout {
public:
    using hook_func = HOOK_CALLBACK_FN;
    using hook_func_handle = SP<HOOK_CALLBACK_FN>;

    struct ScrollArg {
        enum { AUTO, CENTER, ALIGN_L, ALIGN_R, OFFSET } type;
        double value = 0.0;
    };

    static const std::string name;

    virtual ~Layout() override;

    virtual void onEnable() override;
    virtual void onDisable() override;

    virtual bool isWindowTiled(PHLWINDOW win) override;
    virtual void onWindowCreatedTiling(PHLWINDOW win, eDirection) override;
    virtual void onWindowRemovedTiling(PHLWINDOW win) override;
    virtual void onWindowFocusChange(PHLWINDOW win) override;
    virtual void recalculateMonitor(const int &monitor_id) override;
    virtual void recalculateWindow(PHLWINDOW win) override;
    virtual void resizeActiveWindow(const Vector2D &delta, eRectCorner corner, PHLWINDOW win = nullptr) override;
    virtual void fullscreenRequestForWindow(PHLWINDOW win, eFullscreenMode mode, bool on) override;
    virtual std::any layoutMessage(SLayoutMessageHeader header, std::string content) override;
    virtual SWindowRenderLayoutHints requestRenderHints(PHLWINDOW win) override;
    virtual void switchWindows(PHLWINDOW win1, PHLWINDOW win2) override;
    virtual void moveWindowTo(PHLWINDOW win, const std::string &direction, bool silent = false) override;
    virtual void alterSplitRatio(PHLWINDOW win, float ratio, bool exact) override;
    virtual std::string getLayoutName() override;
    virtual PHLWINDOW getNextWindowCandidate(PHLWINDOW win) override;
    virtual void replaceWindowDataWith(PHLWINDOW win_from, PHLWINDOW win_to) override;
    virtual Vector2D predictSizeForNewWindowTiled() override;

    void cmd_column_width(double w);
    void cmd_absorb_window();
    void cmd_expel_window();
    void cmd_scroll(ScrollArg arg);

    template<typename F> requires (std::is_invocable_r_v<bool, F, Workbench &>)
    bool foreach_workspace(F &&fn);
    Workbench *get_workbench(int workspace_id) const;

private:
    class ColumnWidthRules {
    public:
        double operator()(const std::string &client_class);

    private:
        const void *last_conf_str = nullptr;
        std::unordered_map<std::string, double> rules;

        void reload_rules(const char *conf);
    };

    std::array<std::unique_ptr<Workbench>, 10> workspaces;
    std::vector<std::unique_ptr<Workbench>> extra_workspaces;
    ColumnWidthRules column_width_rules;
    hook_func_handle hook_event_workspace;

    Workbench &get_or_new_workbench(int workspace_id);
};

template<typename F> requires (std::is_invocable_r_v<bool, F, Workbench &>)
inline bool Layout::foreach_workspace(F &&fn) {
    for (auto &&w : this->workspaces) {
        if (w) {
            if (!fn(*w.get()))
                return false;
        }
    }
    for (auto &&w : this->extra_workspaces) {
        if (!fn(*w.get()))
            return false;
    }
    return true;
}

}
