################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/main.c \
../source/semihost_hardfault.c \
../source/shield_oled.c 

C_DEPS += \
./source/main.d \
./source/semihost_hardfault.d \
./source/shield_oled.d 

OBJS += \
./source/main.o \
./source/semihost_hardfault.o \
./source/shield_oled.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DPRINTF_FLOAT_ENABLE=0 -DSCANF_FLOAT_ENABLE=0 -DPRINTF_ADVANCED_ENABLE=0 -DSCANF_ADVANCED_ENABLE=0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_BAREMETAL -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\utilities" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\drivers" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\device" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\startup" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\component\uart" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\component\lists" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\CMSIS" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\source" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\frdmmcxn947\demo_apps\hello_world\cm33_core0" -I"C:\Users\princ\Documents\MCUXWorkspace\frdmmcxn947_hello_world_demo\board" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/main.d ./source/main.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o ./source/shield_oled.d ./source/shield_oled.o

.PHONY: clean-source

