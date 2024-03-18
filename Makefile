CXX ?= c++

CXXFLAGS ?= -march=native -pipe -fno-plt
CXXFLAGS += -Wall -Wextra -std=c++23 -fPIC -fno-gnu-unique
LDFLAGS  ?=
LDFLAGS  += -shared -fno-gnu-unique
INCLUDES  = $(shell pkg-config --cflags-only-I hyprland pixman-1 libdrm)
CXXFLAGS += -DWLR_USE_UNSTABLE

ifdef DEBUG
CXXFLAGS += -Og -g
else
CXXFLAGS += -O3 -DNDEBUG
endif

SRCDIR  = src
BINDIR ?= build

TARGET = hypaper.so

SRC = $(wildcard $(SRCDIR)/*.cc)
OBJ = $(patsubst $(SRCDIR)/%.cc,$(BINDIR)/%.o,$(SRC))
OUT = $(BINDIR)/$(TARGET)

all: $(OUT)

.PHONY: debug
debug:
	make DEBUG=1

$(BINDIR):
	[ -d "$@" ] || mkdir "$@"

$(OUT): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm $(OUT) $(OBJ)

.PNONY: plugin-load plugin-unload plugin-layout
plugin-load: $(OUT)
	hyprctl plugin load $(abspath $(OUT))
plugin-unload: $(OUT)
	hyprctl plugin unload $(abspath $(OUT))
plugin-layout:
	hyprctl keyword general:layout paper
