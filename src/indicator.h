#pragma once

namespace hypaper {

class Indicator {
public:
    enum struct ColumnStatus {
        EMPTY  = 0,
        SINGLE = 1,
        FIRST  = 2,
        MIDDLE = 3,
        LAST   = 4,
    };

    Indicator();
    Indicator(const Indicator &) = delete;
    Indicator(Indicator &&) = delete;
    ~Indicator();

    Indicator &operator<<(ColumnStatus x) noexcept;
    void flush();

    Indicator &operator<<(Indicator &(func)(Indicator &)) {
        return func(*this);
    }

private:
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
