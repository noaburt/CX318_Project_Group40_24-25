################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../startup/boot_multicore_slave.c \
../startup/startup_mcxn947_cm33_core0.c 

C_DEPS += \
./startup/boot_multicore_slave.d \
./startup/startup_mcxn947_cm33_core0.d 

OBJS += \
./startup/boot_multicore_slave.o \
./startup/startup_mcxn947_cm33_core0.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.c startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DPRINTF_FLOAT_ENABLE=0 -DSCANF_FLOAT_ENABLE=0 -DPRINTF_ADVANCED_ENABLE=0 -DSCANF_ADVANCED_ENABLE=0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_BAREMETAL -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\utilities" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\drivers" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\device" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\startup" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\component\uart" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\component\lists" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\CMSIS" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\source" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\frdmmcxn947\demo_apps\hello_world\cm33_core0" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_countdown\frdmmcxn947_countdown\board" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-startup

clean-startup:
	-$(RM) ./startup/boot_multicore_slave.d ./startup/boot_multicore_slave.o ./startup/startup_mcxn947_cm33_core0.d ./startup/startup_mcxn947_cm33_core0.o

.PHONY: clean-startup

