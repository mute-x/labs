INCDIR = cmsis/inc/
OBJDIR = obj/
SRCDIR = ./ src/ cmsis/src/
LINKDIR = ldscripts/
OUT_FILE = app

ifneq ($(lastword $(subst /, ,$(CURDIR))),obj)

all:
%:
	mkdir -p $(OBJDIR)
	$(MAKE) -C $(OBJDIR) -f ../Makefile -$(MAKEFLAGS) "ROOTDIR=$(CURDIR)/" $(MAKECMDGOALS)

else

INCDIR := $(addprefix $(ROOTDIR),$(INCDIR))
OBJDIR := $(addprefix $(ROOTDIR),$(OBJDIR))
SRCDIR := $(addprefix $(ROOTDIR),$(SRCDIR))
LINKDIR := $(addprefix $(ROOTDIR),$(LINKDIR))
OUT_FILE := $(addprefix $(ROOTDIR),$(OUT_FILE))

SOURCES := $(wildcard $(addsuffix *.c, $(SRCDIR)))
LINKFILES := $(notdir $(wildcard $(addsuffix *.ld, $(LINKDIR))))
OBJECTS := $(patsubst %.c,%.o,$(notdir $(SOURCES)))

COMPILER_DIR := $(HOME)/opt/gcc-arm-none-eabi-7-2017-q4-major/bin

CC := $(COMPILER_DIR)/arm-none-eabi-gcc
AS := $(COMPILER_DIR)/arm-none-eabi-as
CP := $(COMPILER_DIR)/arm-none-eabi-objcopy

DEFINES += -D__STARTUP_CLEAR_BSS -D__START=main

INCFLAGS := $(addprefix -I ,$(INCDIR))

MCUFLAGS += -mthumb -mcpu=cortex-m4

DEBUGFLAGS += -g3 -Wfatal-errors -DDEBUG

CFLAGS += $(INCFLAGS) $(DEFINES) $(MCUFLAGS)
CFLAGS += -fno-builtin

ASFLAGS += $(MCUFLAGS)

COMMA = ,
LDSCRIPTS= $(addprefix -Wl$(COMMA)-L,$(LINKDIR)) $(addprefix -Wl$(COMMA)-T,$(LINKFILES))
LDFLAGS += $(MCUFLAGS) --specs=nosys.specs $(LDSCRIPTS)

vpath %.h $(INCDIR)
vpath %.o $(OBJDIR)
vpath %.c $(SRCDIR)
vpath %.s $(SRCDIR)

.PHONY: clean build debug flash install-flasher

debug: CFLAGS += $(DEBUGFLAGS)
debug: ASFLAGS+= -g3
debug: build

build: $(OUT_FILE).bin
	@echo "Build success!"

%.bin: %.elf
	$(CP) -O binary $< $@

$(OUT_FILE).elf: startup_stm32f411xe.o $(OBJECTS) $(addprefix $(LINKDIR), $(LINKFILES))
	@echo "Linking files $(filter %.o,$^)…"
	$(CC) $(LDFLAGS) $(filter %.o,$^) $(LDLIBS) -o $@

clean:
	rm -rf $(addprefix $(ROOTDIR),*.o) $(OBJDIR)

flash: $(OUT_FILE).bin
	st-flash write $(OUT_FILE).bin 0x8000000

install-flasher:
	./install-flasher.sh

endif
