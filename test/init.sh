#!/bin/bash

make -C "$HYPAPER_PROJECT_DIR" plugin-load
make -C "$HYPAPER_PROJECT_DIR" plugin-layout

commands="
keyword plugin:hypaper:column_width 1.0 ;
keyword plugin:hypaper:column_width_rules foot=0.6,Alacritty=0.6 ;
keyword plugin:hypaper:indicator_fifo_path /tmp/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hypaper-indicator ;
keyword bind ALT,minus,hypaper:column_width,0.66 ;
keyword bind 'ALT SHIFT,minus,hypaper:column_width,0.5' ;
keyword bind ALT,equal,hypaper:column_width,1.0 ;
keyword bind ALT,i,hypaper:absorb_window, ;
keyword bind ALT,o,hypaper:expel_window, ;
keyword bind ALT,m,hypaper:scroll,^ ;
keyword bind ALT,comma,hypaper:scroll,< ;
keyword bind ALT,period,hypaper:scroll,> ;
keyword bind ALT,slash,hypaper:dump_layout, ;
"
hyprctl --batch "${commands//$'\n'/}"

waybar -c "$HYPAPER_PROJECT_DIR/test/waybar.json"
