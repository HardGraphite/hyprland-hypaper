#include "indicator.h"

#include <cstdio>
#include <cstring>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "logging.h"
#include "workbench.h"

using namespace hypaper;

Indicator::Indicator() :
    type(conf::indicator()), fifo_fd(-1),
    modified(true), column_status{std::size_t(-1), 0}
{
    if (this->type)
        this->open_fifo();
}

Indicator::~Indicator() {
    if (this->fifo_fd >= 0)
        close(this->fifo_fd);
}

Indicator &Indicator::operator<<(const class Workbench &wb) noexcept {
    return *this << ColumnStatus{
        .total = wb.count_columns(),
        .focus = wb.get_focused_column_index(),
    };
}

Indicator &Indicator::operator<<(ColumnStatus x) noexcept {
    if (this->column_status != x) {
        this->column_status = x;
        this->modified = true;
        hypaper_log("Indicator: column_status = ({}/{})", int(x.focus), int(x.total));
    }
    return *this;
}

void Indicator::flush() {
    if (!this->modified)
        return;
    this->modified = false;
    if (this->fifo_fd < 0) {
        this->type = conf::indicator();
        if (!this->type || !this->open_fifo())
            return;
    }
    char buffer[64];
    std::size_t out_size;
    const auto column_status = this->column_status;
    switch (this->type) {
    case 1: // "N/M"
        if (column_status.total <= 9) {
            assert(column_status.focus + 1 <= 9);
            buffer[0] = static_cast<unsigned int>(column_status.focus) + '1';
            buffer[1] = '/';
            buffer[2] = static_cast<unsigned int>(column_status.total) + '0';
            out_size = 3;
        } else {
            int n = std::snprintf(
                buffer, sizeof buffer, "%zu/%zu",
                column_status.focus, column_status.total
            );
            assert(n > 0);
            out_size = static_cast<unsigned int>(n);
        }
        break;
    case 2: // 0,1,2,3,4
        if (column_status.total == 0)
            buffer[0] = '0'; // empty
        else if (column_status.total == 1)
            buffer[0] = '1'; // single
        else if (column_status.focus == 0)
            buffer[0] = '2'; // first
        else if (column_status.focus + 1 == column_status.total)
            buffer[0] = '4'; // last
        else
            buffer[0] = '3'; // middle
        out_size = 1;
        break;
    default:
        return;
    }
    hypaper_log("Indicator: write({}, \"{}\")", this->fifo_fd, std::string_view{buffer, out_size});
    buffer[out_size++] = '\n';
    write(this->fifo_fd, buffer, out_size);
}

bool Indicator::open_fifo() {
    if (const char *fifo_path = conf::indicator_fifo_path(); *fifo_path) {
        struct stat fifo_stat;
        if (stat(fifo_path, &fifo_stat) == 0) {
            if (!S_ISFIFO(fifo_stat.st_mode)) {
                hypaper_log("Indicator: {} is not a fifo", fifo_path);
                return false;
            }
        } else {
            mkfifo(fifo_path, 0600);
            hypaper_log("Indicator: mkfifo(\"{}\", 0600)", fifo_path);
        }
        this->fifo_fd = open(fifo_path, O_WRONLY | O_NDELAY);
        if (this->fifo_fd < 0) {
            hypaper_log("Indicator: failed to open fifo ({}: {})", errno, strerror(errno));
            return false;
        }
        hypaper_log("Indicator: use fifo: path=\"{}\", fd={}", fifo_path, this->fifo_fd);
        return true;
    }
    return false;
}
