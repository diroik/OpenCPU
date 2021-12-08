CHIP                        = qcx212
CHIP_VERSION                = A0
FPGA_DEBUG                  = n
OS                          = freertos
BUILD_USE_OCPU_PREBUILD_LIB = y
BUILD_USE_PREBUILD_LIB      = y
BUILD_ONLY_USE_PHY_LIB      = y
BUILD_FOR_KEY_CUSTOMER1     = n
BUILD_PS                    ?= y
BUILD_FW                    ?= y
#No mater BUILD_USE_PREBUILD_LIB, below is configurable.
DRIVER_DMA_ENABLE           ?= y
DRIVER_CACHE_ENABLE         ?= y
DRIVER_PMU_ENABLE           ?= y
DRIVER_FLASHRT_ENABLE       ?= y
DRIVER_FLASH_ENABLE         ?= y
DRIVER_GPR_ENABLE           ?= y
DRIVER_IC_ENABLE            ?= y
DRIVER_CIPHER_ENABLE        ?= y
DRIVER_CIPHER_FLH_ENABLE    ?= y
DRIVER_EFUSE_ENABLE         ?= y
DRIVER_CIPHER_TLS_ENABLE    ?= y
DRIVER_RNG_ENABLE           ?= y
DRIVER_MCUMODE_ENABLE       ?= y
DRIVER_ECMAIN_ENABLE        ?= y
RF_ADI_USED_ENABLE          ?= n
BUILD_EC_MW                 ?= y
BUILD_OS                    ?= y
BUILD_AT                    ?= y
BUILD_AT_DEBUG              ?= y
#Choose if below driver need to be compiled or not
DRIVER_USART_ENABLE         ?= y
DRIVER_SPI_ENABLE           ?= y
DRIVER_I2C_ENABLE           ?= y
DRIVER_GPIO_ENABLE          ?= y
DRIVER_TIMER_ENABLE         ?= y
DRIVER_UNILOG_ENABLE        ?= y
DRIVER_WDG_ENABLE           ?= y
DRIVER_ADC_ENABLE           ?= y
#Choose if below thirdparty sofware need to be compiled
THIRDPARTY_CISONENET_ENABLE ?= n
THIRDPARTY_CTWING1_5_ENABLE ?= n
THIRDPARTY_WAKAAMA_ENABLE   ?= n
THIRDPARTY_MQTT_ENABLE      ?= n
THIRDPARTY_LIBCOAP_NO_DTLS_ENABLE   ?= n
THIRDPARTY_LIBCOAP_DTLS_ENABLE      ?= n
THIRDPARTY_ERCOAP_ENABLE    ?= n
THIRDPARTY_HTTPC_ENABLE     ?= n
THIRDPARTY_LITTEFS_ENABLE   ?= y
THIRDPARTY_IPERF_ENABLE     ?= y
THIRDPARTY_CJSON_ENABLE     ?= n
THIRDPARTY_LWIP_ENABLE      ?= y
THIRDPARTY_PING_ENABLE      ?= y
THIRDPARTY_ROHC_ENABLE      ?= y
THIRDPARTY_MBEDTLS_ENABLE   ?= n
THIRDPARTY_TINYDTLS_ENABLE  ?= n
THIRDPARTY_RTT_ENABLE       ?= n
THIRDPARTY_LIBSNTP_ENABLE   ?= y
THIRDPARTY_ABUP_FOTA_ENABLE ?= n
THIRDPARTY_CTCC_DM_ENABLE   ?= n
THIRDPARTY_CUCC_DM_ENABLE   ?= n
THIRDPARTY_CMCC_DM_ENABLE   ?= n
THIRDPARTY_EXAMPLE_ENABLE   ?= n
#Choose if below ecomm middleware  need to be compiled
MIDDLEWARE_DEBUG_TRACE_ENABLE ?=n
MIDDLEWARE_CMS_ENABLE         ?=y
MIDDLEWARE_SIMBIP_ENABLE      ?=y
MIDDLEWARE_NVRAM_ENABLE       ?=y
MIDDLEWARE_FEATURE_PRODMODE_ENABLE ?=n
#add for AT REF,default is not define
BUILD_AT_REF                  ?=n
#add to distinguish debug or release version
PLAT_DEBUG_VERSION            ?=n
ifeq ($(PLAT_DEBUG_VERSION),y)
CFLAGS += -DIS_DEBUG_VERSION
endif
CFLAGS += -D__QCX212
ifeq ($(CHIP_VERSION),A0)
CFLAGS += -DCHIP_QCX212
ASMFLAGS += --predefine "CHIP_QCX212_A0 SETL {TRUE} "
endif
ifeq ($(BUILD_FOR_KEY_CUSTOMER1),y)
    CFLAGS += -DCUSTOMER_QCOM 
endif
