#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

class CWindow;

namespace hypaper {

class Column;

/// The container of columns.
class Workbench final {
public:
    struct FindWinResult {
        std::size_t column, window;
        operator bool() const noexcept { return column != NPOS && window != NPOS; }
    };

    static constexpr std::size_t NPOS = std::size_t(-1);

    explicit Workbench(int workspace_id);

    void add_window(CWindow *win);
    CWindow *del_window(std::size_t col_index, std::size_t win_index);
    FindWinResult find_window(CWindow *win) const;
    void focus_column(std::size_t index);
    void focus_column_left();
    void focus_column_right();
    void swap_columns(std::size_t index1, std::size_t index2);
    void move_column_left();
    void move_column_right();
    void clear();

    bool is_empty() const noexcept {
        return this->columns.empty();
    }

    int get_workspace_id() const noexcept {
        return this->workspace_id;
    }

    Column &get_column(std::size_t i) const noexcept {
        assert(i < this->columns.size());
        auto ptr = this->columns[i].get();
        assert(ptr);
        return *ptr;
    }

    Column &get_focused_column() const noexcept {
        return this->get_column(this->focused_column);
    }

    std::size_t get_focused_column_index() const noexcept {
        return this->focused_column;
    }

    std::size_t count_columns() const noexcept {
        return this->columns.size();
    }

    void scroll(bool center = false) {
        this->scroll_to_fit_focus(center);
    }

    void on_column_width_changed(std::size_t index);

private:
    std::vector<std::unique_ptr<Column>> columns;
    std::size_t focused_column;
    int workspace_id;

    double monitor_hposition() const;
    double monitor_width() const;
    void scroll_to_fit_focus(bool center = false);
    void update_column_position(double x, std::size_t index_start, std::size_t count);
};

}
