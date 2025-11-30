##############################################
# CC32XX Platform Configuration
##############################################

##############################################
# TOOLCHAIN
##############################################
TOOLCHAIN_PREFIX = arm-none-eabi-

##############################################
# CORE/MCU
##############################################
CPU = cortex-m4
MCU = CC3220SF

##############################################
# SDK path
##############################################
TI_SDK = /opt/ti/simplelink_cc32xx_sdk_7_10_00_13

##############################################
# Linker
##############################################
LINKER_SCRIPT = $(SRCDIR)/platform-boot/cc32xx/cc32xxsf_nortos.lds

##############################################
# DEFINES
##############################################
DEFINES += -Dgcc -D$(MCU)

##############################################
# Include directories
##############################################
PLATFORM_DIRS = \
    $(TI_SDK)/source \
    $(TI_SDK)/source/ti/devices/cc32xx \
    $(TI_SDK)/source/ti/devices/cc32xx/inc \
    $(TI_SDK)/source/ti/devices/cc32xx/driverlib \
    $(SUBMODULES)/platforms/cc32xx

##############################################
# START-UP
##############################################
PLATFORM_BOOT_DIR = $(SRCDIR)/platform-boot/cc32xx
