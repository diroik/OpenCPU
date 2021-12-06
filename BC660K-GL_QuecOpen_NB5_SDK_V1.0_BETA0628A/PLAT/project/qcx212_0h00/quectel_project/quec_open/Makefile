
AVAILABLE_TARGETS = qcx212_0h00

TOOLCHAIN         = ARMCC
include
TOP               := ../../../../..

include $(TOP)/PLAT/project/$(TARGET)/$(PROJECT)/quectel.def
include $(TOP)/PLAT/device/target/board/$(TARGET)/$(TARGET).mk
ifeq ($(BUILD_USE_OCPU_PREBUILD_LIB),n)
include $(TOP)/PLAT/project/$(TARGET)/$(PROJECT)/quectel_version.h
else
APPBINNAME = APPNB5MDM32A01
endif

THIRDPARTY_WAKAAMA_ENABLE   = n
THIRDPARTY_ERCOAP_ENABLE   = n
THIRDPARTY_MQTT_ENABLE     = n
THIRDPARTY_HTTPC_ENABLE    = y
THIRDPARTY_MBEDTLS_ENABLE  = y
THIRDPARTY_TINYDTLS_ENABLE = n
THIRDPARTY_LIBCOAP_NO_DTLS_ENABLE      = n
THIRDPARTY_LIBCOAP_DTLS_ENABLE         = y
THIRDPARTY_CJSON_ENABLE = y
THIRDPARTY_ABUP_FOTA_ENABLE = n
AT_EXAMPLE_ENABLE = n
AT_ADC_ENABLE = n

COAP_MDM_ENABLE				           = y
LWM2M_MDM_ENABLE			           = y

TSENS_FEATURE_ENABLE                   = n
#[Randy.LI][20210226] Define the Macro to control Qualcomm AT+NFWUPD whether enable, disable in default.
QC_FWUPD_AT_FEATURE_ENABLE             = n

# FLASH_MONITOR_ENABLE = y

#-------------------------------------------------------------------------------
# Configure example and out target  Select only one
#-------------------------------------------------------------------------------
GLOBAL_EXPORT_FLAG += __CUSTOMER_CODE__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_GPIO__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_EINT__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_PS__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_ADC__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_TIMER__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_RTC__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_DFOTA_HTTP__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_FLOAT_MATH__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_PWM__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_I2C__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_MULTITASK__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_SPI__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_TIME__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_FILESYSTEM__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_LWIP_TCPCLIENT__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_LWIP_UDPCLIENT__
#GLOBAL_EXPORT_FLAG += __EXAMPLE_MQTT__
#-------------------------------------------------------------------------------
# Configure example and out target  Select only one
#-------------------------------------------------------------------------------
CFLAGS_INC        +=  -I customer/config \
				      -I ril/inc \
				      -I include \
				  		 
SRC_CUS := $(wildcard customer/*.c)
SRC_CNF := $(wildcard customer/config/*.c)
SRC_RIL := $(wildcard ril/src/*.c)
SRC_EXA := $(wildcard example/*.c)


obj-y             += $(patsubst %.c, PLAT/project/$(TARGET)/$(PROJECT)/quec_open/%.o, $(SRC_CUS)) \
					 $(patsubst %.c, PLAT/project/$(TARGET)/$(PROJECT)/quec_open/%.o, $(SRC_CNF)) \
					 $(patsubst %.c, PLAT/project/$(TARGET)/$(PROJECT)/quec_open/%.o, $(SRC_RIL)) \
					 $(patsubst %.c, PLAT/project/$(TARGET)/$(PROJECT)/quec_open/%.o, $(SRC_EXA)) \


include $(TOP)/PLAT/tools/scripts/Makefile.rules
ifeq ($(FLASH_MONITOR_ENABLE), y)
#enable wdt
CFLAGS += -DWDT_FEATURE_ENABLE=1  -DFS_FILE_OPERATION_STATISTIC
else
CFLAGS += -DWDT_FEATURE_ENABLE=1
endif
ifneq ($(BUILD_AT),y)
$(error This example needs to modify "BUILD_AT" to "y" in device\target\board\$(TARGET)\qcx212_0h00.mk)
endif
