################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../../../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../SDK/platform/drivers/src/flash/flash_driver.c" \

C_SRCS += \
../SDK/platform/drivers/src/flash/flash_driver.c \

OBJS_OS_FORMAT += \
./SDK/platform/drivers/src/flash/flash_driver.o \

C_DEPS_QUOTED += \
"./SDK/platform/drivers/src/flash/flash_driver.d" \

OBJS += \
./SDK/platform/drivers/src/flash/flash_driver.o \

OBJS_QUOTED += \
"./SDK/platform/drivers/src/flash/flash_driver.o" \

C_DEPS += \
./SDK/platform/drivers/src/flash/flash_driver.d \


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/drivers/src/flash/flash_driver.o: ../SDK/platform/drivers/src/flash/flash_driver.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/flash/flash_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "SDK/platform/drivers/src/flash/flash_driver.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


