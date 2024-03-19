#include "workbench.h"

#include <cstddef>
#include <cmath>

#include <hyprland/src/Compositor.hpp>

#include "column.h"
#include "logging.h"

using namespace hypaper;

Workbench::Workbench(int ws) : focused_column(NPOS), workspace_id(ws) {
    hypaper_log("Workbench#{}: created", this->workspace_id);
}

void Workbench::add_window(CWindow *win) {
    hypaper_log("Workbench#{}: add window {}", this->workspace_id, static_cast<void *>(win));

    const std::size_t new_col_index = this->focused_column + 1;
    double new_col_x;
    if (this->is_empty())
        new_col_x = this->monitor_hposition();
    else if (new_col_index != this->columns.size())
        new_col_x = this->get_column(new_col_index).get_hposition();
    else
        new_col_x = this->get_focused_column().get_hposition() + this->get_focused_column().get_actual_width();
    auto new_column = new Column(new_col_x, win);
    this->columns.emplace(this->columns.begin() + new_col_index, new_column);
    this->focused_column = new_col_index;
    this->update_column_position(new_col_x + new_column->get_actual_width(), new_col_index + 1, NPOS);
    this->scroll_to_fit_focus();
}

CWindow *Workbench::del_window(std::size_t col_index, std::size_t win_index) {
    hypaper_log("Workbench#{}: del window #{}:{}", this->workspace_id, col_index, win_index);

    if (col_index >= this->columns.size())
        return nullptr;
    auto &column = *this->columns[col_index].get();
    CWindow *win = column.del_window(win_index);
    if (!win)
        return nullptr;
    if (!column.is_empty())
        return win;
    this->columns.erase(this->columns.begin() + col_index);
    this->update_column_position(win->m_vPosition.x, col_index, NPOS);
    if (this->focused_column >= win_index) {
        this->focused_column = this->columns.empty() ? NPOS : col_index == 0 ? 0 : col_index - 1;
        this->scroll_to_fit_focus();
    }
    return win;
}

void Workbench::focus_column(std::size_t index) {
    hypaper_log("Workbench#{}: focus column #{}", this->workspace_id, index);

    if (index >= this->columns.size())
        return;
    if (this->focused_column == index)
        return;
    this->focused_column = index;
    this->scroll_to_fit_focus();
}

void Workbench::focus_column_left() {
    this->focus_column(this->focused_column - 1);
}

void Workbench::focus_column_right() {
    this->focus_column(this->focused_column + 1);
}

void Workbench::swap_columns(std::size_t index1, std::size_t index2) {
    if (index1 >= index2) {
        if (index1 == index2)
            return;
        const auto old_index2 = index2;
        index2 = index1, index1 = old_index2;
    }

    hypaper_log("Workbench#{}: swap columns #{} and #{}", this->workspace_id, index1, index2);

    const auto old_col1_x = this->columns[index1]->get_hposition();
    std::swap(this->columns[index1], this->columns[index2]);
    this->update_column_position(old_col1_x, index1, index2 - index1);
}

void Workbench::move_column_left() {
    this->swap_columns(this->focused_column - 1, this->focused_column);
}

void Workbench::move_column_right() {
    this->swap_columns(this->focused_column, this->focused_column + 1);
}

void Workbench::clear() {
    hypaper_log("Workbench#{}: clear", this->workspace_id);

    this->columns.clear();
    this->focused_column = Workbench::NPOS;
}

Workbench::FindWinResult Workbench::find_window(CWindow *win) const {
    if (this->is_empty())
        return { Workbench::NPOS, Column::NPOS };
    if (auto wi = this->get_focused_column().find_window(win); wi != Column::NPOS)
        return { this->focused_column, wi };
    for (std::size_t i = 0, n = this->columns.size(); i < n; i++) {
        if (auto wi = this->get_column(i).find_window(win); wi != Column::NPOS)
            return { i, wi };
    }
    return { Workbench::NPOS, Column::NPOS };
}

void Workbench::on_column_width_changed(std::size_t index) {
    if (index + 1 >= this->columns.size())
        return;
    auto &col = *this->columns[index].get();
    this->update_column_position(col.get_hposition() + col.get_actual_width(), index + 1, NPOS);
}

double Workbench::monitor_hposition() const {
    const auto monitor_id = g_pCompositor->getWorkspaceByID(this->workspace_id)->m_iMonitorID;
    return g_pCompositor->getMonitorFromID(monitor_id)->vecPosition.x;
}

double Workbench::monitor_width() const {
    const auto monitor_id = g_pCompositor->getWorkspaceByID(this->workspace_id)->m_iMonitorID;
    return g_pCompositor->getMonitorFromID(monitor_id)->vecSize.x;
}

void Workbench::scroll_to_fit_focus(bool center) {
    if (this->is_empty())
        return;

    auto &fw = this->get_focused_column();
    const auto fw_left = fw.get_hposition();
    const auto mon_left = this->monitor_hposition();
    double scroll_to;

    if (center) {
        scroll_to = std::round(this->monitor_hposition() + (this->monitor_width() - fw.get_actual_width()) / 2);
        if (scroll_to == fw_left)
            return;
    } else {
        if (fw_left < mon_left)
            scroll_to = mon_left;
        else if (const auto mon_right = mon_left + this->monitor_width(), fw_width = fw.get_actual_width(); fw_left + fw_width > mon_right)
            scroll_to = mon_right - fw_width;
        else
            return;
    }

    const auto scroll_offset = scroll_to - fw_left;
    for (auto &&col : this->columns)
        col->set_hposition(col->get_hposition() + scroll_offset);
}

void Workbench::update_column_position(double column_x, std::size_t index_start, std::size_t count) {
    if (index_start >= this->columns.size())
        return;
    if (count == NPOS)
        count = this->columns.size() - index_start;
    else if (index_start + count > this->columns.size())
        count = this->columns.size() - index_start;
    if (!count)
        return;

    for (std::size_t i = 0; i < count; i++) {
        auto &column = *this->columns[index_start + i].get();
        column.set_hposition(column_x);
        column_x += column.get_actual_width();
    }
}
