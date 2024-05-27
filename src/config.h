#pragma once

namespace hypaper::conf {

void _register();

double column_width() noexcept;
const char *column_width_rules() noexcept;
bool mono_center() noexcept;
unsigned int indicator() noexcept;
const char *indicator_fifo_path() noexcept;

unsigned int gaps_in() noexcept;
unsigned int gaps_out() noexcept;

}
