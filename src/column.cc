#include "column.h"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/Window.hpp>

#include <cassert>
#include <cmath>
#include <utility>

#include "config.h"
#include "logging.h"

using namespace hypaper;

Column::Column(double h_pos, CWindow *win):
    width(0), h_position(h_pos),
    focused_window(NPOS),
    has_window_list(false), window(nullptr)
{
    hypaper_log("Column@{}: created, x = {}", static_cast<void *>(this), h_pos);

    this->set_width(conf::column_width());
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
    win->m_vPosition.x = this->h_position;
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

double Column::get_actual_width() const noexcept {
    if (this->width > 1.0)
        return this->width;

    if (this->is_empty())
        return this->width;
    const auto &mon = *g_pCompositor->getMonitorFromID(this->get_window(0)->m_iMonitorID);
    return mon.vecSize.x * this->width;
}

void Column::set_hposition(double x) {
    if (this->h_position == x)
        return;

    hypaper_log("Column@{}: x = {}", static_cast<void *>(this), x);
    this->h_position = x;
    this->update_window_hposition();
}

void Column::update_window_hposition() const {
    if (this->has_window_list) {
        for (const auto &wp : this->window_list) {
            auto &win = *wp;
            const auto rsv = win.getFullWindowReservedArea();
            win.m_vPosition.x = this->h_position;
            win.m_vRealPosition = win.m_vPosition + rsv.topLeft;
            hypaper_log(
                "Column@{}: {}(): win {} => {},{};{},{}",
                static_cast<const void *>(this), __func__, static_cast<const void *>(&win),
                win.m_vPosition, win.m_vSize, win.m_vRealPosition.goalv(), win.m_vRealSize.goalv()
            );
        }
    } else {
        if (this->window) {
            auto &win = *this->window;
            const auto rsv = win.getFullWindowReservedArea();
            win.m_vPosition.x = this->h_position;
            win.m_vRealPosition = win.m_vPosition + rsv.topLeft;
            hypaper_log(
                "Column@{}: {}(): win {} => {},{};{},{}",
                static_cast<const void *>(this), __func__, static_cast<const void *>(&win),
                win.m_vPosition, win.m_vSize, win.m_vRealPosition.goalv(), win.m_vRealSize.goalv()
            );
        }
    }
}

void Column::update_window_vposition_and_size() const {
    double win_width = this->width;
    assert(win_width > 0.0);

    if (this->has_window_list) {
        if (this->window_list.empty())
            return;

        const auto &mon =
            *g_pCompositor->getMonitorFromID(this->window_list.front()->m_iMonitorID);
        auto win_y = mon.vecPosition.y;
        const auto win_height = std::round(mon.vecSize.y / this->window_list.size());
        if (win_width <= 1.0)
            win_width *= mon.vecSize.x;

        for (const auto &wp : this->window_list) {
            auto &win = *wp;
            const auto rsv = win.getFullWindowReservedArea();

            win.m_vPosition.y = win_y;
            win.m_vSize.x     = win_width;
            win.m_vSize.y     = win_height;

            win.m_vRealPosition = win.m_vPosition + rsv.topLeft;
            win.m_vRealSize     = win.m_vSize - (rsv.topLeft + rsv.bottomRight);

            win_y += win_height;

            hypaper_log(
                "Column@{}: {}(): win {} => {},{};{},{}",
                static_cast<const void *>(this), __func__, static_cast<const void *>(&win),
                win.m_vPosition, win.m_vSize, win.m_vRealPosition.goalv(), win.m_vRealSize.goalv()
            );
        }
    } else {
        if (!this->window)
            return;

        auto &win = *this->window;
        const auto &mon = *g_pCompositor->getMonitorFromID(win.m_iMonitorID);
        const auto rsv = win.getFullWindowReservedArea();

        if (win_width <= 1.0)
            win_width *= mon.vecSize.x;

        win.m_vPosition.y = mon.vecPosition.y;
        win.m_vSize.x     = win_width;
        win.m_vSize.y     = mon.vecSize.y;

        win.m_vRealPosition = win.m_vPosition + rsv.topLeft;
        win.m_vRealSize     = win.m_vSize - (rsv.topLeft + rsv.bottomRight);

        hypaper_log(
            "Column@{}: {}(): win {} => {},{};{},{}",
            static_cast<const void *>(this), __func__, static_cast<const void *>(&win),
            win.m_vPosition, win.m_vSize, win.m_vRealPosition.goalv(), win.m_vRealSize.goalv()
        );
    }
}
