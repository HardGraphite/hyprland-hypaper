#include "column.h"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/Window.hpp>
#include <hyprland/src/helpers/Vector2D.hpp>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <tuple>
#include <utility>

#include "config.h"
#include "logging.h"

using namespace hypaper;

static CMonitor &get_window_monitor(CWindow *win) {
    return *g_pCompositor->getMonitorFromID(win->m_iMonitorID);
}

Column::Column(double h_pos, CWindow *win, double width):
    width(0.0), h_position(h_pos),
    focused_window(NPOS),
    has_window_list(false), window(nullptr)
{
    hypaper_log("Column@{}: created, x = {}", static_cast<void *>(this), h_pos);

    this->set_width(width > 0.0 ? width : conf::column_width());
    if (win)
        this->add_window(win);
}

Column::~Column() {
    hypaper_log("Column@{}: deleted", static_cast<void *>(this));

    if (this->has_window_list)
        this->window_list.~vector();
}

void Column::add_window(CWindow *win) {
    hypaper_log("Column@{}: add window {}", static_cast<void *>(this), static_cast<void *>(win));

    if (this->has_window_list) {
        if (this->window_list.empty()) {
            this->focused_window = 0;
            this->window_list.emplace_back(win);
        } else {
            this->focused_window++;
            const auto insert_iter = this->window_list.begin() + this->focused_window;
            this->window_list.emplace(insert_iter, win);
        }
    } else if (!this->window) {
        this->window = win;
        this->focused_window = 0;
    } else {
        auto old_win = this->window;
        this->has_window_list = true;
        new (&this->window_list) decltype(this->window_list){old_win, win};
        this->focused_window = 1;
    }
    this->update_window_hposition();
    this->update_window_vposition_and_size();
}

CWindow *Column::del_window(std::size_t index) {
    CWindow *deleted_win;

    if (this->has_window_list) {
        if (index >= this->window_list.size())
            return nullptr;
        const auto win_iter = this->window_list.begin() + index;
        deleted_win = *win_iter;
        this->window_list.erase(win_iter);
        this->focused_window = this->window_list.empty() ? NPOS : index ? index - 1 : 0;
        if (!this->window_list.empty())
            this->update_window_vposition_and_size();
    } else {
        if (index != 0)
            return nullptr;
        deleted_win = this->window;
        this->window = nullptr;
        this->focused_window = Column::NPOS;
    }

    hypaper_log("Column@{}: del window #{}({})", static_cast<void *>(this), index, static_cast<void *>(deleted_win));
    return deleted_win;
}

CWindow *Column::get_window(std::size_t index) const {
    if (this->has_window_list) {
        if (index >= this->window_list.size())
            return nullptr;
        return this->window_list[index];
    } else {
        if (index != 0)
            return nullptr;
        return this->window;
    }
}

CWindow *Column::get_focused_window() const {
    return this->get_window(this->focused_window);
}

void Column::swap_windows(std::size_t index1, std::size_t index2) {
    if (index1 >= index2) {
        if (index1 == index2)
            return;
        const auto old_index2 = index2;
        index2 = index1, index1 = old_index2;
    }

    if (!this->has_window_list || index2 >= this->window_list.size())
        return;

    hypaper_log("Column@{}: swap window #{} and #{}", static_cast<void *>(this), index1, index2);
    std::swap(this->window_list[index1], this->window_list[index2]);
    this->update_window_vposition_and_size();
}

void Column::move_window_up(std::size_t index) {
    this->swap_windows(index - 1, index);
}

void Column::move_window_down(std::size_t index) {
    this->swap_windows(index, index + 1);
}

void Column::focus_window(std::size_t index) {
    if (!this->has_window_list)
        return;
    if (const auto n = this->window_list.size(); index >= n)
        index = n - 1;
    hypaper_log("Column@{}: focus window #{}", static_cast<void *>(this), index);
    this->focused_window = index;
}

void Column::focus_window_up() {
    this->focus_window(this->focused_window - 1);
}

void Column::focus_window_down() {
    this->focus_window(this->focused_window + 1);
}

std::size_t Column::find_window(CWindow *win) const {
    if (this->has_window_list) {
        if (this->window_list.empty())
            return Column::NPOS;
        if (this->window_list[this->focused_window] == win)
            return this->focused_window;
        for (std::size_t i = 0, n = this->window_list.size(); i < n; i++) {
            if (this->window_list[i] == win)
                return i;
        }
    } else {
        if (this->window == win)
            return 0;
    }
    return Column::NPOS;
}

std::size_t Column::count_windows() const {
    if (this->has_window_list) {
        return this->window_list.size();
    } else {
        return this->window ? 1 : 0;
    }
}

