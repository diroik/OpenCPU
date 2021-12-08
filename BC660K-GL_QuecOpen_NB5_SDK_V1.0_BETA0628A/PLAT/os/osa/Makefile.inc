OSA_DIR   := $(TOP)/PLAT/os/osa/src

CFLAGS_INC += -I$(TOP)/PLAT/os/osa/inc

OSA_SRC_DIRS += $(OSA_DIR)

OSA_CSRC = $(foreach dir, $(OSA_SRC_DIRS), $(wildcard $(dir)/*.c))
OSA_CFILES = $(filter-out $(OSA_EXCLUDE_FILES), $(OSA_CSRC))
OSA_COBJSTEMP := $(patsubst %.c, %.o, $(OSA_CFILES))
OSA_COBJSTEMP :=$(subst $(OSA_DIR),PLAT/os/osa/src,$(OSA_COBJSTEMP))
OSA_COBJS := $(addprefix $(BUILDDIR)/, $(OSA_COBJSTEMP))


-include $(OSA_COBJS:.o=.d)

ifeq ($(TOOLCHAIN),GCC)

lib-y += libosa.a

$(BUILDDIR)/lib/libosa.a: $(OSA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)
ifeq ($(BUILD_USE_PREBUILD_LIB),n)
lib-y += libosa.lib
endif
$(BUILDDIR)/lib/libosa.lib: $(OSA_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif