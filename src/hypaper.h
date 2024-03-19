#pragma once

namespace hypaper {

class Layout;

extern void *hyprland_handle;
extern Layout *layout;

void initialize(void *h);

void finalize();

}
