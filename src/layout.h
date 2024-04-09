#pragma once

#include <hyprland/src/layout/IHyprLayout.hpp>

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
    struct ScrollArg {
        enum { AUTO, CENTER, ALIGN_L, ALIGN_R, OFFSET } type;
        double value = 0.0;
    };

    static const std::string name;

    virtual ~Layout() override;

    virtual void onEnable() override;
    virtual void onDisable() override;

    virtual bool isWindowTiled(CWindow *win) override;
    virtual void onWindowCreatedTiling(CWindow *win, eDirection) override;
    virtual void onWindowRemovedTiling(CWindow *win) override;
    virtual void onWindowFocusChange(CWindow *win) override;
    virtual void recalculateMonitor(const int &monitor_id) override;
    virtual void recalculateWindow(CWindow *win) override;
    virtual void resizeActiveWindow(const Vector2D &delta, eRectCorner corner, CWindow *win = nullptr) override;
    virtual void fullscreenRequestForWindow(CWindow *win, eFullscreenMode mode, bool on) override;
    virtual std::any layoutMessage(SLayoutMessageHeader header, std::string content) override;
    virtual SWindowRenderLayoutHints requestRenderHints(CWindow *win) override;
    virtual void switchWindows(CWindow *win1, CWindow *win2) override;
    virtual void moveWindowTo(CWindow *win, const std::string &direction) override;
    virtual void alterSplitRatio(CWindow *win, float ratio, bool exact) override;
    virtual std::string getLayoutName() override;
    virtual CWindow *getNextWindowCandidate(CWindow *win) override;
    virtual void replaceWindowDataWith(CWindow *win_from, CWindow *win_to) override;
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
