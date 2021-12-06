ifeq ($(BUILD_USE_PREBUILD_LIB),n)
LWIP_HOST_DIRS := $(TOP)/PROTOCOL/SRC/tcpip/lwip/src
else
LWIP_HOST_DIRS := $(TOP)/PLAT/middleware/thirdparty/lwip/src
endif

CFLAGS_INC += -I $(LWIP_HOST_DIRS)/include                 \
              -I $(LWIP_HOST_DIRS)/include/posix           \
              -I $(TOP)/PLAT/os/osa/inc                   \
              -I $(TOP)/PLAT/os/freertos/inc                \
              -I $(TOP)/PLAT/os/freertos/CMSIS/inc	        \
              -I $(TOP)/PLAT/driver/chip/qcx212/inc          \
              -I $(TOP)/PLAT/middleware/Qualcomm/debug/inc 



LWIP_SRC_DIRS += $(LWIP_HOST_DIRS)/core      \
     $(LWIP_HOST_DIRS)/core/ipv4              \
     $(LWIP_HOST_DIRS)/core/ipv6              \
     $(LWIP_HOST_DIRS)/api                    \
     $(LWIP_HOST_DIRS)/netif


LWIP_EXCLUDE_FILES := $(LWIP_HOST_DIRS)/netif/ethernet.c   \
                       $(LWIP_HOST_DIRS)/netif/ethernetif.c \
                       $(LWIP_HOST_DIRS)/netif/lowpan6.c    \
                       $(LWIP_HOST_DIRS)/netif/slipif.c


LWIP_CSRC = $(foreach dir, $(LWIP_SRC_DIRS), $(wildcard $(dir)/*.c))
LWIP_CFILES = $(filter-out $(LWIP_EXCLUDE_FILES), $(LWIP_CSRC))
LWIP_COBJSTEMP := $(patsubst %.c, %.o, $(LWIP_CFILES))
ifeq ($(BUILD_USE_PREBUILD_LIB), n)
LWIP_COBJSTEMP :=$(subst $(LWIP_HOST_DIRS),PROTOCOL/SRC/tcpip/lwip/src,$(LWIP_COBJSTEMP))
else
LWIP_COBJSTEMP :=$(subst $(LWIP_HOST_DIRS),PLAT/middleware/thirdparty/lwip/src,$(LWIP_COBJSTEMP))
endif
LWIP_COBJS := $(addprefix $(BUILDDIR)/, $(LWIP_COBJSTEMP))


-include $(LWIP_COBJS:.o=.d)

LWIPCONFIG_CFLAGS ?= -DLWIP_CONFIG_FILE=\"lwip_config_qcx2120h00.h\"
CFLAGS += $(LWIPCONFIG_CFLAGS)

ifeq ($(TOOLCHAIN),GCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/gcc

lib-y += liblwip.a

$(BUILDDIR)/lib/liblwip.a: $(LWIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif

ifeq ($(TOOLCHAIN),ARMCC)

CFLAGS_INC += -I $(TOP)/PLAT/os/freertos/portable/keil

ifeq ($(BUILD_USE_OCPU_PREBUILD_LIB),n)
lib-y += liblwip.lib
endif

$(BUILDDIR)/lib/liblwip.lib: $(LWIP_COBJS)
	@mkdir -p $(dir $@)
	$(ECHO) AR $@
	$(Q)$(AR) $(ARFLAGS) $@ $^

endif
