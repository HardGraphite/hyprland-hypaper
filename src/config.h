#pragma once

namespace hypaper::conf {

void _register();

double column_width() noexcept;
const char *column_width_rules() noexcept;
const char *indicator_fifo_path() noexcept;

unsigned int gaps_in() noexcept;
unsigned int gaps_out() noexcept;

}
