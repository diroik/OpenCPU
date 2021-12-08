ifeq ($(TOOLCHAIN),GCC)
PRECINIT_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/boot/gcc
SYSCALLS_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/boot/gcc
SYSTEM_FILE_PATH   ?= PLAT/device/target/board/$(TARGET)/boot/src
STARTUP_FILE_PATH  ?= PLAT/device/target/board/$(TARGET)/boot/gcc
LINK_FILE_PATH     ?= PLAT/device/target/board/$(TARGET)/boot/gcc
endif

ifeq ($(TOOLCHAIN),ARMCC)
PRECINIT_FILE_PATH ?= PLAT/device/target/board/$(TARGET)/boot/keil
SYSTEM_FILE_PATH   ?= PLAT/device/target/board/$(TARGET)/boot/src
STARTUP_FILE_PATH  ?= PLAT/device/target/board/$(TARGET)/boot/keil
LINK_FILE_PATH     ?= PLAT/device/target/board/$(TARGET)/boot/keil
endif

##$(error "error  $(MAIN_FILE_PATH)")
include $(TOP)/PLAT/device/target/board/$(TARGET)/$(TARGET).mk
include $(TOP)/PLAT/device/target/board/common/macro.mk

CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/common/ARMCM3/inc


ifeq ($(TOOLCHAIN),GCC)
libstartup-y := $(PRECINIT_FILE_PATH)/prec_init.o        \
                $(SYSCALLS_FILE_PATH)/syscalls.o         \
                $(SYSTEM_FILE_PATH)/system_$(CHIP).o       \
                $(STARTUP_FILE_PATH)/startup_$(CHIP)_gcc.o

STARTUP_OBJS := $(addprefix $(BUILDDIR)/,$(libstartup-y))
-include $(STARTUP_OBJS:.o=.d)

CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/$(TARGET)/boot/gcc
lib-y += libstartup.a

linker-script-y := $(TOP)/$(LINK_FILE_PATH)/$(TARGET)_flash.ld

$(BUILDDIR)/lib/libstartup.a: $(STARTUP_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)
libstartup-y += $(PRECINIT_FILE_PATH)/prec_init.o  \
                $(SYSTEM_FILE_PATH)/system_$(CHIP).o \
                $(STARTUP_FILE_PATH)/startup_$(CHIP).o

ifeq ($(DRIVER_PMU_ENABLE),y)				
libstartup-y += $(STARTUP_FILE_PATH)/vector_$(CHIP).o
endif

STARTUP_OBJS := $(addprefix $(BUILDDIR)/,$(libstartup-y))
-include $(STARTUP_OBJS:.o=.d)

##obj-y        += $(STARTUP_FILE_PATH)/startup_$(CHIP).o

CFLAGS_INC += -I$(TOP)/PLAT/device/target/board/$(TARGET)/boot/keil
ifeq ($(BUILD_USE_OCPU_PREBUILD_LIB),n)
lib-y += libstartup.lib
endif

ifeq ($(FPGA_DEBUG),y)
linker-script-y := $(TOP)/$(LINK_FILE_PATH)/$(TARGET)_flash_fpga.sct
else
linker-script-y := $(TOP)/$(LINK_FILE_PATH)/$(TARGET)_flash.sct
endif


$(BUILDDIR)/lib/libstartup.lib: $(STARTUP_OBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif


