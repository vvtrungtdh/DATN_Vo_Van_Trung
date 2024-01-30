################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/CANTP.c" \
"../Sources/CAN_standard.c" \
"../Sources/main.c" \

C_SRCS += \
../Sources/CANTP.c \
../Sources/CAN_standard.c \
../Sources/main.c \

OBJS_OS_FORMAT += \
./Sources/CANTP.o \
./Sources/CAN_standard.o \
./Sources/main.o \

C_DEPS_QUOTED += \
"./Sources/CANTP.d" \
"./Sources/CAN_standard.d" \
"./Sources/main.d" \

OBJS += \
./Sources/CANTP.o \
./Sources/CAN_standard.o \
./Sources/main.o \

OBJS_QUOTED += \
"./Sources/CANTP.o" \
"./Sources/CAN_standard.o" \
"./Sources/main.o" \

C_DEPS += \
./Sources/CANTP.d \
./Sources/CAN_standard.d \
./Sources/main.d \


# Each subdirectory must supply rules for building sources it contributes
Sources/CANTP.o: ../Sources/CANTP.c
	@echo 'Building file: $<'
	@echo 'Executing target #24 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Sources/CANTP.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "Sources/CANTP.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Sources/CAN_standard.o: ../Sources/CAN_standard.c
	@echo 'Building file: $<'
	@echo 'Executing target #25 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Sources/CAN_standard.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "Sources/CAN_standard.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Sources/main.o: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #26 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Sources/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "Sources/main.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


