#pragma once

#include <cstddef>
#include <vector>

class CMonitor;
class CWindow;

namespace hypaper {

/// A column of windows.
class Column final {
public:
    static constexpr std::size_t NPOS = std::size_t(-1);

    explicit Column(double h_pos, CWindow *win = nullptr, double width = 0.0);
    Column(const Column &) = delete;
    Column(Column &&) = delete;
    ~Column();

    void add_window(CWindow *win);
    CWindow *del_window(std::size_t index);
    CWindow *get_window(std::size_t index) const;
    CWindow *get_focused_window() const;
    std::size_t get_focused_window_index() const noexcept;
    void swap_windows(std::size_t index1, std::size_t index2);
    void move_window_up(std::size_t index);
    void move_window_down(std::size_t index);
    void focus_window(std::size_t index);
    void focus_window_up();
    void focus_window_down();
    std::size_t find_window(CWindow *win) const;
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
    bool has_window_list;
    union {
        CWindow *window;
        std::vector<CWindow *> window_list;
    };

    void update_window_hposition() const;
    void update_window_vposition_and_size() const;
};

inline bool Column::is_empty() const noexcept {
    return this->has_window_list ?
        this->window_list.empty() : !this->window;
}

inline std::size_t Column::get_focused_window_index() const noexcept {
    return this->focused_window;
}

inline void Column::_apply_window_data() const {
    this->update_window_hposition();
    this->update_window_vposition_and_size();
}

}
