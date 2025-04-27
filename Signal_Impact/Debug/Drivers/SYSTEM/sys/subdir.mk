################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../Drivers/SYSTEM/sys/sys.c 

OBJS += \
./Drivers/SYSTEM/sys/sys.o 

C_DEPS += \
./Drivers/SYSTEM/sys/sys.d 


# 每个子目录必须为构建它所贡献的源提供规则
Drivers/SYSTEM/sys/%.o Drivers/SYSTEM/sys/%.su Drivers/SYSTEM/sys/%.cyclo: ../Drivers/SYSTEM/sys/%.c Drivers/SYSTEM/sys/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DARM_MATH_CM4 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../CMSIS_DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SYSTEM-2f-sys

clean-Drivers-2f-SYSTEM-2f-sys:
	-$(RM) ./Drivers/SYSTEM/sys/sys.cyclo ./Drivers/SYSTEM/sys/sys.d ./Drivers/SYSTEM/sys/sys.o ./Drivers/SYSTEM/sys/sys.su

.PHONY: clean-Drivers-2f-SYSTEM-2f-sys