void Column::set_width(double w) {
    if (w <= 0.0)
        return;
    if (this->width == w)
        return;

    hypaper_log("Column@{}: width = {}", static_cast<void *>(this), w);
    this->width = w;
    this->update_window_vposition_and_size();
}

double Column::get_actual_width(double monitor_width) const noexcept {
    if (this->width > 1.0)
        return this->width;

    if (this->is_empty())
        return this->width;

    if (monitor_width <= -0.0)
        monitor_width = get_window_monitor(this->get_window(0)).vecSize.x;
    return monitor_width * this->width;
}

void Column::set_hposition(double x) {
    if (this->h_position == x)
        return;

    hypaper_log("Column@{}: x = {}", static_cast<void *>(this), x);
    this->h_position = x;
    this->update_window_hposition();
}

static void set_window_hposition(CWindow &win, double x, Vector2D padding) {

    win.m_vPosition.x = x;

    const auto rsv = win.getFullWindowReservedArea();
    win.m_vRealPosition = win.m_vPosition + rsv.topLeft + padding;
    win.m_vRealSize     = win.m_vSize - (rsv.topLeft + rsv.bottomRight) - padding * 2;

    hypaper_log(
        "Window@{}: {},{};{},{}",
        static_cast<const void *>(&win),
        win.m_vPosition, win.m_vSize,
        win.m_vRealPosition.goal(), win.m_vRealSize.goal()
    );
}

static void set_window_vposition_and_size(CWindow &win, double y, double w, double h, Vector2D padding) {
    win.m_vPosition.y = y;
    win.m_vSize.x     = w;
    win.m_vSize.y     = h;

    const auto rsv = win.getFullWindowReservedArea();
    win.m_vRealPosition = win.m_vPosition + rsv.topLeft + padding;
    win.m_vRealSize     = win.m_vSize - (rsv.topLeft + rsv.bottomRight) - padding * 2;

    hypaper_log(
        "Window@{}: {},{};{},{}",
        static_cast<const void *>(&win),
        win.m_vPosition, win.m_vSize,
        win.m_vRealPosition.goal(), win.m_vRealSize.goal()
    );
}

static double calc_window_width(const CMonitor &mon, double col_wid) {
    assert(col_wid > 0);
    if (col_wid <= 1.0)
        col_wid *= mon.vecSize.x;
    return col_wid;
}

static std::tuple<double, double>
calc_window0_vposition_and_height(const CMonitor &mon, std::size_t win_count) {
    assert(win_count);
    const auto gaps_out = conf::gaps_out(), gaps_in = conf::gaps_in();
    const auto mon_y = mon.vecPosition.y + mon.vecReservedTopLeft.y;
    const auto mon_h = mon.vecSize.y - mon.vecReservedBottomRight.y - mon.vecReservedTopLeft.y;
    const auto win_y = mon_y + gaps_out - gaps_in;
    const auto col_h = mon_h - (gaps_out - gaps_in) * 2;
    return { win_y, col_h / win_count };
}

void Column::update_window_hposition() const {
    const auto win_x = this->h_position;
    Vector2D win_p(conf::gaps_in(), conf::gaps_in());

    if (this->has_window_list) {
        if (this->window_list.empty())
            return;
        for (const auto &wp : this->window_list) {
            auto &win = *wp;
            if (win.m_bIsFullscreen)
                return;
            set_window_hposition(win, win_x, win_p);
        }
    } else {
        if (!this->window)
            return;
        auto &win = *this->window;
        if (win.m_bIsFullscreen)
            return;
        set_window_hposition(win, win_x, win_p);
    }
}

void Column::update_window_vposition_and_size() const {
    Vector2D win_p(conf::gaps_in(), conf::gaps_in());

    if (this->has_window_list) {
        if (this->window_list.empty())
            return;
        const auto &mon = get_window_monitor(this->window_list.front());
        const auto win_width = calc_window_width(mon, this->width);
        auto [win_y, win_height] =
                calc_window0_vposition_and_height(mon, this->window_list.size());
        for (const auto &wp : this->window_list) {
            auto &win = *wp;
            if (win.m_bIsFullscreen)
                return;
            set_window_vposition_and_size(win, win_y, win_width, win_height, win_p);
            win_y += win_height;
        }
    } else {
        if (!this->window)
            return;
        auto &win = *this->window;
        if (win.m_bIsFullscreen)
            return;
        const auto &mon = get_window_monitor(&win);
        const auto win_width = calc_window_width(mon, this->width);
        const auto [win_y, win_height] = calc_window0_vposition_and_height(mon, 1);
        set_window_vposition_and_size(win, win_y, win_width, win_height, win_p);
    }
}
