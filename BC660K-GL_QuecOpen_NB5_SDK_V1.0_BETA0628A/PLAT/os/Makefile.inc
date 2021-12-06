ifeq ($(BUILD_OS),y)
include $(TOP)/PLAT/os/freertos/Makefile.inc

include $(TOP)/PLAT/os/osa/Makefile.inc
endif
