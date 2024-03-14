CXX ?= c++

CXXFLAGS ?= -march=native -O3 -pipe -fno-plt
CXXFLAGS += -Wall -Wextra -std=c++20 -fvisibility=hidden -DNDEBUG -fPIC
LDFLAGS  ?= -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now
LDFLAGS  += -shared

SRCDIR  = src
BINDIR ?= build

TARGET = hypaper.so

SRC = $(wildcard $(SRCDIR)/*.cc)
OBJ = $(patsubst $(SRCDIR)/%.cc,$(BINDIR)/%.o,$(SRC))
OUT = $(BINDIR)/$(TARGET)

all: $(OUT)

$(BINDIR):
	[ -d "$@" ] || mkdir "$@"

$(OUT): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.cc | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(OUT) $(OBJ)

.PNONY: plugin-load plugin-unload
plugin-load: $(OUT)
	hyprctl plugin load $(abspath $(OUT))
plugin-unload: $(OUT)
	hyprctl plugin unload $(abspath $(OUT))
