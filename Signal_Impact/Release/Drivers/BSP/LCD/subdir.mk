################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/LCD/lcd.c \
../Drivers/BSP/LCD/lcd_ex.c 

OBJS += \
./Drivers/BSP/LCD/lcd.o \
./Drivers/BSP/LCD/lcd_ex.o 

C_DEPS += \
./Drivers/BSP/LCD/lcd.d \
./Drivers/BSP/LCD/lcd_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/LCD/%.o Drivers/BSP/LCD/%.su: ../Drivers/BSP/LCD/%.c Drivers/BSP/LCD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-LCD

clean-Drivers-2f-BSP-2f-LCD:
	-$(RM) ./Drivers/BSP/LCD/lcd.d ./Drivers/BSP/LCD/lcd.o ./Drivers/BSP/LCD/lcd.su ./Drivers/BSP/LCD/lcd_ex.d ./Drivers/BSP/LCD/lcd_ex.o ./Drivers/BSP/LCD/lcd_ex.su

.PHONY: clean-Drivers-2f-BSP-2f-LCD

