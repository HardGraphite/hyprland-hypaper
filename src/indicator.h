#pragma once

#include <cstddef>

namespace hypaper {

class Indicator {
public:
    struct ColumnStatus {
        std::size_t total, focus;

        bool operator==(const ColumnStatus &other) const noexcept {
            return this->focus == other.focus && this->total == other.total;
        }
    };

    Indicator();
    Indicator(const Indicator &) = delete;
    Indicator(Indicator &&) = delete;
    ~Indicator();

    Indicator &operator<<(const class Workbench &wb) noexcept;
    Indicator &operator<<(ColumnStatus x) noexcept;
    void flush();

    Indicator &operator<<(Indicator &(func)(Indicator &)) {
        return func(*this);
    }

private:
    unsigned int type;
    int fifo_fd;
    bool modified;
    ColumnStatus column_status;

    bool open_fifo();
};

inline Indicator &flush(Indicator &i) {
    i.flush();
    return i;
}



}
