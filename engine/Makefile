
# NANA-PS3
# Initial native build configuration

TARGET := nana_ps3
BUILD := build

SOURCES := core/main.c
OBJECTS := $(SOURCES:%.c=$(BUILD)/%.o)

CC := ppu-gcc

CFLAGS := -O2 -Wall -Wextra
LDFLAGS :=

all: check-toolchain

check-toolchain:
	@command -v $(CC) >/dev/null 2>&1 || \
		(echo "ERROR: PPU GCC not installed"; exit 1)
	@echo "PPU compiler detected."
	@echo "PS3 SDK configuration still required."

clean:
	rm -rf $(BUILD)

.PHONY: all clean check-toolchain
