################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/algorithm.c \
../board/board.c \
../board/clock_config.c \
../board/max30102.c \
../board/peripherals.c \
../board/pin_mux.c 

C_DEPS += \
./board/algorithm.d \
./board/board.d \
./board/clock_config.d \
./board/max30102.d \
./board/peripherals.d \
./board/pin_mux.d 

OBJS += \
./board/algorithm.o \
./board/board.o \
./board/clock_config.o \
./board/max30102.o \
./board/peripherals.o \
./board/pin_mux.o 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c board/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DPRINTF_FLOAT_ENABLE=0 -DSCANF_FLOAT_ENABLE=0 -DPRINTF_ADVANCED_ENABLE=0 -DSCANF_ADVANCED_ENABLE=0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_BAREMETAL -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\utilities" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\drivers" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\device" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\startup" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\component\uart" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\component\lists" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\CMSIS" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\source" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\frdmmcxn947\demo_apps\hello_world\cm33_core0" -I"C:\Users\Noa Burt\Documents\Uni (Local)\CX318\CX318_Project_Software\frdmmcxn947_CX318_Project_Software\board" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-board

clean-board:
	-$(RM) ./board/algorithm.d ./board/algorithm.o ./board/board.d ./board/board.o ./board/clock_config.d ./board/clock_config.o ./board/max30102.d ./board/max30102.o ./board/peripherals.d ./board/peripherals.o ./board/pin_mux.d ./board/pin_mux.o

.PHONY: clean-board

