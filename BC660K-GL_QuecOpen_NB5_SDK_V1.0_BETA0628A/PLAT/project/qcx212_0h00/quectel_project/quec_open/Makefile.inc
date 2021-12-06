QUECTEL_SDK_DIR        := $(TOP)/PLAT/project/$(TARGET)/$(PROJECT)/quec_open
BLBUILDDIR = $(TOP)/PLAT/out/$(TARGET)/$(PROJECT)

CFLAGS_INC += -I $(QUECTEL_SDK_DIR)/inc

CFLAGS_INC += -I $(QUECTEL_SDK_DIR)/include
CFLAGS_INC += -I $(QUECTEL_SDK_DIR)/ril/inc

CFLAGS_INC += -I $(QUECTEL_SDK_DIR)/customer/config

SDK_SRC_DIRS += $(QUECTEL_SDK_DIR)/src

SDK_EXCLUDE_FILES :=

SDK_CSRC = $(foreach dir, $(SDK_SRC_DIRS), $(wildcard $(dir)/*.c))
SDK_CFILES = $(filter-out $(SDK_EXCLUDE_FILES), $(SDK_CSRC))
SDK_COBJSTEMP := $(patsubst %.c, %.o, $(SDK_CFILES))
SDK_COBJSTEMP :=$(subst $(QUECTEL_SDK_DIR),PLAT/project/$(TARGET)/$(PROJECT)/quec_open,$(SDK_COBJSTEMP))
SDK_COBJS := $(addprefix $(BUILDDIR)/, $(SDK_COBJSTEMP))


-include $(SDK_COBJS:.o=.d)


ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += libsdk.a

$(BUILDDIR)/lib/libsdk.a: $(SDK_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

ifeq ($(BUILD_USE_OCPU_PREBUILD_LIB),n)
lib-y += libsdk.lib
endif

$(BUILDDIR)/lib/libsdk.lib: $(SDK_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
