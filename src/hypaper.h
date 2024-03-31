#pragma once

namespace hypaper {

class Layout;
class Indicator;

extern void *hyprland_handle;
extern Layout *layout;
extern Indicator *indicator;

void initialize(void *h);

void finalize();

}
