################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/hello_world.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/hello_world.d \
./source/semihost_hardfault.d 

OBJS += \
./source/hello_world.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DPRINTF_FLOAT_ENABLE=0 -DSCANF_FLOAT_ENABLE=0 -DPRINTF_ADVANCED_ENABLE=0 -DSCANF_ADVANCED_ENABLE=0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_BAREMETAL -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\utilities" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\drivers" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\device" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\startup" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\component\uart" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\component\lists" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\CMSIS" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\source" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\frdmmcxn947\demo_apps\hello_world\cm33_core0" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\OLED\board" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/hello_world.d ./source/hello_world.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source

