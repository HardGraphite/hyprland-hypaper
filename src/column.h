#pragma once

#include <hyprland/src/helpers/memory/Memory.hpp>

#include <cstddef>
#include <memory>
#include <vector>

class CMonitor;
class CWindow;

namespace hypaper {

/// A column of windows.
class Column final {
public:
    using window_ptr = SP<CWindow>;

    static constexpr std::size_t NPOS = std::size_t(-1);

    explicit Column(double h_pos, window_ptr win = nullptr, double width = 0.0);
    Column(const Column &) = delete;
    Column(Column &&) = delete;
    ~Column();

    void add_window(window_ptr win);
    window_ptr del_window(std::size_t index);
    window_ptr get_window(std::size_t index) const;
    window_ptr get_focused_window() const;
    std::size_t get_focused_window_index() const noexcept;
    void swap_windows(std::size_t index1, std::size_t index2);
    void focus_window(std::size_t index);
    std::size_t find_window(const window_ptr &win) const;
    std::size_t count_windows() const;
    bool is_empty() const noexcept;
    void set_width(double w);
    double get_width() const noexcept { return this->width; }
    double get_actual_width(double monitor_width = -1.0) const noexcept;
    void set_hposition(double x);
    double get_hposition() const noexcept { return this->h_position; }
    void _apply_window_data() const;

private:
    double width;
    double h_position;
    std::size_t focused_window;
    std::vector<window_ptr> window_list;

    void update_window_hposition() const;
    void update_window_vposition_and_size() const;
};

inline bool Column::is_empty() const noexcept {
    return this->window_list.empty();
}

inline std::size_t Column::get_focused_window_index() const noexcept {
    return this->focused_window;
}

inline void Column::_apply_window_data() const {
    this->update_window_hposition();
    this->update_window_vposition_and_size();
}

}
