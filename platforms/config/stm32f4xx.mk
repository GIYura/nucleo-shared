##############################################
# STM32F4xx Platform Configuration
##############################################

##############################################
# TOOLCHAIN
##############################################
TOOLCHAIN_PREFIX = arm-none-eabi-

##############################################
# CORE/MCU
##############################################
CPU = cortex-m4
MCU = STM32F411xE

##############################################
# SDK path
##############################################
STM32_SDK = $(HOME)/install/STM32Cube_FW_F4_V1.28.0

##############################################
# Linker
##############################################
LINKER_SCRIPT = $(SRCDIR)/platform-boot/stm32f4xx/stm32f411retx_flash.ld

##############################################
# DEFINES
##############################################
DEFINES += -D$(MCU)

##############################################
# Include directories
##############################################
PLATFORM_DIRS = \
    $(STM32_SDK)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
    $(STM32_SDK)/Drivers/CMSIS/Include \
    $(SUBMODULES)/platforms/stm32f4xx

##############################################
# START-UP
##############################################
PLATFORM_BOOT_DIR = $(SRCDIR)/platfrom-boot/stm32f4xx

ifeq ($(PLATFORM),stm32f4xx)

.PHONY: flash ocd

# Run GDB server
ocd: # GDB server 
	@echo 'OCD running!'
	$(OCD_PATH)/openocd -f $(BOARD_CFG)

flash: $(TARGET).bin
	@echo 'Flash running!'
	st-flash write $(TARGET).bin 0x8000000

endif

help-stm32f4xx:
	@echo ""
	@echo "STM32-specific targets:"
	@echo "  flash      Flash firmware via st-flash"
	@echo "  ocd        Run OpenOCD server"
	@echo ""
