# HyPaper

HyPaper is a [Hyprland](https://hyprland.org/) plugin
that provides a [PaperWM](https://github.com/paperwm/PaperWM)-like layout.

## Usage

### Enable the layout

Add the following line to the `hyprland.conf`:

```
general {
    layout = paper
}
```

or run the fowllowing command:

```sh
hyprctl keyword general:layout paper
```

### Configuration

The following config variables are available (in namespace `plugin:hypaper`):

| Name           | Description                                | Type | Default |
|----------------|--------------------------------------------|------|---------|
| `column_width` | default column width; ratio of screen width when in range `(0,1]` | float | `1.0` |

### Dispatchers

The following dispatchers are available:

| Name                    | Description                         | Params       |
|-------------------------|-------------------------------------|--------------|
| `hypaper:column_width`  | set the width of current column     | float        |
| `hypaper:absorb_window` | absorb the window to the right      | none         |
| `hypaper:expel_window`  | expel current window to the right   | none         |
| `hypaper:scroll`        | scroll columns                      | `=`=auto; `^`=center |
