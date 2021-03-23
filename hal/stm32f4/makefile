ifndef HAL_DEVICE_INCLUDE_GUARD
       HAL_DEVICE_INCLUDE_GUARD:=ON

ifndef COMMON
$(error Please define COMMON path before including any common package)
endif

ifndef TARGET_COMPILER
$(error Please define TARGET_COMPILER before including hal package)
endif

INCS += $(COMMON)/hal/$(TARGET_DEVICE)/inc

ifneq ($(filter lib,$(MAKECMDGOALS)),)

SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_adc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_adc_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_can.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_cec.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_cortex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_crc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_cryp.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_cryp_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dac.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dac_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dcmi.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dcmi_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dfsdm.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dma.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dma2d.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dma_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_dsi.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_eth.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_exti.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_flash.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_flash_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_flash_ramfunc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_fmpi2c.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_fmpi2c_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_gpio.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_hash.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_hash_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_hcd.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_i2c.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_i2c_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_i2s.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_i2s_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_irda.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_iwdg.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_lptim.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_ltdc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_ltdc_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_mmc.c
SRCS +=#$(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_msp_template.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_nand.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_nor.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_pccard.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_pcd.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_pcd_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_pwr.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_pwr_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_qspi.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_rcc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_rcc_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_rng.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_rtc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_rtc_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_sai.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_sai_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_sd.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_sdram.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_smartcard.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_smbus.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_spdifrx.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_spi.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_sram.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_tim.c
SRCS +=#$(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_timebase_rtc_alarm_template.c
SRCS +=#$(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_timebase_rtc_wakeup_template.c
SRCS +=#$(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_timebase_tim_template.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_tim_ex.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_uart.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_usart.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_hal_wwdg.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_adc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_crc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_dac.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_dma.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_dma2d.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_exti.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_fmc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_fsmc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_gpio.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_i2c.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_lptim.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_pwr.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_rcc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_rng.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_rtc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_sdmmc.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_spi.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_tim.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_usart.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_usb.c
SRCS += $(COMMON)/hal/$(TARGET_DEVICE)/src/stm32f4xx_ll_utils.c

else

LIBS += $(COMMON)/hal/$(TARGET_DEVICE)/lib/$(TARGET_COMPILER)/$(LIBRARY_PREFIX)HAL$(LIBRARY_SUFFIX)

endif

endif
