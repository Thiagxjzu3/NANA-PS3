
.SUFFIXES:

ifeq ($(strip $(PSL1GHT)),)
$(error PSL1GHT is not configured)
endif

include $(PSL1GHT)/ppu_rules

TARGET := nana_ps3
BUILD := build
SOURCES := core
DATA :=
INCLUDES :=

TITLE := NANA-PS3
APPID := NANAPS301

CFLAGS = -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(MACHDEP) -Wl,-Map,$(TARGET).map

LIBS := -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm
LIBDIRS :=

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
export BUILDDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES := $(CFILES:.c=.o) \
                 $(CPPFILES:.cpp=.o) \
                 $(sFILES:.s=.o) \
                 $(SFILES:.S=.o)

export INCLUDE := $(LIBPSL1GHT_INC) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(LIBPSL1GHT_LIB)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	mkdir -p $@
	$(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile $(OUTPUT).elf

clean:
	rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).self

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
