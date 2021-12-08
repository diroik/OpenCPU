FREERTOS_DIR := $(TOP)/PLAT/os/freertos
CFLAGS_INC += -I$(TOP)/PLAT/os/freertos/inc          \
	            -I$(TOP)/PLAT/os/freertos/CMSIS/inc
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/src            \
                     $(FREERTOS_DIR)/CMSIS/src
ifeq ($(TOOLCHAIN),ARMCC)
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/portable/keil
endif
ifeq ($(TOOLCHAIN),GCC)
FREERTOS_SRC_DIRS += $(FREERTOS_DIR)/portable/gcc
endif
FREERTOS_CSRC = $(foreach dir, $(FREERTOS_SRC_DIRS), $(wildcard $(dir)/*.c))
FREERTOS_CFILES = $(filter-out $(FREERTOS_EXCLUDE_FILES), $(FREERTOS_CSRC))
FREERTOS_COBJSTEMP := $(patsubst %.c, %.o, $(FREERTOS_CFILES))
FREERTOS_COBJSTEMP :=$(subst $(FREERTOS_DIR),PLAT/os/freertos,$(FREERTOS_COBJSTEMP))
FREERTOS_COBJS := $(addprefix $(BUILDDIR)/, $(FREERTOS_COBJSTEMP))
-include $(FREERTOS_COBJS:.o=.d)
ifeq ($(TOOLCHAIN),GCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc
lib-y += libfreertos.a
$(BUILDDIR)/lib/libfreertos.a: $(FREERTOS_COBJS)
	@mkdir -p $(dir $@)
	$(Q)$(AR) -cr $@ $^
endif
ifeq ($(TOOLCHAIN),ARMCC)
CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil
obj-y += $(FREERTOS_COBJSTEMP)
$(BUILDDIR)/lib/libfreertos.lib: $(FREERTOS_COBJS)
	@mkdir -p $(dir $@)
	$(Q)$(AR) $(ARFLAGS) $@ $^
endif
