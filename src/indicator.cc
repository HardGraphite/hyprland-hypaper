#include "indicator.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "logging.h"

using namespace hypaper;

Indicator::Indicator() : fifo_fd(-1) {
    this->open_fifo();
}

Indicator::~Indicator() {
    if (this->fifo_fd >= 0)
        close(this->fifo_fd);
}

Indicator &Indicator::operator<<(ColumnStatus x) noexcept {
    if (this->column_status != x) {
        this->column_status = x;
        this->modified = true;
        hypaper_log("Indicator: column_status = {}", int(x));
    }
    return *this;
}

void Indicator::flush() {
    if (!this->modified)
        return;
    this->modified = false;
    if (this->fifo_fd < 0) {
        if (!this->open_fifo())
            return;
    }
    const char buffer[] = {
        static_cast<char>('0' + int(this->column_status)),
        '\n',
    };
    hypaper_log("Indicator: write: {}", buffer[0]);
    write(this->fifo_fd, buffer, sizeof buffer);
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
            hypaper_log("Indicator: mkfifo({}, 0600)", fifo_path);
        }
        this->fifo_fd = open(fifo_path, O_WRONLY | O_NDELAY);
        hypaper_log("Indicator: use fifo path=\"{}\", fd={}", fifo_path, this->fifo_fd);
        return this->fifo_fd >= 0;
    }
    return false;
}
